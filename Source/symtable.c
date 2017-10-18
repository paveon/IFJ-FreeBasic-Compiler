#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "CompilationErrors.h"
#include "symtable.h"

#define LEFT(node) node->treeLeft
#define RIGHT(node) node->treeRight
#define NEXT(node) node->nextNode
#define TABLE_CHUNK 5
#define NODE_CHUNK 20

#ifdef DEBUG_INFO
static int scope_index = 0;
static int scope_level = 0;
#endif

typedef enum Scope {
	SCOPE_LOCAL,
	SCOPE_GLOBAL
} Scope;

typedef struct Node {
	uint64_t key; //Klic - hodnota hash funkce pro jmeno identifikatoru
	Identifier symbol; //Informace o identifikatoru, ktere zajimaji koncoveho uzivatele rozhrani
	struct Node* treeLeft;
	struct Node* treeRight;
	struct Node* nextNode; //Zretezeny seznam uzlu (kvuli shodam hodnot hash funkce)

	//Urcuje, zda se jedna o globalni nebo lokalni identifikator a tedy v jake tabulce se nachazi
	Scope scope;
} Node;

typedef struct IDTable {
	struct IDTable* parentScope; //Ukazatel na rodicovsky scope (NULL pro main funkci)
	struct IDTable* olderScope; //Ukazatel na stare out-of-scope tabulky (v techto nevyhledavame)
	Node* root; //Ukazatel na koren samotneho BST

#ifdef DEBUG_INFO
	int index;
#endif
} IDTable;

typedef struct NodeStash {
	Node* allocated; //Pole vsech alokovanych uzlu
	Node** unused; //Pole obsahujici ukazatele na nepouzivane uzly, pripadne NULL
	size_t size; //Pocet existujicich uzlu
	size_t used; //Pocet aktivne pouzivanych uzlu
} NodeStash;

typedef struct TableStash {
	IDTable* allocated; //Pole vsech alokovanych tabulek
	IDTable** unused; //Pole obsahujici ukazatele na pouzivane tabulky, pripadne NULL
	size_t size; //Pocet existujicich tabulek
	size_t used; //Pocet aktivne pouzivanych tabulek
} TableStash;

/* Pomocne struktury pro jednodussi spravu pameti a jeji korektni
 * uvolnovani pri chybach.
 */
static NodeStash g_Nodes;
static TableStash g_Tables;

/* Globalni tabulka symbolu - v soucasnosti se pouziva pouze pro ukladani
 * deklaraci a definici funkci, pozdeji by mela byt lehce rozsiritelna pro podporu
 * globalnich promennych. Nepouziva podtabulky (neni potreba).
 */
static IDTable g_GlobalSymbols;

/* Lokalni tabulka symbolu - slouzi pro ukladani vsech lokalnich promennych
 * a parametru funkci. Podporuje vnorovani bloku pomoci podtabulek a invalidaci
 * starych bloku (pri vynoreni z bloku znepristupnime tabulku, kterou jsme pro dany
 * blok pouzili).
 */
static IDTable g_LocalSymbols;

/* Promenna pro ulozeni informace o aktualni urovni zanoreni, slouzi pro
 * automatickou praci s podtabulkami pomoci funkci BeginSubScope a EndSubScope.
 */
static IDTable* g_ActiveScope = &g_LocalSymbols;


/* Vklada novy uzel do binarniho stromu, vraci ukazatel na nove
 * vytvoreny uzel. Pokud uzel se stejnym nazvem jiz existoval, vraci NULL.
 */
Node* InsertNode(const char* name);


/* Hash funkce pro prevedeni nazvu identifikatoru na cislo */
uint64_t HashFunction(const char* str) {
	uint64_t hash = 5381;
	int charCode;

	while ((charCode = *str++)) {
		hash = ((hash << 5) + hash) + charCode;
	}

	return hash;
}


void BeginSubScope() {
	//Zkontrolujeme, zda mame k dispozici nejakou volnou tabulku
	if (g_Tables.used == g_Tables.size) {
		IDTable* tables;
		IDTable** available;
		g_Tables.size += TABLE_CHUNK;
		if ((tables = realloc(g_Tables.allocated, sizeof(IDTable) * g_Tables.size)) == NULL ||
				(available = realloc(g_Tables.unused, sizeof(IDTable*) * g_Tables.size)) == NULL) {
			//Pokud selze jedna z alokaci ukoncime program
			FatalError(ER_FATAL_INTERNAL);
		}
		//Priradime nove ukazatele po korektni realokaci
		g_Tables.allocated = tables;
		g_Tables.unused = available;

		//Zkopirujeme ukazatele na nove tabulky do pole nepouzivanych tabulek, aby byly k dispozici
		for (size_t i = g_Tables.used; i < g_Tables.size; i++) {
			g_Tables.unused[i] = &g_Tables.allocated[i];
		}
	}

	//Pouzijeme prvni volnou tabulku a inkrementujeme pocitadlo pouzitych tabulek
	IDTable* freeTable = g_Tables.unused[g_Tables.used++];

	//Pred pouzitim (re)inicializujeme tabulku - mohla byt drive pouzivana
	freeTable->root = NULL;
	freeTable->parentScope = g_ActiveScope;
	freeTable->olderScope = g_ActiveScope->olderScope;
	g_ActiveScope->olderScope = freeTable;
	g_ActiveScope = freeTable;

	/* Nic se nevraci, modul si interne udrzuje v pameti aktivni tabulku identifikatoru */

#ifdef DEBUG_INFO
	newTable->index = scope_index;
	for (int i = 0; i < scope_level; i++) {
		printf("\t");
	}
	printf("Begin subscope %d\n\n", newTable->index);
	scope_level++;
	scope_index++;
#endif
}


/* Ukoncuje aktualni podblok (if, while apod.), posouvame se do rodicovskeho bloku. */
void EndSubScope() {
#ifdef DEBUG_INFO
	scope_level--;
	for (int i = 0; i < scope_level; i++) {
		printf("\t");
	}
	printf("End subscope %d\n\n", g_ActiveScope->index);
#endif

	//Presun zpet na vyssi uroven
	if (g_ActiveScope->parentScope != NULL) {
		g_ActiveScope = g_ActiveScope->parentScope;
	}
}

/* Ukoncuje aktualni blok funkce. Uvolni vsechny lokalni tabulky a identifikatory
 * pro dalsi pouziti v dalsich funkcich (neprovadi dealokaci).
 */
void EndScope(void) {
	//Uvolnime vsechny lokalni symboly
	for (size_t i = 0; i < g_Nodes.used; i++) {
		if (g_Nodes.allocated[i].scope == SCOPE_LOCAL) {
			g_Nodes.unused[--g_Nodes.used] = &g_Nodes.allocated[i];
		}
	}

	//Vsechny alokovane tabulky jsou lokalni - uvolnujeme tedy vsechny
	for (size_t i = 0; i < g_Tables.used; i++) {
		g_Tables.unused[i] = &g_Tables.allocated[i];
	}
	g_Tables.used = 0;

	g_LocalSymbols.root = NULL;
	g_LocalSymbols.olderScope = NULL;
	g_ActiveScope = &g_LocalSymbols;
}


Node* CreateNode(uint64_t key, const char* symbolName) {
	//Zkontrolujeme, zda mame k dispozici nejaky volny alokovany uzel
	if (g_Nodes.used == g_Nodes.size) {
		Node* nodes;
		Node** available;
		g_Nodes.size += NODE_CHUNK;
		if ((nodes = realloc(g_Nodes.allocated, sizeof(Node) * g_Nodes.size)) == NULL ||
				(available = realloc(g_Nodes.unused, sizeof(Node*) * g_Nodes.size)) == NULL) {
			//Pokud selze jedna z alokaci ukoncime program
			FatalError(ER_FATAL_INTERNAL);
		}
		//Priradime nove ukazatele po korektni realokaci
		g_Nodes.allocated = nodes;
		g_Nodes.unused = available;

		//Zkopirujeme ukazatele na nove uzly do pole nepouzivanych uzlu, aby byly k dispozici
		for (size_t i = g_Nodes.used; i < g_Nodes.size; i++) {
			g_Nodes.unused[i] = &g_Nodes.allocated[i];
		}
	}

	//Pouzijeme prvni volny uzel a inkrementujeme pocitadlo pouzitych uzlu
	Node* freeNode = g_Nodes.unused[g_Nodes.used++];

	//Pred pouzitim (re)inicializujeme hodnoty - uzel mohl byt drive pouzivan
	freeNode->key = key;
	freeNode->scope = SCOPE_LOCAL;
	freeNode->symbol.declaration = false;
	freeNode->symbol.argIndex = 1;
	freeNode->symbol.name = symbolName;
	LEFT(freeNode) = RIGHT(freeNode) = NEXT(freeNode) = NULL;

	//Vratime ukazatel na (re)inicializovany nepouzivany uzel
	return freeNode;
}


Identifier* InsertLocalID(const char* name) {
	if (!name) { return NULL; }

	//Pokud uzel jiz existoval, vraci funkce NULL
	Node* newNode = InsertNode(name);
	if (newNode) {
		newNode->scope = SCOPE_LOCAL;
		return &newNode->symbol;
	}
	return NULL;
}


Identifier* InsertGlobalID(const char* name) {
	if (!name) { return NULL; }

	//ulozeni aktivniho lokalniho scopu
	IDTable* inActiveScope = g_ActiveScope;
	g_ActiveScope = &g_GlobalSymbols;

	Node* newNode = InsertNode(name); //Pokud uzel jiz existoval, vraci funkce NULL
	g_ActiveScope = inActiveScope; //obnoveni scopu

	if (newNode) {
		newNode->scope = SCOPE_GLOBAL;
		return &newNode->symbol;
	}
	return NULL;
}


/* Interni funkce, neprovadi kontrolu ukazatele */
Node* InsertNode(const char* name) {
	//Pouzivam hashovani, protoze neustale porovnavani pripadne
	//velmi dlouhych identifikatoru je pomale a u stromu je riziko kolize minimalni
	uint64_t hash = HashFunction(name);

	if (g_ActiveScope->root == NULL) {
		return (g_ActiveScope->root = CreateNode(hash, name));
	}

	Node* current = g_ActiveScope->root;
	while (true) {
		if (hash < current->key) {
			if (LEFT(current)) {
				current = LEFT(current);
			}
			else {
				return (LEFT(current) = CreateNode(hash, name));
			}
		}
		else if (hash > current->key) {
			if (RIGHT(current)) {
				current = RIGHT(current);
			}
			else {
				return (RIGHT(current) = CreateNode(hash, name));
			}
		}
		else {
			//Stejny hash, nutna blizsi kontrola
			Node* previous = NULL;

			while (current) {
				if (strlen(name) == strlen(current->symbol.name) &&
						strcmp(name, current->symbol.name) == 0) {
					return NULL; //Symbol jiz existuje
				}
				previous = current;
				current = NEXT(current);
			}

			return (NEXT(previous) = CreateNode(hash, name));
		}
	}
}


Identifier* LookupID(const char* symbol) {
	if (!symbol) { return NULL; }

	uint64_t hash = HashFunction(symbol);
	Node* current;

	IDTable* table = g_ActiveScope;
	while (table) {
		current = table->root;
		while (current) {
			if (hash < current->key) {
				current = LEFT(current);
			}
			else if (hash > current->key) {
				current = RIGHT(current);
			}
			else {
				while (current) {
					if (strlen(symbol) == strlen(current->symbol.name) &&
							strcmp(symbol, current->symbol.name) == 0) {
						//Symbol existuje
						return &current->symbol;
					}
					current = NEXT(current);
				}
				break;
			}
		}

		//Prohledame vyssi tabulku
		table = table->parentScope;
	}

	return NULL;
}


Identifier* LookupGlobalID(const char* name) {
	//Ulozeni aktivniho lokalniho scopu
	IDTable* inActiveScope = g_ActiveScope;
	g_ActiveScope = &g_GlobalSymbols;

	Identifier* symbol = LookupID(name);

	//Obnoveni scopu
	g_ActiveScope = inActiveScope;
	return symbol;
}


bool SetSignature(Identifier* id, Terminal type, bool returnType) {
	if (!id || id->argIndex == MAX_ARGS) { return false; }
	size_t index = returnType ? 0 : id->argIndex;
	switch (type) {
		case T_INTEGER:
			id->signature[index] = 'i';
			break;
		case T_DOUBLE:
			id->signature[index] = 'd';
			break;
		case T_STRING:
			id->signature[index] = 's';
			break;
		default:
			return false;
	}
	if (!returnType) {
		id->argIndex++;
	}
	return true;
}


bool CompareSignature(Identifier* id, Terminal type, size_t index) {
	if (!id || id->argIndex < index) { return false; }
	switch (type) {
		case T_INTEGER:
			return id->signature[index] == 'i';
		case T_DOUBLE:
			return id->signature[index] == 'd';
		case T_STRING:
			return id->signature[index] == 's';
		default:
			return false;
	}
}


void TableCleanup() {
	if (g_Nodes.allocated) {
		//Obe pole se alokuji spolecne, staci kontrolovat jedno
		free(g_Nodes.allocated);
		free(g_Nodes.unused);

		//Reinicializace pro pripadnou budouci alokaci
		g_Nodes.allocated = NULL;
		g_Nodes.unused = NULL;
		g_Nodes.used = g_Nodes.size = 0;
	}

	if (g_Tables.allocated) {
		//Obe pole se alokuji spolecne, staci kontrolovat jedno
		free(g_Tables.allocated);
		free(g_Tables.unused);

		//Reinicializace pro pripadnou budouci alokaci
		g_Tables.allocated = NULL;
		g_Tables.unused = NULL;
		g_Tables.used = g_Tables.size = 0;
	}

	//Reinicializace ukazatelu nejvyssich urovni tabulek
	g_LocalSymbols.olderScope = g_GlobalSymbols.olderScope = NULL;
	g_LocalSymbols.root = g_GlobalSymbols.root = NULL;

	//Aktivni scope je defaultne nejvyssi uroven lokalnich tabulek
	g_ActiveScope = &g_LocalSymbols;
}
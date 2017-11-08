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
#define TABLE_CHUNK 20
#define NODE_CHUNK 50
#define PARAM_CHUNK 20

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
	struct Node* treeLeft;
	struct Node* treeRight;
	struct Node* nextNode; //Zretezeny seznam uzlu (kvuli shodam hodnot hash funkce)

	bool function; //Zda se jedna o funkci nebo promennou
	union {
		//Nepotrebujeme vyuzit obe promenne zaraz
		Variable var; //Pokud se jedna o promennou
		Function func; //Pokud se jedna o funkci
	} data;

	//Urcuje, zda se jedna o globalni nebo lokalni uzel a tedy v jake tabulce se nachazi
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
	Node** allocated; //Pole vsech alokovanych uzlu
	Node** unused; //Pole obsahujici ukazatele na nepouzivane uzly, pripadne NULL
	size_t size; //Pocet existujicich uzlu
	size_t used; //Pocet aktivne pouzivanych uzlu
} NodeStash;

typedef struct TableStash {
	IDTable** allocated; //Pole vsech alokovanych tabulek
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


/* Vlozi novy uzel do aktivni tabulky, pokud chceme vlozit
 * funkci staci pouzit parametr 'func'. Identifikatory
 * funkci patri pouze do globalni tabulky, a proto je pred
 * volanim teto funkce nutne prepnout se do globalni tabulky,
 * pokud chceme vkladat identifikator funkce nebo globalni promenne.
 *
 * Jedna se o interni funkci, neprovadi kontrolu ukazatele.
 */
Node* InsertNode(const char* name, bool function);


/* Prohleda aktivni tabulku symbolu a pokusi se najit uzel
 * daneho jmena a typu (funkce / promenna). Funkce se nachazi
 * pouze v globalni tabulce, proto je nutne prepnout se do
 * globalni tabulky pred volanim teto funkce, pokud hledame
 * identifikator funkce.
 *
 * Jedna se o interni funkci, neprovadi kontrolu ukazatelu.
 */
Node* FindNode(const char* name, bool function, bool onlyCurrentScope, bool allowGlobals);


/* Hash funkce pro prevedeni nazvu identifikatoru na cislo */
uint_least32_t HashFunction(const char* str) {
	uint_least32_t hash = 5381;
	int charCode;

	while ((charCode = *str++)) {
		hash = ((hash << 5) + hash) + charCode;
	}

	return hash;
}


void BeginSubScope() {
	//Zkontrolujeme, zda mame k dispozici nejakou volnou tabulku
	if (g_Tables.used == g_Tables.size) {
		IDTable* newTables;
		IDTable** pointers;
		IDTable** available;
		g_Tables.size += TABLE_CHUNK;

		//Alokace pomocnych poli pro uschovani ukazatelu.
		//Samotne tabulky se alokuji jako posledni, aby byla zachovana konzistence stavu v pripade
		//selhani alokace a nehrozil memory leak
		if ((pointers = realloc(g_Tables.allocated, sizeof(IDTable*) * g_Tables.size)) == NULL ||
				(available = realloc(g_Tables.unused, sizeof(IDTable*) * g_Tables.size)) == NULL ||
				(newTables = malloc(sizeof(IDTable) * TABLE_CHUNK)) == NULL) {
			//Pokud selze jedna z alokaci ukoncime program
			FatalError(ER_FATAL_INTERNAL);
		}
		//Priradime nove ukazatele po korektni realokaci
		g_Tables.allocated = pointers;
		g_Tables.unused = available;

		//Zkopirujeme ukazatele na nove tabulky do pole nepouzivanych tabulek, aby byly k dispozici
		size_t tmp = 0;
		for (size_t i = g_Tables.used; i < g_Tables.size; i++) {
			g_Tables.unused[i] = g_Tables.allocated[i] = &newTables[tmp++];
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
		if (g_Nodes.allocated[i]->scope == SCOPE_LOCAL) {
			g_Nodes.unused[--g_Nodes.used] = g_Nodes.allocated[i];
		}
	}

	//Vsechny alokovane tabulky jsou lokalni - uvolnujeme tedy vsechny
	for (size_t i = 0; i < g_Tables.used; i++) {
		g_Tables.unused[i] = g_Tables.allocated[i];
	}
	g_Tables.used = 0;

	g_LocalSymbols.root = NULL;
	g_LocalSymbols.olderScope = NULL;
	g_ActiveScope = &g_LocalSymbols;
}


Node* CreateNode(uint64_t key) {
	//Zkontrolujeme, zda mame k dispozici nejaky volny alokovany uzel
	if (g_Nodes.used == g_Nodes.size) {
		Node* newNodes;
		Node** pointers;
		Node** available;
		g_Nodes.size += NODE_CHUNK;
		//Alokace pomocnych poli. Samotne uzly se alokuji jako posledni, aby nehrozil
		//memory leak v pripade selhani jedne z alokaci
		if ((pointers = realloc(g_Nodes.allocated, sizeof(Node*) * g_Nodes.size)) == NULL ||
				(available = realloc(g_Nodes.unused, sizeof(Node*) * g_Nodes.size)) == NULL ||
				(newNodes = malloc(sizeof(Node) * NODE_CHUNK)) == NULL) {
			//Pokud selze jedna z alokaci ukoncime program
			FatalError(ER_FATAL_INTERNAL);
		}
		//Priradime nove ukazatele po korektni realokaci
		g_Nodes.allocated = pointers;
		g_Nodes.unused = available;

		//Zkopirujeme ukazatele na nove uzly do pole nepouzivanych uzlu, aby byly k dispozici
		size_t tmp = 0;
		for (size_t i = g_Nodes.used; i < g_Nodes.size; i++) {
			g_Nodes.unused[i] = g_Nodes.allocated[i] = &newNodes[tmp++];
		}
	}

	//Pouzijeme prvni volny uzel a inkrementujeme pocitadlo pouzitych uzlu
	Node* freeNode = g_Nodes.unused[g_Nodes.used++];

	//Pred pouzitim (re)inicializujeme hodnoty - uzel mohl byt drive pouzivan
	freeNode->key = key;
	LEFT(freeNode) = RIGHT(freeNode) = NEXT(freeNode) = NULL;

	//Vratime ukazatel na (re)inicializovany nepouzivany uzel
	return freeNode;
}


Variable* InsertVariable(const char* name, bool global, size_t line) {
	if (!name) { return NULL; }

	Node* newNode;
	if (global) {
		//Vkladame globalni promennou, prepneme se do globalni tabulky
		IDTable* inActiveScope = g_ActiveScope;
		g_ActiveScope = &g_GlobalSymbols;
		newNode = InsertNode(name, false); //Vytvoreni nove globalni promenne
		g_ActiveScope = inActiveScope; //Obnoveni scopu

		if (!newNode) {
			return NULL; //Uzel se stejnym nazevem jiz existoval
		}
		newNode->scope = SCOPE_GLOBAL; //Nachazi se v globalni tabulce
	}
	else {
		newNode = InsertNode(name, false); //Vytvoreni nove lokalni promenne
		if (!newNode) {
			return NULL; //Uzel se stejnym nazevem jiz existoval
		}
		newNode->scope = SCOPE_LOCAL; //Nachazi se v lokalni tabulce
	}

	//Provedem inicializaci specifickou pro promenne
	newNode->function = false;
	newNode->data.var.name = name; //Nazev promenne
	newNode->data.var.type = T_UNDEFINED; //Zatim nespecifikovany typ
	newNode->data.var.staticVariable = false;
	newNode->data.var.codeLine = line; //Radek na kterem se deklarace /definice nachazi
	return &newNode->data.var;
}


Function* InsertFunction(const char* name, bool declaration, size_t line) {
	if (!name) { return NULL; }

	//ulozeni aktivniho lokalniho scopu
	IDTable* inActiveScope = g_ActiveScope;
	g_ActiveScope = &g_GlobalSymbols;

	Node* newNode = InsertNode(name, NULL); //Pokud uzel jiz existoval, vraci funkce NULL
	g_ActiveScope = inActiveScope; //obnoveni scopu

	if (newNode) {
		newNode->scope = SCOPE_GLOBAL;
		newNode->function = true;
		newNode->data.func.name = name; //Nazev funkce
		newNode->data.func.declaration = declaration; //Zda se jedna pouze o deklaraci
		newNode->data.func.argCount = 0; //Prozatim 0 parametru
		newNode->data.func.parameters = NULL;
		newNode->data.func.returnType = T_UNDEFINED; //Prozatim nespecifikovany navratovy typ
		newNode->data.func.codeLine = line; //Radek na kterem se deklarace /definice nachazi
		return &newNode->data.func;
	}
	return NULL;
}


Variable* LookupVariable(const char* symbol, bool onlyCurrentScope, bool allowGlobals) {
	Node* node = FindNode(symbol, false, onlyCurrentScope, allowGlobals);
	if (node) {
		//Promenna existuje
		return &node->data.var;
	}
	return NULL;
}


Function* LookupFunction(const char* name) {
	//Ulozime si aktivni scope a prepneme se do
	//globalniho scopu, protoze funkce se nemohou nachazet nikde jinde
	IDTable* inActiveScope = g_ActiveScope;
	g_ActiveScope = &g_GlobalSymbols;

	Node* node = FindNode(name, true, true, true);

	//Obnoveni scopu
	g_ActiveScope = inActiveScope;
	if (node) {
		//Funkce existuje
		return &node->data.func;
	}
	return NULL;
}


Node* InsertNode(const char* name, bool function) {
	//Pouzivam hashovani, protoze neustale porovnavani pripadne
	//velmi dlouhych identifikatoru je pomale a u stromu je riziko kolize minimalni
	uint_least32_t hash = HashFunction(name);

	if (g_ActiveScope->root == NULL) {
		return (g_ActiveScope->root = CreateNode(hash));
	}

	Node* current = g_ActiveScope->root;
	while (true) {
		if (hash < current->key) {
			if (LEFT(current)) {
				current = LEFT(current);
			}
			else {
				return (LEFT(current) = CreateNode(hash));
			}
		}
		else if (hash > current->key) {
			if (RIGHT(current)) {
				current = RIGHT(current);
			}
			else {
				return (RIGHT(current) = CreateNode(hash));
			}
		}
		else {
			//Uzel s danym hashem existuje, nyni je potreba provest kontrolu
			//na plnou shodu, jelikoz je umozneno vytvaret funkce a globalni
			//promenne se stejnym nazvem.
			Node* previous = NULL;

			while (current) {
				if (function && current->function) {
					//Vkladame funkci a aktualni uzel je funkce -> moznost kolize
					if (strcmp(name, current->data.func.name) == 0) {
						//Funkce se stejnym nazvem jiz existuje
						return NULL;
					}
				}
				else if (!function && !current->function) {
					//Vkladame promennou a aktualni uzel je promenna -> moznost kolize
					if (strcmp(name, current->data.var.name) == 0) {
						//Promenna se stejnym nazvem jiz existuje.
						return NULL;
					}
				}

				//Nazvy se lisi, posuneme se na dalsi uzel v seznamu
				previous = current;
				current = NEXT(current);
			}

			return (NEXT(previous) = CreateNode(hash));
		}
	}
}


Node* FindNode(const char* name, bool function, bool onlyCurrentScope, bool allowGlobals) {
	if (!name) { return NULL; }

	uint_least32_t hash = HashFunction(name);
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
				//Uzel s danym hashem existuje, nyni je potreba provest kontrolu
				//na plnou shodu, kvuli mozne existenci globalni promenne i funkce
				//se stejnym nazvem
				while (current) {
					if (function && current->function) {
						//Hledame funkci, porovname jmena
						if (strcmp(name, current->data.func.name) == 0) {
							//Funkce existuje
							return current;
						}
					}
					else if (!function && !current->function) {
						//Hledame promennou, porovname jmena
						if (strcmp(name, current->data.var.name) == 0) {
							//Promenna existuje
							return current;
						}
					}

					//Jmeno se neshodovalo, posuneme se dale v seznamu uzlu
					current = NEXT(current);
				}
				break;
			}
		}

		//Prohledavame vyssi tabulky, pouze pokud je to povoleno argumentem
		if (!onlyCurrentScope) {
			table = table->parentScope;
		}
		else {
			table = NULL;
		}

		if (allowGlobals && !table) {
			//Pokud se aktualne nenachazime v globalni tabulce, rodicovska tabulka neexistuje a
			//symbol jsme stale nenasli, zkusime vyhledat v globalni tabulce
			table = &g_GlobalSymbols;

			//Zabranime nekonecnemu cyklu, globalni tabulka ma pouze jednu uroven
			allowGlobals = false;
		}
	}

	return NULL;
}


void AddParameter(Function* function, Terminal parameter) {
	//Abychom si nemuseli pamatovat celkovou velikost pole
	if (function->argCount % PARAM_CHUNK == 0) {
		Terminal* tmp = NULL;
		size_t newSize = function->argCount + PARAM_CHUNK;
		if ((tmp = realloc(function->parameters, sizeof(Terminal) * newSize)) == NULL) {
			FatalError(ER_FATAL_INTERNAL);
		}
		function->parameters = tmp;
	}
	function->parameters[function->argCount++] = parameter;
}


void TableCleanup() {
	if (g_Nodes.allocated) {
		//Obe pole se alokuji spolecne, staci kontrolovat jedno
		Node* node;
		for (size_t i = 0; i < g_Nodes.used; i++) {
			node = g_Nodes.allocated[i];
			if (node->function && node->data.func.parameters != NULL) {
				free(node->data.func.parameters);
			}
		}
		for (size_t i = 0; i < g_Nodes.size; i += NODE_CHUNK) {
			//Na nasobcich NODE_CHUNK jsou ulozene pocatky alokovanych bloku pameti
			free(g_Nodes.allocated[i]);
		}
		free(g_Nodes.allocated);
		free(g_Nodes.unused);

		//Reinicializace pro pripadnou budouci alokaci
		g_Nodes.allocated = NULL;
		g_Nodes.unused = NULL;
		g_Nodes.used = g_Nodes.size = 0;
	}

	if (g_Tables.allocated) {
		for (size_t i = 0; i < g_Tables.size; i += TABLE_CHUNK) {
			//Na nasobcich TABLE_CHUNK jsou ulozeny ukazatele na pocatky alokovanych bloku pameti
			free(g_Tables.allocated[i]);
		}

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
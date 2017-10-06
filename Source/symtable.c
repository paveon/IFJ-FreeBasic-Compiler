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
#define MALLOC_SMALL_CHUNK 20
#define MALLOC_BIG_CHUNK 100

#ifdef DEBUG_INFO
static int scope_index = 0;
static int scope_level = 0;
#endif

typedef struct Node {
	uint64_t key;
	Identifier symbol;
	struct Node* treeLeft;
	struct Node* treeRight;
	struct Node* nextNode;
	int height;
} Node;

struct IDTable {
	struct IDTable* parentScope;
	struct IDTable* olderScope;
	Node* root;

#ifdef DEBUG_INFO
	int index;
#endif
};

typedef struct NodeStash {
	Node** nodeArray;
	size_t arraySize, arrayUsed;
} NodeStash;

typedef struct TableStash {
	IDTable** tableArray;
	size_t arraySize, arrayUsed;
} TableStash;

static NodeStash g_Nodes;
static TableStash g_Tables;


static IDTable g_GlobalSymbols;
static IDTable g_LocalSymbols;
static IDTable* g_ActiveScope = &g_LocalSymbols;


void ResizeTableStash(TableStash* stash) {
	IDTable** tmp = NULL;
	stash->arraySize += MALLOC_SMALL_CHUNK;
	if ((tmp = realloc(stash->tableArray, sizeof(IDTable*) * stash->arraySize)) == NULL) {
		FatalError(ER_FATAL_INTERNAL);
	}
	stash->tableArray = tmp;
}


void ResizeNodeStash(void) {
	Node** tmp = NULL;
	g_Nodes.arraySize += MALLOC_BIG_CHUNK;
	if ((tmp = realloc(g_Nodes.nodeArray, sizeof(Node*) * g_Nodes.arraySize)) == NULL) {
		FatalError(ER_FATAL_INTERNAL);
	}
	g_Nodes.nodeArray = tmp;
}


uint64_t HashFunction(const char* str) {
	uint64_t hash = 5381;
	int charCode;

	while ((charCode = *str++)) {
		hash = ((hash << 5) + hash) + charCode;
	}

	return hash;
}


void BeginSubScope() {
	IDTable* newTable = NULL;

	if ((newTable = malloc(sizeof(IDTable))) == NULL) {
		FatalError(ER_FATAL_INTERNAL);
	}
	if (g_Tables.arrayUsed == g_Tables.arraySize) {
		ResizeTableStash(&g_Tables);
	}
	newTable->root = NULL;
	newTable->parentScope = g_ActiveScope;
	newTable->olderScope = g_ActiveScope->olderScope;
	g_ActiveScope->olderScope = newTable;
	g_ActiveScope = newTable;

	g_Tables.tableArray[g_Tables.arrayUsed++] = newTable;


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


void EndSubScope() {
#ifdef DEBUG_INFO
	scope_level--;
	for (int i = 0; i < scope_level; i++) {
		printf("\t");
	}
	printf("End subscope %d\n\n", g_ActiveScope->index);
#endif

	if (g_ActiveScope->parentScope != NULL) {
		g_ActiveScope = g_ActiveScope->parentScope;
	}
}


void EndScope(void) {
	TableCleanup(false);
	g_LocalSymbols.root = NULL;
	g_LocalSymbols.olderScope = NULL;
	g_ActiveScope = &g_LocalSymbols;
}


Node* CreateNode(uint64_t key, const char* symbolName) {
	Node* newNode = NULL;

	if ((newNode = malloc(sizeof(Node))) == NULL) {
		FatalError(ER_FATAL_INTERNAL);
	}
	if (g_Nodes.arrayUsed == g_Nodes.arraySize) {
		ResizeNodeStash();
	}

	newNode->key = key;
	newNode->symbol.name = symbolName;
	newNode->symbol.idType = IDENTIFIER_LOCAL;
	LEFT(newNode) = RIGHT(newNode) = NEXT(newNode) = NULL;

	//TODO: casem mozna prepracovat na AVL?
	newNode->height = 0;

	g_Nodes.nodeArray[g_Nodes.arrayUsed++] = newNode;
	return newNode;
}


Identifier* InsertGlobalID(const char* name) {
	//ulozeni aktivniho lokalniho scopu
	IDTable* inActiveScope = g_ActiveScope;
	g_ActiveScope = &g_GlobalSymbols;

	Identifier* newSymbol = InsertLocalID(name);
	if (newSymbol != NULL) {
		newSymbol->idType = IDENTIFIER_GLOBAL;
	}

	//obnoveni scopu
	g_ActiveScope = inActiveScope;
	return newSymbol;
}


Identifier* InsertLocalID(const char* name) {
	if (!name) { return NULL; }

	//Pouzivam hashovani, protoze neustale porovnavani pripadne
	//velmi dlouhych identifikatoru je pomale a u stromu je riziko kolize minimalni
	uint64_t hash = HashFunction(name);

	if (g_ActiveScope->root == NULL) {
		return &(g_ActiveScope->root = CreateNode(hash, name))->symbol;
	}

	Node* current = g_ActiveScope->root;
	while (true) {
		if (hash < current->key) {
			if (LEFT(current)) {
				current = LEFT(current);
			}
			else {
				return &(LEFT(current) = CreateNode(hash, name))->symbol;
			}
		}
		else if (hash > current->key) {
			if (RIGHT(current)) {
				current = RIGHT(current);
			}
			else {
				return &(RIGHT(current) = CreateNode(hash, name))->symbol;
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

			return &(NEXT(previous) = CreateNode(hash, name))->symbol;
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
		if (g_ActiveScope != &g_GlobalSymbols) {
			table = table->parentScope;
		}
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


void TableCleanup(bool allNodes) {
	if (g_Nodes.nodeArray != NULL) {
		if (allNodes) {
			for (size_t i = 0; i < g_Nodes.arrayUsed; i++) {
				free(g_Nodes.nodeArray[i]);
			}
			g_Nodes.arraySize = g_Nodes.arrayUsed = 0;
			free(g_Nodes.nodeArray);
		}
		else {
			//Mazeme pouze lokalni symboly a preskladavame pole, abychom zachovali konzistenci
			for (size_t i = 0; i < g_Nodes.arrayUsed;) {
				if (g_Nodes.nodeArray[i]->symbol.idType == IDENTIFIER_LOCAL) {
					free(g_Nodes.nodeArray[i]);
					g_Nodes.nodeArray[i] = g_Nodes.nodeArray[g_Nodes.arrayUsed - 1];
					g_Nodes.nodeArray[g_Nodes.arrayUsed - 1] = NULL;
					g_Nodes.arrayUsed--;
				}
				else {
					i++;
				}
			}
		}
	}
	if (g_Tables.tableArray != NULL) {
		for (size_t i = 0; i < g_Tables.arrayUsed; i++) {
			free(g_Tables.tableArray[i]);

		}
		g_Tables.arrayUsed = 0;
		if (allNodes) {
			free(g_Tables.tableArray);
			g_Tables.arraySize = 0;
		}
	}
}
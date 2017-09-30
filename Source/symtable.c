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
	Symbol symbol;
	struct Node* treeLeft;
	struct Node* treeRight;
	struct Node* nextNode;
	int height;
} Node;

struct SymbolTable {
	struct SymbolTable* parentScope;
	struct SymbolTable* olderScope;
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
	SymbolTable** tableArray;
	size_t arraySize, arrayUsed;
} TableStash;

static NodeStash Nodes;
static TableStash Tables;


static SymbolTable GlobalSymbols;
static SymbolTable LocalSymbols;
static SymbolTable* ActiveScope = &LocalSymbols;


void ResizeTableStash(TableStash* stash) {
	SymbolTable** tmp = NULL;
	stash->arraySize += MALLOC_SMALL_CHUNK;
	if ((tmp = realloc(stash->tableArray, sizeof(SymbolTable*) * stash->arraySize)) == NULL) {
		FatalError(ER_FATAL_INTERNAL);
	}
	stash->tableArray = tmp;
}


void ResizeNodeStash(void) {
	Node** tmp = NULL;
	Nodes.arraySize += MALLOC_BIG_CHUNK;
	if ((tmp = realloc(Nodes.nodeArray, sizeof(Node*) * Nodes.arraySize)) == NULL) {
		FatalError(ER_FATAL_INTERNAL);
	}
	Nodes.nodeArray = tmp;
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
	SymbolTable* newTable = NULL;

	if ((newTable = malloc(sizeof(SymbolTable))) == NULL) {
		FatalError(ER_FATAL_INTERNAL);
	}
	if (Tables.arrayUsed == Tables.arraySize) {
		ResizeTableStash(&Tables);
	}
	newTable->root = NULL;
	newTable->parentScope = ActiveScope;
	newTable->olderScope = ActiveScope->olderScope;
	ActiveScope->olderScope = newTable;
	ActiveScope = newTable;

	Tables.tableArray[Tables.arrayUsed++] = newTable;


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
	printf("End subscope %d\n\n", ActiveScope->index);
#endif

	if (ActiveScope->parentScope != NULL) {
		ActiveScope = ActiveScope->parentScope;
	}
}


void EndScope(void) {
	TableCleanup(false);
	LocalSymbols.root = NULL;
	LocalSymbols.olderScope = NULL;
	ActiveScope = &LocalSymbols;
}


Node* CreateNode(uint64_t key, const char* symbolName) {
	Node* newNode = NULL;

	if ((newNode = malloc(sizeof(Node))) == NULL) {
		FatalError(ER_FATAL_INTERNAL);
	}
	if (Nodes.arrayUsed == Nodes.arraySize) {
		ResizeNodeStash();
	}

	newNode->key = key;
	newNode->symbol.name = symbolName;
	newNode->symbol.symbolType = SYMBOL_LOCAL;
	LEFT(newNode) = RIGHT(newNode) = NEXT(newNode) = NULL;

	//TODO: casem mozna prepracovat na AVL?
	newNode->height = 0;

	Nodes.nodeArray[Nodes.arrayUsed++] = newNode;
	return newNode;
}


Symbol* InsertGlobalSymbol(const char* name) {
	//ulozeni aktivniho lokalniho scopu
	SymbolTable* inActiveScope = ActiveScope;
	ActiveScope = &GlobalSymbols;

	Symbol* newSymbol = InsertSymbol(name);
	if (newSymbol != NULL) {
		newSymbol->symbolType = SYMBOL_GLOBAL;
	}

	//obnoveni scopu
	ActiveScope = inActiveScope;
	return newSymbol;
}


Symbol* InsertSymbol(const char* name) {
	if (!name) { return NULL; }

	//Pouzivam hashovani, protoze neustale porovnavani pripadne
	//velmi dlouhych identifikatoru je pomale a u stromu je riziko kolize minimalni
	uint64_t hash = HashFunction(name);

	if (ActiveScope->root == NULL) {
		return &(ActiveScope->root = CreateNode(hash, name))->symbol;
	}

	Node* current = ActiveScope->root;
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


Symbol* LookupSymbol(const char* symbol) {
	if (!symbol) { return NULL; }

	uint64_t hash = HashFunction(symbol);
	Node* current;

	SymbolTable* table = ActiveScope;
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
		if (ActiveScope != &GlobalSymbols) {
			table = table->parentScope;
		}
	}

	return NULL;
}


Symbol* LookupGlobalSymbol(const char* name) {
	//Ulozeni aktivniho lokalniho scopu
	SymbolTable* inActiveScope = ActiveScope;
	ActiveScope = &GlobalSymbols;

	Symbol* symbol = LookupSymbol(name);

	//Obnoveni scopu
	ActiveScope = inActiveScope;
	return symbol;
}


void TableCleanup(bool allNodes) {
	if (Nodes.nodeArray != NULL) {
		if (allNodes) {
			for (size_t i = 0; i < Nodes.arrayUsed; i++) {
				free(Nodes.nodeArray[i]);
			}
			Nodes.arraySize = Nodes.arrayUsed = 0;
			free(Nodes.nodeArray);
		}
		else {
			//Mazeme pouze lokalni symboly a preskladavame pole, abychom zachovali konzistenci
			for (size_t i = 0; i < Nodes.arrayUsed;) {
				if (Nodes.nodeArray[i]->symbol.symbolType == SYMBOL_LOCAL) {
					free(Nodes.nodeArray[i]);
					Nodes.nodeArray[i] = Nodes.nodeArray[Nodes.arrayUsed - 1];
					Nodes.nodeArray[Nodes.arrayUsed - 1] = NULL;
					Nodes.arrayUsed--;
				}
				else {
					i++;
				}
			}
		}
	}
	if (Tables.tableArray != NULL) {
		for (size_t i = 0; i < Tables.arrayUsed; i++) {
			free(Tables.tableArray[i]);

		}
		Tables.arrayUsed = 0;
		if (allNodes) {
			free(Tables.tableArray);
			Tables.arraySize = 0;
		}
	}
}
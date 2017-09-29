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


typedef struct Node {
	uint64_t key;
	Symbol symbol;
	struct Node* treeLeft;
	struct Node* treeRight;
	struct Node* nextNode;
	int height;
} Node;

struct SymbolTable {
	struct SymbolTable* parentTable;
	struct SymbolTable* previousSubscope;
	Node* root;
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


static SymbolTable* Functions;
static SymbolTable* MainScope;

//TODO: budeme podporovat globalni promenne, mozne budouci rozsireni?
//static SymbolTable* GlobalScope;


uint64_t HashFunction(const char* str) {
	uint64_t hash = 5381;
	int charCode;

	while ((charCode = *str++)) {
		hash = ((hash << 5) + hash) + charCode;
	}

	return hash;
}

void TableCleanup(void) {
	if (Nodes.nodeArray != NULL) {
		for (size_t i = 0; i < Nodes.arrayUsed; i++) {
			free(Nodes.nodeArray[i]);
		}
		free(Nodes.nodeArray);
	}
	if (Tables.tableArray != NULL) {
		for (size_t i = 0; i < Tables.arrayUsed; i++) {
			free(Tables.tableArray[i]);
		}
		free(Tables.tableArray);
	}
	if (Functions != NULL) { free(Functions); }
	if (MainScope != NULL) { free(MainScope); }
}

void CreateBaseScopes(void) {
	if (Functions || MainScope) { return; }

	if ((Functions = malloc(sizeof(SymbolTable))) == NULL ||
	    (MainScope = malloc(sizeof(SymbolTable))) == NULL) {
		FatalError(ER_FATAL_INTERNAL);
	}

	MainScope->parentTable = MainScope->previousSubscope = NULL;
	Functions->parentTable = Functions->previousSubscope = NULL;
	Functions->root = MainScope->root = NULL;
}

SymbolTable* GetMainScope(void) {
	if (MainScope == NULL) {
		CreateBaseScopes();
	}
	return MainScope;
}

SymbolTable* GetFuncScope(void) {
	if (Functions == NULL) {
		CreateBaseScopes();
	}
	return Functions;
}

SymbolTable* CreateScope(SymbolTable* parentScope) {
	SymbolTable* newTable = NULL;

	if ((newTable = malloc(sizeof(SymbolTable))) == NULL) {
		FatalError(ER_FATAL_INTERNAL);
	}
	if (Tables.arrayUsed == Tables.arraySize) {
		SymbolTable** tmp = NULL;
		Tables.arraySize += MALLOC_SMALL_CHUNK;
		if ((tmp = realloc(Tables.tableArray, sizeof(SymbolTable*) * Tables.arraySize)) == NULL) {
			FatalError(ER_FATAL_INTERNAL);
		}
		Tables.tableArray = tmp;
	}
	newTable->root = NULL;

	//Defaultni parent scope je main scope
	if (parentScope == NULL) {
		parentScope = GetMainScope();
	}
	else {
		newTable->parentTable = parentScope;
	}

	//Tabulka muze mit pouze jeden aktivni subscope, stare tabulky uschovame v seznamu
	newTable->previousSubscope = parentScope->previousSubscope;
	parentScope->previousSubscope = newTable;

	Tables.tableArray[Tables.arrayUsed++] = newTable;
	return newTable;
}


Node* CreateNode(uint64_t key, const char* symbolName) {
	Node* newNode = NULL;

	if ((newNode = malloc(sizeof(Node))) == NULL) {
		FatalError(ER_FATAL_INTERNAL);
	}
	if (Nodes.arrayUsed == Nodes.arraySize) {
		Node** tmp = NULL;
		Nodes.arraySize += MALLOC_BIG_CHUNK;
		if ((tmp = realloc(Nodes.nodeArray, sizeof(Node*) * Nodes.arraySize)) == NULL) {
			FatalError(ER_FATAL_INTERNAL);
		}
		Nodes.nodeArray = tmp;
	}

	newNode->key = key;
	newNode->symbol.name = symbolName;
	LEFT(newNode) = RIGHT(newNode) = NEXT(newNode) = NULL;

	//TODO: casem mozna prepracovat na AVL?
	newNode->height = 0;

	Nodes.nodeArray[Nodes.arrayUsed++] = newNode;
	return newNode;
}

bool InsertSymbol(SymbolTable* table, const char* symbol) {
	if (!table || !symbol) { return false; }

	//Pouzivam hashovani, protoze neustale porovnavani pripadne
	//velmi dlouhych identifikatoru je pomale a u stromu je riziko kolize minimalni
	uint64_t hash = HashFunction(symbol);

	if (table->root == NULL) {
		table->root = CreateNode(hash, symbol);
		return true;
	}

	Node* current = table->root;
	while (true) {
		if (hash < current->key) {
			if (LEFT(current)) {
				current = LEFT(current);
			}
			else {
				LEFT(current) = CreateNode(hash, symbol);
				return true;
			}
		}
		else if (hash > current->key) {
			if (RIGHT(current)) {
				current = RIGHT(current);
			}
			else {
				RIGHT(current) = CreateNode(hash, symbol);
				return true;
			}
		}
		else {
			//Stejny hash, nutna blizsi kontrola
			Node* previous = NULL;

			while (current) {
				if (strlen(symbol) == strlen(current->symbol.name) &&
				    strcmp(symbol, current->symbol.name) == 0) {
					return false;
				}
				previous = current;
				current = NEXT(current);
			}

			NEXT(previous) = CreateNode(hash, symbol);
			return true;
		}
	}
}


Symbol* LookupSymbol(SymbolTable* table, const char* symbol) {
	if (!table || !symbol) { return NULL; }

	uint64_t hash = HashFunction(symbol);
	Node* current = table->root;

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
					break;
				}
				current = NEXT(current);
			}
			break;
		}
	}

	if (current) {
		return &current->symbol;
	}
	return NULL;
}
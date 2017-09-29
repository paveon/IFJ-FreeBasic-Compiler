#ifndef FREEBASIC_COMPILER_SYMTABLE_H
#define FREEBASIC_COMPILER_SYMTABLE_H

#include <stdbool.h>

typedef struct Symbol {
	//TODO: popremyslet, co vsechno bude semanticky analyzator potrebovat
	int type;
	const char* name;
} Symbol;

typedef struct SymbolTable SymbolTable;

SymbolTable* GetMainScope(void);

SymbolTable* GetFuncScope(void);

SymbolTable* CreateScope(SymbolTable* parentScope);

bool InsertSymbol(SymbolTable* table, const char* symbol);

Symbol* LookupSymbol(SymbolTable* table, const char* symbol);


/* Interni funkce pro uvolneni alokovane pameti souvisejici s tabulkami symbolu. !!NEPOUZIVAT!! */
void TableCleanup(void);

#endif //FREEBASIC_COMPILER_SYMTABLE_H

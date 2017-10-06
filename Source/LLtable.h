#ifndef FREEBASIC_COMPILER_LLTABLE_H
#define FREEBASIC_COMPILER_LLTABLE_H

#include <stdbool.h>
#include "Token.h"

typedef enum NTerminal {
	NT_PROGRAM = 0,
	NT_HEADER = 1,
	NT_FUNCTION = 2,
	NT_ARGUMENT = 3,
	NT_NEXT_ARGUMENT = 4,
	NT_STATEMENT_LIST = 5,
	NT_STATEMENT = 6,
	NT_ELSE = 7,
	NT_NEXT_EXPRESSION = 8,
	NT_INITIALIZATION = 9,
	NT_TYPE = 10
} NTerminal;

typedef enum Terminal {
	T_DECLARE,
	T_DIM,
	T_DO,
	T_DOUBLE,
	T_ELSE,
	T_END,
	T_FUNCTION,
	T_IF,
	T_INPUT,
	T_INTEGER,
	T_LOOP,
	T_PRINT,
	T_RETURN,
	T_SCOPE,
	T_STRING,
	T_COMMA,
	T_RIGHT_BRACKET,
	T_EQUAL,
	T_EOL,
	T_ID,
	T_LEFT_BRACKET,
	T_AS,
	T_WHILE,
	T_THEN,
	T_SEMICOLON
} Terminal;

typedef enum SymbolType {
	SYMBOL_UNDEFINED,
	SYMBOL_BOTTOM,
	SYMBOL_TERMINAL,
	SYMBOL_NONTERMINAL
} SymbolType;

typedef struct Symbol {
	struct Symbol* up;
	struct Symbol* down;
	bool reduceEnd;
	bool used; //Uvolnit lze pouze pouzivany symbol
	SymbolType type;

	//Nepotrebujeme vyuzivat obe promenne zaraz
	union {
		Terminal terminal;
		NTerminal nonTerminal;
	} data;
} Symbol;



/* Interni funkce, ocekava jiz osetrene vstupy --NEPOUZIVAT-- */
bool ProduceString(Symbol* nonterm, Token* token, Symbol** stackTop);

void CreateString(Symbol** stackTop, size_t length);


#endif //FREEBASIC_COMPILER_LLTABLE_H

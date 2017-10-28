#ifndef FREEBASIC_COMPILER_STACKSYMBOL_H
#define FREEBASIC_COMPILER_STACKSYMBOL_H

#include <stdbool.h>

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
	NT_TYPE = 10,
	NT_LINE_BREAK = 11,
	NT_EXPRESSION,
	NT_STRING,
	NT_UNDEFINED
} NTerminal;

typedef enum Terminal {
	T_DECLARE = 0,
	T_DIM = 1,
	T_DO = 2,
	T_DOUBLE = 3,
	T_ELSE = 4,
	T_END = 5,
	T_FUNCTION = 6,
	T_IF = 7,
	T_INPUT = 8,
	T_INTEGER = 9,
	T_LOOP = 10,
	T_PRINT = 11,
	T_RETURN = 12,
	T_SCOPE = 13,
	T_STRING = 14,
	T_COMMA = 15,
	T_RIGHT_BRACKET = 16,
	T_EQUAL = 17,
	T_EOL = 18,
	T_ID = 19,
	T_EOF = 20,
	T_LEFT_BRACKET = 21,
	T_AS = 22,
	T_WHILE = 23,
	T_THEN = 24,
	T_SEMICOLON = 25,
	T_BOTTOM,
	T_OPERATOR_PLUS,
	T_OPERATOR_MINUS,
	T_OPERATOR_MULTIPLY,
	T_OPERATOR_REAL_DIVIDE,
	T_OPERATOR_INT_DIVIDE,
	T_OPERATOR_LESS,
	T_OPERATOR_LESS_EQ,
	T_OPERATOR_GRT,
	T_OPERATOR_GRT_EQ,
	T_OPERATOR_NOT_EQ,
	T_UNDEFINED
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
	SymbolType type;

	//Nepotrebujeme vyuzivat obe promenne zaraz
	union {
		Terminal terminal;
		NTerminal nonTerminal;
	} data;
} Symbol;

#endif //FREEBASIC_COMPILER_STACKSYMBOL_H

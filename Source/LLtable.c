#include <stdlib.h>
#include <string.h>
#include "LLtable.h"

#define ITEMS_TOTAL 36
#define ITEMS_PREDICT 20

/* Nemenit poradi, zavisi na tom indexovani. Lepsi zpusob me bohuzel zatim nenapadl */
static const char* const Terminals[] = {
		  "DECLARE", "DIM", "DO", "DOUBLE", "ELSE", "END", "FUNCTION", "IF",
		  "INPUT", "INTEGER", "LOOP", "PRINT", "RETURN", "SCOPE", "STRING",
		  ",", ")", "=", "\n", "ID", "(", "AS", "WHILE", "THEN", ";"
};

static Rule LLTable[11][20] = {
		  {2, 0,  0,  0,  0,  0,  3, 0,  0,  0,  0,  0,  0,  1, 0,  0, 0,  0,  0,  0},
		  {0, 0,  0,  0,  0,  0,  4, 0,  0,  0,  0,  0,  0,  0, 0,  0, 0,  0,  0,  0},
		  {0, 0,  0,  0,  0,  0,  5, 0,  0,  0,  0,  0,  0,  0, 0,  0, 0,  0,  0,  0},
		  {0, 0,  0,  0,  0,  0,  0, 0,  0,  0,  0,  0,  0,  0, 0,  0, 27, 0,  0,  6},
		  {0, 0,  0,  0,  0,  0,  0, 0,  0,  0,  0,  0,  0,  0, 0,  7, 8,  0,  0,  0},
		  {0, 9,  9,  0,  10, 10, 0, 9,  9,  0,  10, 9,  9,  0, 0,  0, 0,  0,  0,  9},
		  {0, 11, 15, 0,  0,  0,  0, 17, 13, 0,  0,  14, 16, 0, 0,  0, 0,  0,  0,  12},
		  {0, 0,  0,  0,  18, 19, 0, 0,  0,  0,  0,  0,  0,  0, 0,  0, 0,  0,  0,  0},
		  {0, 0,  0,  0,  0,  0,  0, 0,  0,  0,  0,  0,  0,  0, 0,  0, 0,  0,  21, 0},
		  {0, 0,  0,  0,  0,  0,  0, 0,  0,  0,  0,  0,  0,  0, 0,  0, 0,  22, 23, 0},
		  {0, 0,  0,  26, 0,  0,  0, 0,  0,  25, 0,  0,  0,  0, 24, 0, 0,  0,  0,  0}
};

const char* GetTerminalValue(Terminal terminal) {
	return Terminals[terminal];
}

Rule GetLLRule(NTerminal nTerminal, const Token* token) {
	TokenType type = GetTokenType(token);
	const void* value = GetTokenValue(token);
	size_t column = 0;

	switch (type) {
		case TOKEN_IDENTIFIER:
			column = T_ID;
			break;

		case TOKEN_OPERATOR:
			if (strcmp(value, "=") != 0) {
				return false;
			}
			column = T_EQUAL;
			break;

		case TOKEN_R_BRACKET:
			column = T_RIGHT_BRACKET;
			break;

		case TOKEN_EOL:
			column = T_EOL;
			break;

		case TOKEN_KEYWORD:
			for (size_t i = 0; i < ITEMS_PREDICT - 1; i++) {
				if (strcmp(value, Terminals[i]) == 0) {
					column = i;
					break;
				}
			}
			break;

		default:
			return false;
	}

	return LLTable[nTerminal][column];
}
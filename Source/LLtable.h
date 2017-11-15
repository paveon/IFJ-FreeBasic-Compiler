#ifndef FREEBASIC_COMPILER_LLTABLE_H
#define FREEBASIC_COMPILER_LLTABLE_H

#include <stdbool.h>
#include "Symbol.h"
#include "Token.h"

typedef enum Rules {
	RULE_MISSING, 			// 0
	RULE_EPSILON, 			// 1
	RULE_MAIN_SCOPE,		// 2
	RULE_FUNC_DECL,			// 3
	RULE_FUNC_DEF,			// 4
	RULE_FUNC_HEADER,		// 5
	RULE_FUNC_BODY,			// 6
	RULE_FUNC_ARG,			// 7
	RULE_FUNC_NEXT_ARG,	// 8
	RULE_ST_LIST,				// 9
	RULE_ST_VAR_DECL,		// 10
	RULE_ST_VAR_STATIC,	// 11
	RULE_ST_ASSIGNMENT,	// 12
	RULE_ST_INPUT,			// 13
	RULE_ST_PRINT,			// 14
	RULE_ST_WHILE,			// 15
	RULE_ST_RETURN,			// 16
	RULE_ST_IF,					// 17
	RULE_ELSEIF,				// 18
	RULE_ELSE,					// 19
	RULE_END_IF,				// 20
	RULE_NEXT_EXPR,			// 21
	RULE_VAR_INIT,			// 22
	RULE_TYPE_STRING,		// 23
	RULE_TYPE_INT,			// 24
	RULE_TYPE_DOUBLE,		// 25
	RULE_LINE_BREAK,		// 26
	RULE_VAR_GLOBAL,		// 27
	RULE_NEW_SCOPE,			// 28
	RULE_OP_EQ,					// 29
	RULE_OP_PLUS_EQ,		// 30
	RULE_OP_MINUS_EQ,		// 31
	RULE_OP_MULTIPLY_EQ,// 32
	RULE_OP_INT_DIV_EQ,	// 33
	RULE_OP_REAL_DIV_EQ,  // 34
	RULE_ST_WHILE_END //35 - umele pravidlo pro rozpoznani konce while smycky
} Rules;

typedef unsigned char Rule;

Rule GetLLRule(NTerminal nTerminal, Terminal tokenTerminal);

//const char* GetTerminalValue(Terminal terminal);


#endif //FREEBASIC_COMPILER_LLTABLE_H

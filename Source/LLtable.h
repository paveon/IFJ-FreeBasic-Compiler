#ifndef FREEBASIC_COMPILER_LLTABLE_H
#define FREEBASIC_COMPILER_LLTABLE_H

#include <stdbool.h>
#include "Symbol.h"
#include "Token.h"

typedef enum Rules {
	RULE_MAIN_SCOPE = 1,
	RULE_FUNC_DECL = 2,
	RULE_FUNC_DEF = 3,
	RULE_FUNC_HEADER = 4,
	RULE_FUNC_BODY = 5,
	RULE_FUNC_ARG = 6,
	RULE_FUNC_NEXT_ARG = 7,
	RULE_ST_LIST = 9,
	RULE_ST_VAR_DECL = 11,
	RULE_ST_FUNC_CALL = 12,
	RULE_ST_INPUT = 13,
	RULE_ST_PRINT = 14,
	RULE_ST_WHILE = 15,
	RULE_ST_RETURN = 16,
	RULE_ST_IF = 17,
	RULE_ST_ELSE = 18,
	RULE_ST_END_IF = 19,
	RULE_NEXT_EXPR = 20,
	RULE_VAR_INIT = 22,
	RULE_TYPE_STRING = 24,
	RULE_TYPE_INT = 25,
	RULE_TYPE_DOUBLE = 26,
	RULE_LINE_BREAK = 28,
	RULE_VAR_GLOBAL = 30,
	RULE_ST_VAR_STATIC = 31,
	RULE_NEW_SCOPE = 32
} Rules;

typedef unsigned char Rule;

Rule GetLLRule(NTerminal nTerminal, Terminal tokenTerminal);

//const char* GetTerminalValue(Terminal terminal);


#endif //FREEBASIC_COMPILER_LLTABLE_H

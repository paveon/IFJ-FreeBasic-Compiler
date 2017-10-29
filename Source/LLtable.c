#include <stdlib.h>
#include <string.h>
#include "LLtable.h"


//TODO: dodelat scope pravidla
//TODO: dodelat pravidla pro globalni promenne

typedef struct Pair {
	Terminal terminal;
	Rule ruleNumber;
} Pair;

static Pair g_NT_PROGRAM[] = {
				{T_DECLARE,   RULE_FUNC_DECL},
				{T_FUNCTION,  RULE_FUNC_DEF},
				{T_SCOPE,     RULE_MAIN_SCOPE},
				{T_DIM,       RULE_VAR_GLOBAL},
				{T_UNDEFINED, RULE_MISSING}
};
static Pair g_NT_SCOPE[] = {
				{T_SCOPE,     RULE_NEW_SCOPE},
				{T_UNDEFINED, RULE_MISSING}
};
static Pair g_NT_HEADER[] = {
				{T_FUNCTION,  RULE_FUNC_HEADER},
				{T_UNDEFINED, RULE_MISSING}
};
static Pair g_NT_FUNCTION[] = {
				{T_FUNCTION,  RULE_FUNC_BODY},
				{T_UNDEFINED, RULE_MISSING}
};
static Pair g_NT_ARGUMENT[] = {
				{T_ID,            RULE_FUNC_ARG},
				{T_RIGHT_BRACKET, RULE_EPSILON},
				{T_UNDEFINED,     RULE_MISSING}
};
static Pair g_NT_NEXT_ARGUMENT[] = {
				{T_COMMA,         RULE_FUNC_NEXT_ARG},
				{T_RIGHT_BRACKET, RULE_EPSILON},
				{T_UNDEFINED,     RULE_MISSING}
};
static Pair g_NT_STATEMENT_LIST[] = {
				{T_DIM,       RULE_ST_LIST},
				{T_STATIC,    RULE_ST_LIST},
				{T_SCOPE,     RULE_ST_LIST},
				{T_ID,        RULE_ST_LIST},
				{T_DO,        RULE_ST_LIST},
				{T_IF,        RULE_ST_LIST},
				{T_INPUT,     RULE_ST_LIST},
				{T_PRINT,     RULE_ST_LIST},
				{T_RETURN,    RULE_ST_LIST},
				{T_LOOP,      RULE_EPSILON},
				{T_ELSE,      RULE_EPSILON},
				{T_END,       RULE_EPSILON},
				{T_ELSEIF,    RULE_EPSILON},
				{T_UNDEFINED, RULE_MISSING}
};
static Pair g_NT_STATEMENT[] = {
				{T_DIM,       RULE_ST_VAR_DECL},
				{T_STATIC,    RULE_ST_VAR_STATIC},
				{T_SCOPE,     RULE_NEW_SCOPE},
				{T_ID,        RULE_ST_FUNC_CALL},
				{T_IF,        RULE_ST_IF},
				{T_DO,        RULE_ST_WHILE},
				{T_INPUT,     RULE_ST_INPUT},
				{T_PRINT,     RULE_ST_PRINT},
				{T_RETURN,    RULE_ST_RETURN},
				{T_UNDEFINED, RULE_MISSING}
};
static Pair g_NT_ELSEIF[] = {
				{T_ELSEIF,    RULE_ELSEIF},
				{T_ELSE,      RULE_EPSILON},
				{T_END,       RULE_EPSILON},
				{T_UNDEFINED, RULE_MISSING}
};
static Pair g_NT_ELSE[] = {
				{T_ELSE,      RULE_ELSE},
				{T_END,       RULE_END_IF},
				{T_UNDEFINED, RULE_MISSING}
};
static Pair g_NT_NEXT_EXPRESSION[] = {
				{T_EOL,       RULE_EPSILON}, //epsilon
				{T_UNDEFINED, RULE_MISSING}
};
static Pair g_NT_INITIALIZATION[] = {
				{T_OPERATOR_EQUAL, RULE_VAR_INIT},
				{T_EOL,            RULE_EPSILON}, //epsilon
				{T_UNDEFINED,      RULE_MISSING}
};
static Pair g_NT_TYPE[] = {
				{T_DOUBLE,    RULE_TYPE_DOUBLE},
				{T_INTEGER,   RULE_TYPE_INT},
				{T_STRING,    RULE_TYPE_STRING},
				{T_UNDEFINED, RULE_MISSING}
};
static Pair g_NT_LINE_BREAK[] = {
				{T_EOL,       RULE_LINE_BREAK},
				{T_DECLARE,   RULE_EPSILON},
				{T_DIM,       RULE_EPSILON},
				{T_STATIC,    RULE_EPSILON},
				{T_DO,        RULE_EPSILON},
				{T_ELSE,      RULE_EPSILON},
				{T_ELSEIF,    RULE_EPSILON},
				{T_END,       RULE_EPSILON},
				{T_FUNCTION,  RULE_EPSILON},
				{T_IF,        RULE_EPSILON},
				{T_INPUT,     RULE_EPSILON},
				{T_LOOP,      RULE_EPSILON},
				{T_PRINT,     RULE_EPSILON},
				{T_RETURN,    RULE_EPSILON},
				{T_SCOPE,     RULE_EPSILON},
				{T_ID,        RULE_EPSILON},
				{T_EOF,       RULE_EPSILON},
				{T_UNDEFINED, RULE_MISSING}
};


Rule GetLLRule(NTerminal nTerminal, Terminal tokenTerminal) {
	Pair* rules;

	switch (nTerminal) {
		case NT_PROGRAM:
			rules = &g_NT_PROGRAM[0];
			break;
		case NT_SCOPE:
			rules = &g_NT_SCOPE[0];
			break;
		case NT_HEADER:
			rules = &g_NT_HEADER[0];
			break;
		case NT_FUNCTION:
			rules = &g_NT_FUNCTION[0];
			break;
		case NT_ARGUMENT:
			rules = &g_NT_ARGUMENT[0];
			break;
		case NT_NEXT_ARGUMENT:
			rules = &g_NT_NEXT_ARGUMENT[0];
			break;
		case NT_STATEMENT_LIST:
			rules = &g_NT_STATEMENT_LIST[0];
			break;
		case NT_STATEMENT:
			rules = &g_NT_STATEMENT[0];
			break;
		case NT_ELSEIF:
			rules = &g_NT_ELSEIF[0];
			break;
		case NT_ELSE:
			rules = &g_NT_ELSE[0];
			break;
		case NT_NEXT_EXPRESSION:
			rules = &g_NT_NEXT_EXPRESSION[0];
			break;
		case NT_INITIALIZATION:
			rules = &g_NT_INITIALIZATION[0];
			break;
		case NT_TYPE:
			rules = &g_NT_TYPE[0];
			break;
		case NT_LINE_BREAK:
			rules = &g_NT_LINE_BREAK[0];
			break;

		default:
			return RULE_MISSING;
	}

	size_t index = 0;
	while (rules[index].terminal != T_UNDEFINED) {
		if (rules[index].terminal == tokenTerminal) {
			return rules[index].ruleNumber;
		}
		index++;
	}
	return RULE_MISSING;
}
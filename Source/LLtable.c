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
				{T_DECLARE, RULE_FUNC_DECL},
				{T_FUNCTION, RULE_FUNC_DEF},
				{T_SCOPE, RULE_MAIN_SCOPE},
				{T_DIM, RULE_VAR_GLOBAL},
				{T_UNDEFINED, 0}
};
static Pair g_NT_SCOPE[] = {
				{T_SCOPE, RULE_NEW_SCOPE},
				{T_UNDEFINED, 0}
};
static Pair g_NT_HEADER[] = {
				{T_FUNCTION, RULE_FUNC_HEADER},
				{T_UNDEFINED, 0}
};
static Pair g_NT_FUNCTION[] = {
				{T_FUNCTION, RULE_FUNC_BODY},
				{T_UNDEFINED, 0}
};
static Pair g_NT_ARGUMENT[] = {
				{T_ID, RULE_FUNC_ARG},
				{T_RIGHT_BRACKET, 27}, //epsilon
				{T_UNDEFINED, 0}
};
static Pair g_NT_NEXT_ARGUMENT[] = {
				{T_COMMA, RULE_FUNC_NEXT_ARG},
				{T_RIGHT_BRACKET, 8}, //epsilon
				{T_UNDEFINED, 0}
};
static Pair g_NT_STATEMENT_LIST[] = {
				{T_DIM, RULE_ST_LIST},
				{T_STATIC, RULE_ST_LIST},
				{T_SCOPE, RULE_ST_LIST},
				{T_ID, RULE_ST_LIST},
				{T_DO, RULE_ST_LIST},
				{T_IF, RULE_ST_LIST},
				{T_ELSE, 10}, //epsilon
				{T_END, 10}, //epsilon
				{T_INPUT, RULE_ST_LIST},
				{T_LOOP, 10}, //epsilon
				{T_PRINT, RULE_ST_LIST},
				{T_RETURN, RULE_ST_LIST},
				{T_UNDEFINED, 0}
};
static Pair g_NT_STATEMENT[] = {
				{T_DIM, RULE_ST_VAR_DECL},
				{T_STATIC, RULE_ST_VAR_STATIC},
				{T_SCOPE, RULE_NEW_SCOPE},
				{T_ID, RULE_ST_FUNC_CALL},
				{T_IF, RULE_ST_IF},
				{T_DO, RULE_ST_WHILE},
				{T_INPUT, RULE_ST_INPUT},
				{T_PRINT, RULE_ST_PRINT},
				{T_RETURN, RULE_ST_RETURN},
				{T_UNDEFINED, 0}
};
static Pair g_NT_ELSE[] = {
				{T_ELSE, RULE_ST_ELSE},
				{T_END, RULE_ST_END_IF},
				{T_UNDEFINED, 0}
};
static Pair g_NT_NEXT_EXPRESSION[] = {
				{T_EOL, 21}, //epsilon
				{T_UNDEFINED, 0}
};
static Pair g_NT_INITIALIZATION[] = {
				{T_OPERATOR_EQUAL, RULE_VAR_INIT},
				{T_EOL, 23}, //epsilon
				{T_UNDEFINED, 0}
};
static Pair g_NT_TYPE[] = {
				{T_DOUBLE, RULE_TYPE_DOUBLE},
				{T_INTEGER, RULE_TYPE_INT},
				{T_STRING, RULE_TYPE_STRING},
				{T_UNDEFINED, 0}
};
static Pair g_NT_LINE_BREAK[] = {
				{T_EOL, RULE_LINE_BREAK},

				/* Epsilon pravidlo */
				{T_DECLARE, 29},
				{T_DIM, 29},
				{T_STATIC, 29},
				{T_DO, 29},
				{T_ELSE, 29},
				{T_END, 29},
				{T_FUNCTION, 29},
				{T_IF, 29},
				{T_INPUT, 29},
				{T_LOOP, 29},
				{T_PRINT, 29},
				{T_RETURN, 29},
				{T_SCOPE, 29},
				{T_ID, 29},
				{T_EOF, 29},
				{T_UNDEFINED, 0}
};

//static unsigned char g_Rules[NONTERMINALS][TERMINALS] = {
//				{2,  0,  0,  0,  0,  0,  3,  0,  0,  0,  0,  0,  0,  1,  0,  0, 0,  0,  0,  0,  0},
//				{0,  0,  0,  0,  0,  0,  4,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0,  0,  0,  0,  0},
//				{0,  0,  0,  0,  0,  0,  5,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0,  0,  0,  0,  0},
//				{0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 27, 0,  0,  6,  0},
//				{0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  7, 8,  0,  0,  0,  0},
//				{0,  9,  9,  0,  10, 10, 0,  9,  9,  0,  10, 9,  9,  0,  0,  0, 0,  0,  0,  9,  0},
//				{0,  11, 15, 0,  0,  0,  0,  17, 13, 0,  0,  14, 16, 0,  0,  0, 0,  0,  0,  12, 0},
//				{0,  0,  0,  0,  18, 19, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0,  0,  0,  0,  0},
//				{0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0,  0,  21, 0,  0},
//				{0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0,  22, 23, 0,  0},
//				{0,  0,  0,  26, 0,  0,  0,  0,  0,  25, 0,  0,  0,  0,  24, 0, 0,  0,  0,  0,  0},
//				{29, 29, 29, 0,  29, 29, 29, 29, 29, 0,  29, 29, 29, 29, 0,  0, 0,  0,  28, 29, 29},
//};

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
			return 0;
	}

	size_t index = 0;
	while (rules[index].terminal != T_UNDEFINED) {
		if (rules[index].terminal == tokenTerminal) {
			return rules[index].ruleNumber;
		}
		index++;
	}
	return 0;
}
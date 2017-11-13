#include <stdio.h>
#include <stdlib.h>
#include "CodeGenerator.h"
#include "CompilationErrors.h"
#include "LLtable.h"
#include "symtable.h"
#include <string.h>

#define RULE_CHUNK 100
#define CODE_CHUNK 1024

typedef unsigned char uchar;

typedef struct Buffer {
	uchar* buffer;
	size_t size;
	size_t used;
} Buffer;


typedef struct TokenArray {
	Token** array;
	size_t used;
	size_t size;
} TokenArray;

typedef struct Labels {
	unsigned int* labels;
	unsigned int size;
	unsigned int count;
	unsigned int used;
} Labels;



void PushString(char* newString);
char *TypeToStringForInit(Terminal type);
char *ScopeToString(bool global);
const void* FindID(int *tokenPos);
void PushWLabel (void);
unsigned int TopWLabel(void);
void PopWLabel(void);
void PushIfLabel(void);
unsigned int TopIfLabel(void);
void PopIfLabel(void);



//Globalni staticka promenna pro jednodussi spravu pameti
static Buffer g_Rules;
static Buffer g_Code;
static TokenArray g_Tokens;
static Labels g_WLabels;
static Labels g_IfLabels;


bool isGlobal = true;





void GenerateCode(void) {
	int rule;
	int tokenPos = 0;
	char tmp[CODE_CHUNK];
	char tmp2[CODE_CHUNK];
	//TokenType type;
	const void* value;
	Variable *var;
	bool elseWasUsed = false;

	if (g_Rules.used > 0) {
		printf("BLOCK START - TOKENS...size: %d, used: %d, RULES...size: %d, used: %d\n",(int)g_Tokens.size, (int)g_Tokens.used, (int)g_Rules.size, (int)g_Rules.used);
		/*for (size_t i = 0; i < g_Tokens.used; i++) {
			printf("%s ", (char*)GetTokenValue(g_Tokens.array[i])); // pomocne vypisy
		}*/
		putchar('\n');
		for (size_t j = 0; j < g_Rules.used; j++) {
			memset(tmp, 0, CODE_CHUNK);
			rule = (int)g_Rules.buffer[j];
			//printf ("pravidla: %d\n", rule);

			switch (rule) {
				case RULE_EPSILON: // 1
					break;
				case RULE_MAIN_SCOPE: // 2
					isGlobal = false;
					break;
				case RULE_FUNC_DECL: // 3
					while (GetTokenType(g_Tokens.array[tokenPos]) != TOKEN_IDENTIFIER) tokenPos++; // deklaracie nas nezaujimaju
					tokenPos++;
					break;
				case RULE_FUNC_DEF: // 4
					while (GetTokenType(g_Tokens.array[tokenPos]) != TOKEN_IDENTIFIER) tokenPos++;
					value = GetTokenValue(g_Tokens.array[tokenPos]);
					sprintf(tmp, "LABEL GF@%s\n", (char*)value);
					tokenPos++;
					PushString(tmp);
					// TODO
					break;
				case RULE_ST_LIST: // 9
					break;
				case RULE_VAR_GLOBAL: // 27 // TODO lokalna prepisuje globalnu!
					value = FindID (&tokenPos);
					var = LookupGlobalVariable((const char*)value);
					sprintf (tmp, "DEFVAR GF@%s_%d\nMOVE GF@%s_%d %s\n", var->name, (int)var->codeLine, var->name, (int)var->codeLine, TypeToStringForInit(var->type));
					PushString(tmp);
					break;
				case RULE_ST_VAR_DECL: // 10
					value = FindID (&tokenPos);
					//printf ("TOKEN POS JE %d a id: %s\n", tokenPos, (char*) value);
					var = LookupVariable((const char*)value, false, true);

					sprintf (tmp, "DEFVAR %s@%s_%d\nMOVE %s@%s_%d %s\n", ScopeToString(var->globalVariable), var->name,(int)var->codeLine ,
									 ScopeToString(var->globalVariable), var->name, (int)var->codeLine,TypeToStringForInit(var->type));
					PushString(tmp);
					break;
				case RULE_ST_PRINT: // 14
					sprintf(tmp, "WRITE <symb>;\n"); // TODO.. token na printenie nepride?
					PushString(tmp);
					break;
				case RULE_ST_WHILE: // 15
					// TODO  ...jumpy podla vyrazov
					PushWLabel();
					sprintf(tmp, "LABEL LF@_wlabel_%d\n", TopWLabel());
					PushString(tmp);
					PushWLabel();
					sprintf(tmp, "JUMPIFEQ LF@_wlabel_%d false X\n", TopWLabel()); // TODO podla vyrazov
					PushString(tmp);
					break;
				case RULE_ST_IF: // 17
					// TODO jumpy podla toho ako to bude s vyrazmi
					PushIfLabel();
					sprintf(tmp, "JUMPIFEQ LF@_iflabel_%d false X\n", TopIfLabel()); // TODO s vyrazmi
					PushString(tmp);
					break;
				case RULE_ELSEIF: // 18
					sprintf(tmp, "LABEL LF@_iflabel_%d\n", TopIfLabel());
					PopIfLabel();
					PushString(tmp);
					PushIfLabel();
					sprintf(tmp, "JUMPIFEQ LF@_iflabel_%d false X\n", TopIfLabel());
					PushString(tmp);
					break;
				case RULE_ELSE: // 20
					elseWasUsed = true;
					sprintf(tmp, "LABEL LF@_iflabel_%d\n", TopIfLabel());
					PopIfLabel();
					PushString(tmp);
					break;
				case RULE_END_IF: // 21
					if (g_IfLabels.used > 0 && !elseWasUsed) {
						sprintf(tmp, "LABEL LF@_iflabel_%d\n", TopIfLabel());
						PopIfLabel();
						PushString(tmp);
					}
					elseWasUsed = false;
					break;
				case	RULE_VAR_INIT: // 22
					if (isGlobal) {
						sprintf (tmp, "MOVE GF@%s_%d X\n", var->name, (int)var->codeLine); // TODO podla vyrazov
					}
					else {
						sprintf(tmp, "MOVE %s@%s_%d X\n", ScopeToString(var->globalVariable), var->name, (int)var->codeLine); // TODO podla vyrazov
					}
					PushString(tmp);
					break;
				case RULE_OP_EQ: // 29
					value = FindID(&tokenPos);
					var = LookupVariable((const char*)value, false, true);
					sprintf (tmp, "MOVE %s@%s_%d X\n", ScopeToString(var->globalVariable), var->name, (int)var->codeLine);
					PushString(tmp);
					break;
				case RULE_OP_PLUS_EQ: // 30
					value = FindID(&tokenPos);
					var = LookupVariable((const char*)value, false, true);
					sprintf (tmp, "ADD %s@%s_%d %s@%s_%d X\n", ScopeToString(var->globalVariable), var->name, (int)var->codeLine,
									 ScopeToString(var->globalVariable), var->name, (int)var->codeLine); // TODO s vyrazmi
					PushString(tmp);
					break;
				case RULE_OP_MINUS_EQ: // 31
					value = FindID(&tokenPos);
					var = LookupVariable((const char*)value, false, true);
					sprintf (tmp, "SUB %s@%s_%d %s@%s_%d X\n", ScopeToString(var->globalVariable), var->name, (int)var->codeLine,
									 ScopeToString(var->globalVariable), var->name,(int)var->codeLine); // TODO s vyrazmi
					PushString(tmp);
					break;
				case RULE_OP_MULTIPLY_EQ: // 32
					value = FindID(&tokenPos);
					var = LookupVariable((const char*)value, false, true);
					sprintf (tmp, "MUL %s@%s_%d %s@%s_%d X\n", ScopeToString(var->globalVariable), var->name,(int) var->codeLine,
									 ScopeToString(var->globalVariable), var->name, (int)var->codeLine); // TODO s vyrazmi
					PushString(tmp);
					break;
				case RULE_OP_INT_DIV_EQ: // 33 TODO lisia sa? zas zistit podla vyrazov
				case RULE_OP_REAL_DIV_EQ: // 34
					value = FindID(&tokenPos);
					var = LookupVariable((const char*)value, false, true);
					sprintf (tmp, "DIV %s@%s_%d %s@%s_%d X\n", ScopeToString(var->globalVariable), var->name, (int)var->codeLine,
									 ScopeToString(var->globalVariable), var->name, (int)var->codeLine); // TODO s vyrazmi
					PushString(tmp);
					break;
				default:
					break;

			}

		}

		/*
		 * Kedze nemame pravidlo pre loop a potrebujem generovat navestia na konci while
		 * musel som to vyriesit takto.
		 */
		if (g_WLabels.used != 0) {

			while ((unsigned)tokenPos < g_Tokens.used) {
				if (GetTokenType(g_Tokens.array[tokenPos]) == TOKEN_KEYWORD) {
					if ((strcmp(GetTokenValue(g_Tokens.array[tokenPos]), "LOOP")) == 0) {
						printf("DO STUFF\n");
						memset(tmp, 0, CODE_CHUNK);
						memset(tmp2, 0, CODE_CHUNK);
						sprintf(tmp2, "LABEL LF@_wlabel_%d\n", TopWLabel());
						PopWLabel();
						sprintf(tmp, "JUMP LF@_wlabel_%d\n", TopWLabel());
						PopWLabel();
						PushString(tmp);
						PushString(tmp2);

					}
				}
				tokenPos++;
			}
		}
	}

	//printf ("BLOCK END\n");

	g_Tokens.used = 0;
	g_Rules.used = 0;




	/*
	TokenType type;
	const void* value;
	if (g_Tokens.used > 0) {
		printf("-- BLOCK START\n");
		for (size_t i = 0; i < g_Tokens.used; i++) {
			type = GetTokenType(g_Tokens.array[i]);
			value = GetTokenValue(g_Tokens.array[i]);
			switch (type) {
				case TOKEN_INTEGER:
					printf("%d", *(int*) value);
					break;
				case TOKEN_DOUBLE:
					printf("%g", *(double*) value);
					break;

				case TOKEN_STRING:
				case TOKEN_COMMA:
				case TOKEN_SEMICOLON:
				case TOKEN_L_BRACKET:
				case TOKEN_R_BRACKET:
				case TOKEN_EOL:
				case TOKEN_OPERATOR:
				case TOKEN_KEYWORD:
				case TOKEN_IDENTIFIER:
					printf("%s ", (char*) value);
					break;

				default:
					break;
			}
		}
		printf("-- BLOCK END\n\n");
		g_Tokens.used = 0;
	}
	 */
}


const void* FindID(int *tokenPos) {
	const void *tmp;
	while (GetTokenType(g_Tokens.array[(*tokenPos)]) != TOKEN_IDENTIFIER) (*tokenPos)++; // hladame identifikator
	tmp = GetTokenValue(g_Tokens.array[(*tokenPos)]);
	(*tokenPos)++; // preskocime identifikator
	return tmp;
}

char *ScopeToString(bool global) {
	if (global == true)
		return "GF";
	else
		return "LF";
}


char *TypeToStringForInit(Terminal type) {
	switch (type) {
		case T_DOUBLE:
			return "0.0";
		case T_INTEGER:
			return "0";
		case T_STRING:
			return "!\"\"";
		default:
			return "";
	}
}


void PushString(char *newString) {
	size_t length = strlen(newString);

	if (g_Code.size - g_Code.used <= length) {
		uchar* tmp;
		if ((tmp = realloc(g_Code.buffer, sizeof(uchar) * (g_Code.size + CODE_CHUNK))) == NULL) {
			FatalError(ER_FATAL_ALLOCATION);
		}
		g_Code.buffer = tmp;
		if (g_Code.size == 0) {
			sprintf((char*)g_Code.buffer,".IFJcode17\n");
		}
		g_Code.size += CODE_CHUNK;
	}


	strcat((char*)g_Code.buffer, newString);

	g_Code.used = g_Code.used + length;

}



void PushWLabel (void) {
	if (g_WLabels.used == g_WLabels.size) {
		//Zvetsime pole ukazatelu
		unsigned int* tmp;
		g_WLabels.size += RULE_CHUNK;
		if ((tmp = realloc(g_WLabels.labels, sizeof(unsigned int*) * g_WLabels.size)) == NULL) {
			FatalError(ER_FATAL_ALLOCATION);
		}
		g_WLabels.labels = tmp;
	}

	g_WLabels.labels[g_WLabels.used++] = g_WLabels.count;
	g_WLabels.count++;
}


unsigned int TopWLabel(void) {
	return g_WLabels.labels[g_WLabels.used-1];
}

void PopWLabel(void) {
	g_WLabels.used--;
}

void PushIfLabel (void) {
	if (g_IfLabels.used == g_IfLabels.size) {
		//Zvetsime pole ukazatelu
		unsigned int* tmp;
		g_IfLabels.size += RULE_CHUNK;
		if ((tmp = realloc(g_IfLabels.labels, sizeof(unsigned int*) * g_IfLabels.size)) == NULL) {
			FatalError(ER_FATAL_ALLOCATION);
		}
		g_IfLabels.labels = tmp;
	}

	g_IfLabels.labels[g_IfLabels.used++] = g_IfLabels.count;
	g_IfLabels.count++;
}


unsigned int TopIfLabel(void) {
	return g_IfLabels.labels[g_IfLabels.used-1];
}

void PopIfLabel(void) {
	g_IfLabels.used--;
}






void PushToken(Token* token) {
	if (g_Tokens.used == g_Tokens.size) {
		//Zvetsime pole ukazatelu
		Token** tmp;
		g_Tokens.size += RULE_CHUNK;
		if ((tmp = realloc(g_Tokens.array, sizeof(Token*) * g_Tokens.size)) == NULL) {
			FatalError(ER_FATAL_ALLOCATION);
		}
		g_Tokens.array = tmp;
	}

	g_Tokens.array[g_Tokens.used++] = token;
}

void PopToken(void) {
	if (g_Tokens.used > 0) {
		g_Tokens.used--;
	}
}


void OutputCode(void) {
	printf ("%s", g_Code.buffer);
}


void InsertRule(size_t ruleID) {
	if (g_Rules.used == g_Rules.size) {
		uchar* newBuffer;
		g_Rules.size += RULE_CHUNK;
		if ((newBuffer = realloc(g_Rules.buffer, sizeof(uchar) * g_Rules.size)) == NULL) {
			FatalError(ER_FATAL_ALLOCATION);
		}
		g_Rules.buffer = newBuffer;
	}
	g_Rules.buffer[g_Rules.used++] = (uchar) ruleID;
}


void GeneratorCleanup(void) {
	if (g_Rules.buffer) {
		free(g_Rules.buffer);
		g_Rules.buffer = NULL;
		g_Rules.used = g_Rules.size = 0;
	}
	if (g_Code.buffer) {
		free(g_Code.buffer);
		g_Code.buffer = NULL;
		g_Code.used = g_Code.size = 0;
	}
	if (g_Tokens.array) {
		free(g_Tokens.array);
		g_Tokens.array = NULL;
		g_Tokens.used = g_Tokens.size = 0;
	}
	if (g_WLabels.labels) {
		free(g_WLabels.labels);
		g_WLabels.labels = NULL;
		g_WLabels.used = g_Tokens.size = 0;
	}
	if (g_IfLabels.labels) {
		free(g_IfLabels.labels);
		g_IfLabels.labels = NULL;
		g_IfLabels.used = g_Tokens.size = 0;
	}
}
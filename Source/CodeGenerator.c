#include <stdio.h>
#include <stdlib.h>
#include "CodeGenerator.h"
#include "CompilationErrors.h"
#include "LLtable.h"
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

//Globalni staticka promenna pro jednodussi spravu pameti
static Buffer g_Rules;
static Buffer g_Code;
static TokenArray g_Tokens;


void GenerateCode(void) {
	int rule;
	int tokenPos = 0;
	char tmp[CODE_CHUNK];
	TokenType type;
	const void* value;

	if (g_Rules.used > 0) {
		printf("BLOCK START\n");

		for (size_t j = 0; j < g_Rules.used; j++) {
			memset(tmp, 0, CODE_CHUNK);
			rule = (int)g_Rules.buffer[j];
			printf ("pravidla: %d\n", rule);

			switch (rule) {
				case RULE_EPSILON: // 1
				case RULE_MAIN_SCOPE: // 2
				case RULE_ST_LIST: // 9
					break;
				case RULE_ST_VAR_DECL: // 10
					while (GetTokenType(g_Tokens.array[tokenPos]) != TOKEN_IDENTIFIER) tokenPos++; // hladame identifikator
					value = GetTokenValue(g_Tokens.array[tokenPos]);
					tokenPos++; // preskocime identifikator
					switch((int)g_Rules.buffer[j+1]) {
						case RULE_TYPE_INT:
							sprintf(tmp, "DEFVAR LF@%s\nMOVE LF@%s 0\n", (char*)value, (char*)value); // TODO LF GF - inicializacia na 0
							break;
						case RULE_TYPE_DOUBLE:
							sprintf(tmp, "DEFVAR LF@%s\nMOVE LF@%s 0.0\n", (char*)value, (char*)value); // TODO LF GF - inicializacia na 0.0
							break;
						case RULE_TYPE_STRING:
							sprintf(tmp, "DEFVAR LF@%s\nMOVE LF@%s !""\n", (char*)value, (char*)value); // TODO LF GF - inicializacia na prazdny string
							break;
					}

					PushString(tmp);
					printf ("%s strlen je: %d\n", g_Code.buffer, (int)strlen((char*)g_Code.buffer));
					printf ("%d je size, %d je used\n", (int)g_Code.size, (int)g_Code.used);

				default:
					break;

			}







		}
	}







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


void PushString(char *newString) {
	size_t length = strlen(newString);

	if (g_Code.size - g_Code.used <= length) {
		uchar* tmp;
		if ((tmp = realloc(g_Code.buffer, sizeof(uchar) * (g_Code.size + CODE_CHUNK))) == NULL) {
			FatalError(ER_FATAL_INTERNAL);
		}
		g_Code.buffer = tmp;
		if (g_Code.size == 0) { // prvy krat je potreba memsetnut, inak su tam nejake smeti
			memset(g_Code.buffer, 0, CODE_CHUNK);
		}
		g_Code.size += CODE_CHUNK;
	}


	strcat((char*)g_Code.buffer, newString);

	g_Code.used = g_Code.used + length;

}



void PushToken(Token* token) {
	if (g_Tokens.used == g_Tokens.size) {
		//Zvetsime pole ukazatelu
		Token** tmp;
		g_Tokens.size += RULE_CHUNK;
		if ((tmp = realloc(g_Tokens.array, sizeof(Token*) * g_Tokens.size)) == NULL) {
			FatalError(ER_FATAL_INTERNAL);
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

}


void InsertRule(size_t ruleID) {
	if (g_Rules.used == g_Rules.size) {
		uchar* newBuffer;
		g_Rules.size += RULE_CHUNK;
		if ((newBuffer = realloc(g_Rules.buffer, sizeof(uchar) * g_Rules.size)) == NULL) {
			FatalError(ER_FATAL_INTERNAL);
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
}
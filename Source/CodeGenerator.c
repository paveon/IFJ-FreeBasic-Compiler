#include <stdio.h>
#include <stdlib.h>
#include "CodeGenerator.h"
#include "CompilationErrors.h"

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
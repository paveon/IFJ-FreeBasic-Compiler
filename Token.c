#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "CompilationErrors.h"
#include "Token.h"


typedef enum TokenTypes {
	TOKEN_UNDEFINED,
	TOKEN_OPERATOR,
	TOKEN_KEYWORD,
	TOKEN_IDENTIFIER,
	TOKEN_INTEGER,
	TOKEN_DOUBLE,
	TOKEN_STRING,
	TOKEN_EOL,
	TOKEN_EOF
} TokenType;

struct Token {
	void* value;
	TokenType type;
};

static char* const KeyWords[] = {
		  "AS", "ASC", "DECLARE", "DIM", "DO", "DOUBLE", "ELSE", "END", "CHR",
		  "FUNCTION", "IF", "INPUT", "INTEGER", "LENGTH", "LOOP", "PRINT", "RETURN",
		  "SCOPE", "STRING", "SUBSTR", "THEN", "WHILE",
		  "AND", "BOOLEAN", "CONTINUE", "ELSEIF", "EXIT", "FALSE", "FOR", "NEXT",
		  "NOT", "OR", "SHARED", "STATIC", "TRUE"
};

static char* const Operators[] = {
		  "*", "/", "\\", "+", "-", "=", "<>", "<", "<=", ">", ">="
};


void StrToUpper(char* str) {
	while (*str) {
		if (isalpha(str[0])) {
			str[0] = (char) toupper(str[0]);
		}
		str++;
	}
}

Token* CreateToken(void) {
	Token* newToken;

	if ((newToken = malloc(sizeof(Token))) != NULL) {
		newToken->type = TOKEN_UNDEFINED;
		newToken->value = NULL;

		return newToken;

		//TODO: vytvorit strukturu pro uchovavani vsech tokenu
	}

	//Nema smysl zotavovat se z alokacni chyby
	exit(ER_EXIT_INTERNAL);
}

void TokenSetOperator(Token* token, const char* operator) {
	size_t length = strlen(operator);

	//Nemuze se jednat o operator
	if (!token || !operator || length == 0 || length > 2) { return; }

	for (size_t i = 0; i < (sizeof(Operators) / sizeof(char*)); i++) {
		if (strcmp(Operators[i], operator) == 0) {
			//Operator byl nalezen
			token->type = TOKEN_OPERATOR;
			token->value = Operators[i];
			return;
		}
	}

	token->type = TOKEN_UNDEFINED;
	token->value = NULL;
}

void TokenSetKeyword(Token* token, char* keyword) {
	StrToUpper(keyword);
	size_t length = strlen(keyword);

	//Nemuze se jednat o klicove slovo
	if (!token || !keyword || length == 0 || length > 8) { return; }

	for (size_t i = 0; i < (sizeof(KeyWords) / sizeof(char*)); i++) {
		if (strcmp(KeyWords[i], keyword) == 0) {
			//Klicove slovo bylo nalezeno
			token->type = TOKEN_KEYWORD;
			token->value = KeyWords[i];
			return;
		}
	}

	token->type = TOKEN_UNDEFINED;
	token->value = NULL;
}

void TokenSetIdentifier(Token* token, const char* identifier) {
	if(!token || !identifier){return;}

	size_t length = strlen(identifier);
	if(length > 0){

		//Predpoklada se, ze pri nastavenem TOKEN_IDENTIFIER je bezpecne smazat predchozi obsah
		if(token->type == TOKEN_IDENTIFIER && token->value != NULL){
			free(token->value);
		}

		if((token->value = malloc(sizeof(char) * (length + 1))) == NULL){
			exit(ER_EXIT_INTERNAL); //Bez zotaveni
		}

		memcpy(token->value, identifier, length);
		((char*)token->value)[length] = '\0';
	}
}
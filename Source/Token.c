#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "CompilationErrors.h"
#include "Token.h"

#define KEYWORD_MAX_LEN 8
#define OPERATOR_MAX_LEN 2
#define MALLOC_CHUNK 20

struct Token {
	void* value;
	TokenType type;
};

typedef struct ReadOnlyData {
	int* integers;
	double* doubles;
	char** strings;
	char** symbols;
	size_t intArraySize;
	size_t doubleArraySize;
	size_t stringArraySize;
	size_t symbolArraySize;
} ReadOnlyData;

static ReadOnlyData Data;
static char* const Keywords[] = {
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


void StrToLower(char* str) {
	while (*str) {
		if (isalpha(str[0])) {
			str[0] = (char) tolower(str[0]);
		}
		str++;
	}
}


TokenType GetType(const Token* token) {
	return token->type;
}


const void* GetValue(const Token* token) {
	return token->value;
}


int GetInt(const Token* token) {
	if (token->type == TOKEN_INTEGER) { return *(int*) token->value; }
	return -1;
}


double GetDouble(const Token* token) {
	if (token->type == TOKEN_DOUBLE) { return *(double*) token->value; }
	return NAN;
}


Token* CreateToken(void) {
	Token* newToken;

	if ((newToken = malloc(sizeof(Token))) == NULL) {
		//TODO: error output + cleanup
		exit(ER_EXIT_INTERNAL);
	}

	//TODO: vytvorit strukturu pro uchovavani vsech tokenu
	newToken->type = TOKEN_UNDEFINED;
	newToken->value = NULL;

	return newToken;
}


void SetOperator(Token* token, const char* operator) {
	size_t opLength = strlen(operator);

	if (!token || token->type != TOKEN_UNDEFINED || !operator || opLength == 0) { return; }

	//Nemuze se jednat o operator
	if (opLength > OPERATOR_MAX_LEN) { return; }

	size_t arraySize = (sizeof(Operators) / sizeof(char*));
	for (size_t i = 0; i < arraySize; i++) {
		if (strcmp(Operators[i], operator) == 0) {
			//Operator byl nalezen
			token->type = TOKEN_OPERATOR;
			token->value = Operators[i];
			return;
		}
	}
}


void SetIdentifier(Token* token, char* symbol) {
	static size_t arrayIndex = 0;
	size_t symbolLength = strlen(symbol);

	if (!token || token->type != TOKEN_UNDEFINED || !symbol || symbolLength == 0) { return; }

	if (symbolLength <= KEYWORD_MAX_LEN) {
		//Mohlo by se jednat o klicove slovo
		StrToUpper(symbol);
		size_t arraySize = (sizeof(Keywords) / sizeof(char*));
		for (size_t i = 0; i < arraySize; i++) {
			if (strcmp(Keywords[i], symbol) == 0) {
				token->type = TOKEN_KEYWORD;
				token->value = Keywords[i];
				return;
			}
		}
	}

	StrToLower(symbol);
	token->type = TOKEN_IDENTIFIER;

	//Pokus o recyklaci identifikatoru
	for (size_t i = 0; i < arrayIndex; i++) {
		if (strcmp(Data.symbols[i], symbol) == 0) {
			token->value = Data.symbols[i];
			return;
		}
	}

	if (arrayIndex == Data.symbolArraySize) {
		char** tmp = NULL;
		Data.symbolArraySize += MALLOC_CHUNK;
		if ((tmp = realloc(Data.symbols, sizeof(char**) * Data.symbolArraySize)) == NULL) {
			//TODO: error output + cleanup
			exit(ER_EXIT_INTERNAL);
		}
		Data.symbols = tmp;
	}

	char* newSymbol = NULL;
	if ((newSymbol = malloc(sizeof(char) * (symbolLength + 1))) == NULL) {
		//TODO: error output + cleanup
		exit(ER_EXIT_INTERNAL); //Bez zotaveni
	}
	memcpy(newSymbol, symbol, symbolLength);
	newSymbol[symbolLength] = '\0';
	Data.symbols[arrayIndex++] = newSymbol;
	token->value = newSymbol;
}


void SetInteger(Token* token, const char* number) {
	static size_t arrayIndex = 0; //K sledovani aktualniho insert indexu do pole
	size_t numberLength = strlen(number);
	int value;

	if (!token || token->type != TOKEN_UNDEFINED || !number || numberLength == 0) { return; }

	//Predpoklada se validni hodnota predana z lexikalniho analyzatoru
	value = (int) strtol(number, NULL, 10);
	token->type = TOKEN_INTEGER;

	for (size_t i = 0; i < arrayIndex; i++) {
		if (value == Data.integers[i]) {
			token->value = &Data.integers[i];
			return;
		}
	}

	if (arrayIndex == Data.intArraySize) {
		int* tmp = NULL;
		Data.intArraySize += MALLOC_CHUNK;
		if ((tmp = realloc(Data.integers, sizeof(int) * Data.intArraySize)) == NULL) {
			//TODO: error output + cleanup
			exit(ER_EXIT_INTERNAL);
		}
		Data.integers = tmp;
	}
	Data.integers[arrayIndex] = value;
	token->value = &Data.integers[arrayIndex++];
}


void SetDouble(Token* token, const char* number) {
	static size_t arrayIndex = 0; //K sledovani aktualniho insert indexu do pole
	size_t numberLength = strlen(number);

	if (!token || token->type != TOKEN_UNDEFINED || !number || numberLength == 0) { return; }

	if (arrayIndex == Data.doubleArraySize) {
		double* tmp = NULL;
		Data.doubleArraySize += MALLOC_CHUNK;
		if ((tmp = realloc(Data.doubles, sizeof(double) * Data.doubleArraySize)) == NULL) {
			//TODO: error output + cleanup
			exit(ER_EXIT_INTERNAL);
		}
		Data.doubles = tmp;
	}

	//Predpoklada se validni hodnota predana z lexikalniho analyzatoru
	Data.doubles[arrayIndex] = strtod(number, NULL);
	token->type = TOKEN_DOUBLE;
	token->value = &Data.doubles[arrayIndex++];
}


void SetString(Token* token, const char* string) {
	static size_t arrayIndex = 0; //K sledovani aktualniho insert indexu do pole
	size_t stringLength = strlen(string);

	if (!token || token->type != TOKEN_UNDEFINED || !string) { return; }

	token->type = TOKEN_STRING;

	for (size_t i = 0; i < arrayIndex; i++) {
		if (strcmp(Data.strings[i], string) == 0) {
			token->value = Data.strings[i];
			return;
		}
	}

	if (arrayIndex == Data.stringArraySize) {
		char** tmp = NULL;
		Data.stringArraySize += MALLOC_CHUNK;
		if ((tmp = realloc(Data.strings, sizeof(char**) * Data.stringArraySize)) == NULL) {
			//TODO: error output + cleanup
			exit(ER_EXIT_INTERNAL);
		}
		Data.strings = tmp;
	}

	char* newString = NULL;
	if ((newString = malloc(sizeof(char) * (stringLength + 1))) == NULL) {
		//TODO: error output + cleanup
		exit(ER_EXIT_INTERNAL);
	}

	//Ukladam string literaly do pole, abychom je mohli pripadne jednoduse recyklovat
	//a zaroven jednoduse hlidat memory leaky
	memcpy(newString, string, stringLength);
	newString[stringLength] = '\0';
	Data.strings[arrayIndex++] = newString;
	token->value = newString;
}


void SetEOL(Token* token) {
	if (!token || token->type != TOKEN_UNDEFINED) { return; }

	token->type = TOKEN_EOL;
}


void SetEOF(Token* token) {
	if (!token || token->type != TOKEN_UNDEFINED) { return; }

	token->type = TOKEN_EOF;
}

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "CompilationErrors.h"
#include "Token.h"

#define KEYWORD_MAX_LEN 8
#define OPERATOR_MAX_LEN 2
#define MALLOC_CHUNK 20
#define STASH_CHUNK 200

struct Token {
	void* value;
	TokenType type;
};

typedef struct ReadOnlyData {
	int* integers;
	double* doubles;
	char** strings;
	char** symbols;
	size_t intsSize, intsUsed;
	size_t doublesSize, doublesUsed;
	size_t stringsSize, stringsUsed;
	size_t symbolsSize, symbolsUsed;
} ReadOnlyData;


typedef struct TokenStash{
	Token* tokenArray;
	size_t arraySize;
	size_t arrayUsed;
	size_t activeToken;
} TokenStash;

static TokenStash Stash;
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


Token* GetNextToken(void){
	if(Stash.activeToken < Stash.arrayUsed){
		return &Stash.tokenArray[Stash.activeToken++];
	}
	return NULL;
}


Token* CreateToken(void) {
	Token* newToken;

	if(Stash.arrayUsed == Stash.arraySize){
		Token* tmp = NULL;
		Stash.arraySize += STASH_CHUNK;
		if((tmp = realloc(Stash.tokenArray, sizeof(Token) * Stash.arraySize)) == NULL){
			FatalError(ER_FATAL_INTERNAL);
		}
		Stash.tokenArray = tmp;
	}

	newToken = &Stash.tokenArray[Stash.arrayUsed++];
	newToken->type = TOKEN_UNDEFINED;
	newToken->value = NULL;

	return newToken;
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
	for (size_t i = 0; i < Data.symbolsUsed; i++) {
		if (strcmp(Data.symbols[i], symbol) == 0) {
			token->value = Data.symbols[i];
			return;
		}
	}

	if (Data.symbolsUsed == Data.symbolsSize) {
		char** tmp = NULL;
		Data.symbolsSize += MALLOC_CHUNK;
		if ((tmp = realloc(Data.symbols, sizeof(char**) * Data.symbolsSize)) == NULL) {
			FatalError(ER_FATAL_INTERNAL);
		}
		Data.symbols = tmp;
	}

	char* newSymbol = NULL;
	if ((newSymbol = malloc(sizeof(char) * (symbolLength + 1))) == NULL) {
		FatalError(ER_FATAL_INTERNAL);
	}
	memcpy(newSymbol, symbol, symbolLength);
	newSymbol[symbolLength] = '\0';
	Data.symbols[Data.symbolsUsed++] = newSymbol;
	token->value = newSymbol;
}


void SetInteger(Token* token, const char* number) {
	size_t numberLength = strlen(number);
	int value;

	if (!token || token->type != TOKEN_UNDEFINED || !number || numberLength == 0) { return; }

	//Predpoklada se validni hodnota predana z lexikalniho analyzatoru
	value = (int) strtol(number, NULL, 10);
	token->type = TOKEN_INTEGER;

	for (size_t i = 0; i < Data.intsUsed; i++) {
		if (value == Data.integers[i]) {
			token->value = &Data.integers[i];
			return;
		}
	}

	if (Data.intsUsed == Data.intsSize) {
		int* tmp = NULL;
		Data.intsSize += MALLOC_CHUNK;
		if ((tmp = realloc(Data.integers, sizeof(int) * Data.intsSize)) == NULL) {
			FatalError(ER_FATAL_INTERNAL);
		}
		Data.integers = tmp;
	}
	Data.integers[Data.intsUsed] = value;
	token->value = &Data.integers[Data.intsUsed++];
}


void SetDouble(Token* token, const char* number) {
	size_t numberLength = strlen(number);

	if (!token || token->type != TOKEN_UNDEFINED || !number || numberLength == 0) { return; }

	if (Data.doublesUsed == Data.doublesSize) {
		double* tmp = NULL;
		Data.doublesSize += MALLOC_CHUNK;
		if ((tmp = realloc(Data.doubles, sizeof(double) * Data.doublesSize)) == NULL) {
			FatalError(ER_FATAL_INTERNAL);
		}
		Data.doubles = tmp;
	}

	//Predpoklada se validni hodnota predana z lexikalniho analyzatoru
	Data.doubles[Data.doublesUsed] = strtod(number, NULL);
	token->type = TOKEN_DOUBLE;
	token->value = &Data.doubles[Data.doublesUsed++];
}


void SetString(Token* token, const char* string) {
	size_t stringLength = strlen(string);

	if (!token || token->type != TOKEN_UNDEFINED || !string) { return; }

	token->type = TOKEN_STRING;

	for (size_t i = 0; i < Data.stringsUsed; i++) {
		if (strcmp(Data.strings[i], string) == 0) {
			token->value = Data.strings[i];
			return;
		}
	}

	if (Data.stringsUsed == Data.stringsSize) {
		char** tmp = NULL;
		Data.stringsSize += MALLOC_CHUNK;
		if ((tmp = realloc(Data.strings, sizeof(char**) * Data.stringsSize)) == NULL) {
			FatalError(ER_FATAL_INTERNAL);
		}
		Data.strings = tmp;
	}

	char* newString = NULL;
	if ((newString = malloc(sizeof(char) * (stringLength + 1))) == NULL) {
		FatalError(ER_FATAL_INTERNAL);
	}

	//Ukladam string literaly do pole, abychom je mohli pripadne jednoduse recyklovat
	//a zaroven jednoduse hlidat memory leaky
	memcpy(newString, string, stringLength);
	newString[stringLength] = '\0';
	Data.strings[Data.stringsUsed++] = newString;
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


void TokenCleanup(void) {
	for(size_t i = 0; i < Data.stringsUsed; i++){
		free(Data.strings[i]);
	}
	for(size_t i = 0; i < Data.symbolsUsed; i++){
		free(Data.symbols[i]);
	}
	free(Data.integers);
	free(Data.doubles);
	free(Data.strings);
	free(Data.symbols);
	free(Stash.tokenArray);
}

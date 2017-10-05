#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include "CompilationErrors.h"
#include "Token.h"

#define KEYWORD_MAX_LEN 8
#define OPERATOR_MAX_LEN 2
#define MALLOC_SMALL_CHUNK 20
#define STASH_CHUNK 200

struct Token {
	void* value;
	TokenType type;
	bool trailSpace;
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


typedef struct TokenStash {
	Token* tokenArray;
	size_t arraySize;
	size_t arrayUsed;
	size_t activeToken;
} TokenStash;

static TokenStash Stash;
static ReadOnlyData Data;
static Token* Current;

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


static char* Miscellaneous[] = {
		  ",", ";", "(", ")"
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


Token* GetNextToken(void) {
	if (Stash.activeToken < Stash.arrayUsed) {
		return &Stash.tokenArray[Stash.activeToken++];
	}
	return NULL;
}


void ReturnToken(void) {
	if (Stash.activeToken > 0) {
		Stash.activeToken--;
	}
}


void CreateToken(void) {
	if (Stash.arrayUsed == Stash.arraySize) {
		Token* tmp = NULL;
		Stash.arraySize += STASH_CHUNK;
		if ((tmp = realloc(Stash.tokenArray, sizeof(Token) * Stash.arraySize)) == NULL) {
			FatalError(ER_FATAL_INTERNAL);
		}
		Stash.tokenArray = tmp;
	}

	Current = &Stash.tokenArray[Stash.arrayUsed++];
	Current->type = TOKEN_UNDEFINED;
	Current->value = NULL;
	Current->trailSpace = false;
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


void SetOperator(const char* operator) {
	size_t length = strlen(operator);
	if (!Current || !operator || length == 0) { return; }

	//Nemuze se jednat o operator
	if (length > OPERATOR_MAX_LEN) { return; }

	size_t arraySize = (sizeof(Operators) / sizeof(char*));
	for (size_t i = 0; i < arraySize; i++) {
		if (strcmp(Operators[i], operator) == 0) {
			//Operator byl nalezen
			Current->type = TOKEN_OPERATOR;
			Current->value = Operators[i];
			Current = NULL;
			return;
		}
	}
}


void SetComma(void) {
	if (!Current) { return; }

	Current->type = TOKEN_COMMA;
	Current->value = Miscellaneous[TOKEN_COMMA];
	Current = NULL;
}


void SetSemicolon(void) {
	if (!Current) { return; }

	Current->type = TOKEN_SEMICOLON;
	Current->value = Miscellaneous[TOKEN_SEMICOLON];
	Current = NULL;
}


void SetLeftBracket(void) {
	if (!Current) { return; }

	Current->type = TOKEN_L_BRACKET;
	Current->value = Miscellaneous[TOKEN_L_BRACKET];
	Current = NULL;
}

void SetRightBracket(void) {
	if (!Current) { return; }

	Current->type = TOKEN_R_BRACKET;
	Current->value = Miscellaneous[TOKEN_R_BRACKET];
	Current = NULL;
}


void SetIdentifier(char* symbol) {
	size_t length = strlen(symbol);

	if (!Current || !symbol || length == 0) { return; }

	//Token je ukoncen pomoci libovolneho whitespace charakteru -> nemuze byt volani funkce
	if (isspace(symbol[length - 1])) {
		length--;
		symbol[length] = '\0';
		Current->trailSpace = true;
	}

	//Mohlo by se jednat o klicove slovo
	if (length <= KEYWORD_MAX_LEN) {
		StrToUpper(symbol);
		size_t arraySize = (sizeof(Keywords) / sizeof(char*));
		for (size_t i = 0; i < arraySize; i++) {
			if (strcmp(Keywords[i], symbol) == 0) {
				Current->type = TOKEN_KEYWORD;
				Current->value = Keywords[i];
				Current = NULL;
				return;
			}
		}
	}

	StrToLower(symbol);
	Current->type = TOKEN_IDENTIFIER;

	//Pokus o recyklaci identifikatoru
	for (size_t i = 0; i < Data.symbolsUsed; i++) {
		if (strcmp(Data.symbols[i], symbol) == 0) {
			Current->value = Data.symbols[i];
			Current = NULL;
			return;
		}
	}

	if (Data.symbolsUsed == Data.symbolsSize) {
		char** tmp = NULL;
		Data.symbolsSize += MALLOC_SMALL_CHUNK;
		if ((tmp = realloc(Data.symbols, sizeof(char**) * Data.symbolsSize)) == NULL) {
			FatalError(ER_FATAL_INTERNAL);
		}
		Data.symbols = tmp;
	}

	char* newSymbol = NULL;
	if ((newSymbol = malloc(sizeof(char) * (length + 1))) == NULL) {
		FatalError(ER_FATAL_INTERNAL);
	}

	memcpy(newSymbol, symbol, length);
	newSymbol[length] = '\0';
	Data.symbols[Data.symbolsUsed++] = newSymbol;
	Current->value = newSymbol;
	Current = NULL;
}


void SetInteger(const char* number) {
	size_t length = strlen(number);
	int value;

	if (!Current || !number || length == 0) { return; }

	//Predpoklada se validni hodnota predana z lexikalniho analyzatoru
	//TODO: vymyslet nejake reseni pro prilis velka cisla
	value = (int) strtol(number, NULL, 10);
	Current->type = TOKEN_INTEGER;

	for (size_t i = 0; i < Data.intsUsed; i++) {
		if (value == Data.integers[i]) {
			Current->value = &Data.integers[i];
			Current = NULL;
			return;
		}
	}

	if (Data.intsUsed == Data.intsSize) {
		int* tmp = NULL;
		Data.intsSize += MALLOC_SMALL_CHUNK;
		if ((tmp = realloc(Data.integers, sizeof(int) * Data.intsSize)) == NULL) {
			FatalError(ER_FATAL_INTERNAL);
		}
		Data.integers = tmp;
	}
	Data.integers[Data.intsUsed] = value;
	Current->value = &Data.integers[Data.intsUsed++];
	Current = NULL;
}


void SetDouble(const char* number) {
	size_t length = strlen(number);

	if (!Current || !number || length == 0) { return; }

	if (Data.doublesUsed == Data.doublesSize) {
		double* tmp = NULL;
		Data.doublesSize += MALLOC_SMALL_CHUNK;
		if ((tmp = realloc(Data.doubles, sizeof(double) * Data.doublesSize)) == NULL) {
			FatalError(ER_FATAL_INTERNAL);
		}
		Data.doubles = tmp;
	}

	//Predpoklada se validni hodnota predana z lexikalniho analyzatoru
	Data.doubles[Data.doublesUsed] = strtod(number, NULL);
	Current->type = TOKEN_DOUBLE;
	Current->value = &Data.doubles[Data.doublesUsed++];
	Current = NULL;
}


void SetString(const char* string) {
	if (!Current || !string) { return; }
	Token* token = Current;
	Current = NULL;

	token->type = TOKEN_STRING;

	for (size_t i = 0; i < Data.stringsUsed; i++) {
		if (strcmp(Data.strings[i], string) == 0) {
			token->value = Data.strings[i];
			return;
		}
	}

	if (Data.stringsUsed == Data.stringsSize) {
		char** tmp = NULL;
		Data.stringsSize += MALLOC_SMALL_CHUNK;
		if ((tmp = realloc(Data.strings, sizeof(char**) * Data.stringsSize)) == NULL) {
			FatalError(ER_FATAL_INTERNAL);
		}
		Data.strings = tmp;
	}
	size_t stringLength = strlen(string);
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


void SetEOL(void) {
	if (!Current) { return; }
	Current->type = TOKEN_EOL;
	Current = NULL;
}


void SetEOF(void) {
	if (!Current) { return; }
	Current->type = TOKEN_EOF;
	Current = NULL;
}


void TokenCleanup(void) {
	for (size_t i = 0; i < Data.stringsUsed; i++) {
		free(Data.strings[i]);
	}
	for (size_t i = 0; i < Data.symbolsUsed; i++) {
		free(Data.symbols[i]);
	}
	free(Data.integers);
	free(Data.doubles);
	free(Data.strings);
	free(Data.symbols);
	free(Stash.tokenArray);
}

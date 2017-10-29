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
#define BUFFER_CHUNK 1024

struct Token {
	void* value;
	Terminal terminal;
	TokenType type;
	bool trailSpace;
};

typedef struct ReadOnlyData {
	int* integers;
	char* textBuffer;
	double* doubles;
	size_t intsSize, intsUsed;
	size_t doublesSize, doublesUsed;
	size_t bufferSize, bufferIndex;
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

typedef struct Pair {
	char* value;
	Terminal terminal;
} Pair;


static const Pair Keywords[] = {
				{"AS", T_AS},
				{"ASC", T_UNDEFINED},
				{"DECLARE", T_DECLARE},
				{"DIM", T_DIM},
				{"DO", T_DO},
				{"DOUBLE", T_DOUBLE},
				{"ELSE", T_ELSE},
				{"END", T_END},
				{"CHR", T_UNDEFINED},
				{"FUNCTION", T_FUNCTION},
				{"IF", T_IF},
				{"INPUT", T_INPUT},
				{"INTEGER", T_INTEGER},
				{"LENGTH", T_UNDEFINED},
				{"LOOP", T_LOOP},
				{"PRINT", T_PRINT},
				{"RETURN", T_RETURN},
				{"SCOPE", T_SCOPE},
				{"STRING", T_STRING},
				{"SUBSTR", T_UNDEFINED},
				{"THEN", T_THEN},
				{"WHILE", T_WHILE},
				{"AND", T_UNDEFINED},
				{"BOOLEAN", T_UNDEFINED},
				{"CONTINUE", T_UNDEFINED},
				{"ELSEIF", T_ELSEIF},
				{"EXIT", T_UNDEFINED},
				{"FALSE", T_UNDEFINED},
				{"FOR", T_UNDEFINED},
				{"NEXT", T_UNDEFINED},
				{"NOT", T_UNDEFINED},
				{"OR", T_UNDEFINED},
				{"SHARED", T_SHARED},
				{"STATIC", T_STATIC},
				{"TRUE", T_UNDEFINED},
};

static const Pair Operators[] = {
				{"*", T_OPERATOR_MULTIPLY},
				{"/", T_OPERATOR_REAL_DIVIDE},
				{"\\", T_OPERATOR_INT_DIVIDE},
				{"+", T_OPERATOR_PLUS},
				{"-", T_OPERATOR_MINUS},
				{"=", T_OPERATOR_EQUAL},
				{"<>", T_OPERATOR_NOT_EQ},
				{"<", T_OPERATOR_LESS},
				{"<=", T_OPERATOR_LESS_EQ},
				{">", T_OPERATOR_GRT},
				{">=", T_OPERATOR_GRT_EQ}
};


static char* Miscellaneous[5] = {
				[TOKEN_COMMA] = ",",
				[TOKEN_SEMICOLON] = ";",
				[TOKEN_L_BRACKET] = "(",
				[TOKEN_R_BRACKET] = ")",
				[TOKEN_EOL] = "\n"
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


void ResizeBuffer() {
	char* newBuffer;
	Data.bufferSize += BUFFER_CHUNK;
	if ((newBuffer = realloc(Data.textBuffer, sizeof(char) * Data.bufferSize)) == NULL) {
		FatalError(ER_FATAL_INTERNAL);
	}
	Data.textBuffer = newBuffer;
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


TokenType GetTokenType(const Token* token) {
	return token->type;
}


const void* GetTokenValue(const Token* token) {
	return token->value;
}


Terminal GetTokenTerminal(const Token* token) {
	return token->terminal;
}


int GetTokenInt(const Token* token) {
	if (token->type == TOKEN_INTEGER) { return *(int*) token->value; }
	return -1;
}


double GetTokenDouble(const Token* token) {
	if (token->type == TOKEN_DOUBLE) { return *(double*) token->value; }
	return NAN;
}

bool GetTrailSpace(const Token* token) {
	return token->trailSpace;
}


void SetOperator(const char* operator) {
	size_t length = strlen(operator);
	if (!Current || !operator || length == 0) { return; }

	//Nemuze se jednat o operator
	if (length > OPERATOR_MAX_LEN) { return; }

	size_t arraySize = (sizeof(Operators) / sizeof(char*));
	for (size_t i = 0; i < arraySize; i++) {
		if (strcmp(Operators[i].value, operator) == 0) {
			//Operator byl nalezen
			Current->type = TOKEN_OPERATOR;
			Current->value = Operators[i].value;
			Current->terminal = Operators[i].terminal;
			Current = NULL;
			return;
		}
	}
}


void SetComma(void) {
	if (!Current) { return; }

	Current->type = TOKEN_COMMA;
	Current->value = Miscellaneous[TOKEN_COMMA];
	Current->terminal = T_COMMA;
	Current = NULL;
}


void SetSemicolon(void) {
	if (!Current) { return; }

	Current->type = TOKEN_SEMICOLON;
	Current->value = Miscellaneous[TOKEN_SEMICOLON];
	Current->terminal = T_SEMICOLON;
	Current = NULL;
}


void SetLeftBracket(void) {
	if (!Current) { return; }

	Current->type = TOKEN_L_BRACKET;
	Current->value = Miscellaneous[TOKEN_L_BRACKET];
	Current->terminal = T_LEFT_BRACKET;
	Current = NULL;
}

void SetRightBracket(void) {
	if (!Current) { return; }

	Current->type = TOKEN_R_BRACKET;
	Current->value = Miscellaneous[TOKEN_R_BRACKET];
	Current->terminal = T_RIGHT_BRACKET;
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
		size_t arraySize = (sizeof(Keywords) / sizeof(Pair));
		for (size_t i = 0; i < arraySize; i++) {
			if (strcmp(Keywords[i].value, symbol) == 0) {
				Current->type = TOKEN_KEYWORD;
				Current->value = Keywords[i].value;
				Current->terminal = Keywords[i].terminal;
				Current = NULL;
				return;
			}
		}
	}

	StrToLower(symbol);
	Current->type = TOKEN_IDENTIFIER;

	//Pokud by se novy retezec nevlezl do bufferu, rozsirime buffer
	while ((Data.bufferIndex + length + 1) > Data.bufferSize) {
		ResizeBuffer();
	}

	char* tmp = &Data.textBuffer[Data.bufferIndex];
	memcpy(tmp, symbol, length);
	tmp[length++] = 0;
	Data.bufferIndex += length;
	Current->value = tmp;
	Current->terminal = T_ID;
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
	Current->terminal = T_INTEGER;

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
	Current->terminal = T_DOUBLE;
	Current = NULL;
}


void SetString(const char* string) {
	if (!Current || !string) { return; }

	Current->type = TOKEN_STRING;
	Current->terminal = T_STRING;
	size_t length = strlen(string);

	//Pokud by se novy retezec nevlezl do bufferu, rozsirime buffer
	while ((Data.bufferIndex + length + 1) > Data.bufferSize) {
		ResizeBuffer();
	}

	char* tmp = &Data.textBuffer[Data.bufferIndex];
	memcpy(tmp, string, length);
	tmp[length++] = 0;
	Data.bufferIndex += length;
	Current->value = tmp;
	Current = NULL;
}


void SetEOL(void) {
	if (!Current) { return; }
	Current->type = TOKEN_EOL;
	Current->value = Miscellaneous[TOKEN_EOL];
	Current->terminal = T_EOL;
	Current = NULL;
}


void SetEOF(void) {
	if (!Current) { return; }
	Current->type = TOKEN_EOF;
	Current->terminal = T_EOF;
	Current = NULL;
}


void TokenCleanup(void) {
	//Uvolnime pamet
	if (Data.integers) {
		free(Data.integers);
		Data.integers = NULL;
	}
	if (Data.doubles) {
		free(Data.doubles);
		Data.doubles = NULL;
	}
	if (Data.textBuffer) {
		free(Data.textBuffer);
		Data.textBuffer = NULL;
	}
	if (Stash.tokenArray) {
		free(Stash.tokenArray);
		Stash.tokenArray = NULL;
	}

	//Reinicializujeme hodnoty pro pripadnou pristi alokaci
	Data.intsUsed = Data.intsSize = 0;
	Data.doublesUsed = Data.doublesSize = 0;
	Data.bufferIndex = Data.bufferSize = 0;
	Stash.activeToken = Stash.arrayUsed = Stash.arraySize = 0;
}

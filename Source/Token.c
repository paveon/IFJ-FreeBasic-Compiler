#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include "CompilationErrors.h"
#include "CodeGenerator.h"

#define KEYWORD_MAX_LEN 8
#define OPERATOR_MAX_LEN 2
#define MALLOC_SMALL_CHUNK 20
#define STASH_CHUNK 200
#define BUFFER_CHUNK 1024

struct Token {
	void** value;
	size_t bufferIdx;
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

static TokenStash g_Stash;
static ReadOnlyData g_Data;
static Token* g_Current;

typedef struct Pair {
	char* value;
	Terminal terminal;
} Pair;


static const Pair g_Keywords[] = {
				{"AS",       T_AS},
				{"ASC",      T_UNDEFINED},
				{"DECLARE",  T_DECLARE},
				{"DIM",      T_DIM},
				{"DO",       T_DO},
				{"DOUBLE",   T_DOUBLE},
				{"ELSE",     T_ELSE},
				{"END",      T_END},
				{"CHR",      T_UNDEFINED},
				{"FUNCTION", T_FUNCTION},
				{"IF",       T_IF},
				{"INPUT",    T_INPUT},
				{"INTEGER",  T_INTEGER},
				{"LENGTH",   T_UNDEFINED},
				{"LOOP",     T_LOOP},
				{"PRINT",    T_PRINT},
				{"RETURN",   T_RETURN},
				{"SCOPE",    T_SCOPE},
				{"STRING",   T_STRING},
				{"SUBSTR",   T_UNDEFINED},
				{"THEN",     T_THEN},
				{"WHILE",    T_WHILE},
				{"AND",      T_UNDEFINED},
				{"BOOLEAN",  T_UNDEFINED},
				{"CONTINUE", T_UNDEFINED},
				{"ELSEIF",   T_ELSEIF},
				{"EXIT",     T_UNDEFINED},
				{"FALSE",    T_UNDEFINED},
				{"FOR",      T_UNDEFINED},
				{"NEXT",     T_UNDEFINED},
				{"NOT",      T_UNDEFINED},
				{"OR",       T_UNDEFINED},
				{"SHARED",   T_SHARED},
				{"STATIC",   T_STATIC},
				{"TRUE",     T_UNDEFINED},
};

static const Pair g_Operators[] = {
				{"*",   T_OPERATOR_MULTIPLY},
				{"/",   T_OPERATOR_REAL_DIVIDE},
				{"\\",  T_OPERATOR_INT_DIVIDE},
				{"+",   T_OPERATOR_PLUS},
				{"-",   T_OPERATOR_MINUS},
				{"=",   T_OPERATOR_EQUAL},
				{"<>",  T_OPERATOR_NOT_EQ},
				{"<",   T_OPERATOR_LESS},
				{"<=",  T_OPERATOR_LESS_EQ},
				{">",   T_OPERATOR_GRT},
				{">=",  T_OPERATOR_GRT_EQ},
				{"+=",  T_OPERATOR_PLUS_EQ},
				{"-=",  T_OPERATOR_MINUS_EQ},
				{"*=",  T_OPERATOR_MULTIPLY_EQ},
				{"/=",  T_OPERATOR_REAL_DIVIDE_EQ},
				{"\\=", T_OPERATOR_INT_DIVIDE_EQ}
};


static char* g_Miscellaneous[5] = {
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
	g_Data.bufferSize += BUFFER_CHUNK;
	if ((newBuffer = realloc(g_Data.textBuffer, sizeof(char) * g_Data.bufferSize)) == NULL) {
		FatalError(ER_FATAL_INTERNAL);
	}
	g_Data.textBuffer = newBuffer;
}


Token* GetNextToken(void) {
	if (g_Stash.activeToken < g_Stash.arrayUsed) {
		return &g_Stash.tokenArray[g_Stash.activeToken++];
	}
	return NULL;
}


void ReturnToken(void) {
	if (g_Stash.activeToken > 0) {
		g_Stash.activeToken--;
	}
}


void CreateToken(void) {
	if (g_Stash.arrayUsed == g_Stash.arraySize) {
		Token* tmp = NULL;
		g_Stash.arraySize += STASH_CHUNK;
		if ((tmp = realloc(g_Stash.tokenArray, sizeof(Token) * g_Stash.arraySize)) == NULL) {
			FatalError(ER_FATAL_INTERNAL);
		}
		g_Stash.tokenArray = tmp;
	}

	g_Current = &g_Stash.tokenArray[g_Stash.arrayUsed++];
	g_Current->type = TOKEN_UNDEFINED;
	g_Current->value = NULL;
	g_Current->trailSpace = false;
}


TokenType GetTokenType(const Token* token) {
	return token->type;
}


const void* GetTokenValue(const Token* token) {
	if (token->type == TOKEN_IDENTIFIER || token->type == TOKEN_STRING) {
		return &((char*) (*token->value))[token->bufferIdx];
	}
	return *token->value;
}


Terminal GetTokenTerminal(const Token* token) {
	return token->terminal;
}


int GetTokenInt(const Token* token) {
	if (token->type == TOKEN_INTEGER) {
		return *((int*) (*token->value));
	}
	return -1;
}


double GetTokenDouble(const Token* token) {
	if (token->type == TOKEN_DOUBLE) {
		return *((double*) (*token->value));
	}
	return NAN;
}

bool GetTrailSpace(const Token* token) {
	return token->trailSpace;
}


void SetOperator(const char* operator) {
	if (!g_Current || !operator) { return; }
	size_t length = strlen(operator);

	//Nemuze se jednat o operator
	if (length == 0 || length > OPERATOR_MAX_LEN) { return; }

	size_t arraySize = (sizeof(g_Operators) / sizeof(char*));
	for (size_t i = 0; i < arraySize; i++) {
		if (strcmp(g_Operators[i].value, operator) == 0) {
			//Operator byl nalezen
			g_Current->type = TOKEN_OPERATOR;
			g_Current->value = (void**) &g_Operators[i].value;
			g_Current->terminal = g_Operators[i].terminal;
			g_Current = NULL;
			return;
		}
	}
}


void SetComma(void) {
	if (!g_Current) { return; }

	g_Current->type = TOKEN_COMMA;
	g_Current->value = (void**) &g_Miscellaneous[TOKEN_COMMA];
	g_Current->terminal = T_COMMA;
	g_Current = NULL;
}


void SetSemicolon(void) {
	if (!g_Current) { return; }

	g_Current->type = TOKEN_SEMICOLON;
	g_Current->value = (void**) &g_Miscellaneous[TOKEN_SEMICOLON];
	g_Current->terminal = T_SEMICOLON;
	g_Current = NULL;
}


void SetLeftBracket(void) {
	if (!g_Current) { return; }

	g_Current->type = TOKEN_L_BRACKET;
	g_Current->value = (void**) &g_Miscellaneous[TOKEN_L_BRACKET];
	g_Current->terminal = T_LEFT_BRACKET;
	g_Current = NULL;
}

void SetRightBracket(void) {
	if (!g_Current) { return; }

	g_Current->type = TOKEN_R_BRACKET;
	g_Current->value = (void**) &g_Miscellaneous[TOKEN_R_BRACKET];
	g_Current->terminal = T_RIGHT_BRACKET;
	g_Current = NULL;
}


void SetIdentifier(char* symbol) {
	if (!g_Current || !symbol) { return; }
	size_t length = strlen(symbol);
	if (length == 0) { return; }

	//Token je ukoncen pomoci libovolneho whitespace charakteru -> nemuze byt volani funkce
	if (isspace(symbol[length - 1])) {
		length--;
		symbol[length] = '\0';
		g_Current->trailSpace = true;
	}

	//Mohlo by se jednat o klicove slovo
	if (length <= KEYWORD_MAX_LEN) {
		StrToUpper(symbol);
		size_t arraySize = (sizeof(g_Keywords) / sizeof(Pair));
		for (size_t i = 0; i < arraySize; i++) {
			if (strcmp(g_Keywords[i].value, symbol) == 0) {
				g_Current->type = TOKEN_KEYWORD;
				g_Current->value = (void**) &g_Keywords[i].value;
				g_Current->terminal = g_Keywords[i].terminal;
				g_Current = NULL;
				return;
			}
		}
	}

	StrToLower(symbol);
	g_Current->type = TOKEN_IDENTIFIER;

	//Pokud by se novy retezec nevlezl do bufferu, rozsirime buffer
	while ((g_Data.bufferIndex + length + 1) > g_Data.bufferSize) {
		ResizeBuffer();
	}

	char* tmp = &g_Data.textBuffer[g_Data.bufferIndex];
	memcpy(tmp, symbol, length);
	tmp[length++] = 0;
	g_Current->bufferIdx = g_Data.bufferIndex;
	g_Current->value = (void**) &g_Data.textBuffer;
	g_Current->terminal = T_ID;
	g_Current = NULL;
	g_Data.bufferIndex += length;
}


void SetInteger(const char* number) {
	if (!g_Current || !number) { return; }
	size_t length = strlen(number);
	if (length == 0) { return; }

	//Predpoklada se validni hodnota predana z lexikalniho analyzatoru
	//TODO: vymyslet nejake reseni pro prilis velka cisla
	int value = (int) strtol(number, NULL, 10);
	g_Current->type = TOKEN_INTEGER;
	g_Current->terminal = T_INTEGER;

	for (size_t i = 0; i < g_Data.intsUsed; i++) {
		if (value == g_Data.integers[i]) {
			g_Current->value = (void**) &g_Data.integers;
			g_Current->bufferIdx = i;
			g_Current = NULL;
			return;
		}
	}

	if (g_Data.intsUsed == g_Data.intsSize) {
		int* tmp = NULL;
		g_Data.intsSize += MALLOC_SMALL_CHUNK;
		if ((tmp = realloc(g_Data.integers, sizeof(int) * g_Data.intsSize)) == NULL) {
			FatalError(ER_FATAL_INTERNAL);
		}
		g_Data.integers = tmp;
	}
	g_Data.integers[g_Data.intsUsed] = value;
	g_Current->value = (void**) &g_Data.integers;
	g_Current->bufferIdx = g_Data.intsUsed++;
	g_Current = NULL;
}


void SetDouble(const char* number) {
	size_t length = strlen(number);

	if (!g_Current || !number || length == 0) { return; }

	if (g_Data.doublesUsed == g_Data.doublesSize) {
		double* tmp = NULL;
		g_Data.doublesSize += MALLOC_SMALL_CHUNK;
		if ((tmp = realloc(g_Data.doubles, sizeof(double) * g_Data.doublesSize)) == NULL) {
			FatalError(ER_FATAL_INTERNAL);
		}
		g_Data.doubles = tmp;
	}

	//Predpoklada se validni hodnota predana z lexikalniho analyzatoru
	g_Data.doubles[g_Data.doublesUsed] = strtod(number, NULL);
	g_Current->type = TOKEN_DOUBLE;
	g_Current->value = (void**) &g_Data.doubles;
	g_Current->bufferIdx = g_Data.doublesUsed++;
	g_Current->terminal = T_DOUBLE;
	g_Current = NULL;
}


void SetString(const char* string) {
	if (!g_Current || !string) { return; }

	g_Current->type = TOKEN_STRING;
	g_Current->terminal = T_STRING;
	size_t length = strlen(string);

	//Pokud by se novy retezec nevlezl do bufferu, rozsirime buffer
	while ((g_Data.bufferIndex + length + 1) > g_Data.bufferSize) {
		ResizeBuffer();
	}

	char* tmp = &g_Data.textBuffer[g_Data.bufferIndex];
	memcpy(tmp, string, length);
	tmp[length++] = 0;
	g_Current->value = (void**) &g_Data.textBuffer;
	g_Current->bufferIdx = g_Data.bufferIndex;
	g_Current = NULL;
	g_Data.bufferIndex += length;
}


void SetEOL(void) {
	if (!g_Current) { return; }
	g_Current->type = TOKEN_EOL;
	g_Current->value = (void**) &g_Miscellaneous[TOKEN_EOL];
	g_Current->terminal = T_EOL;
	g_Current = NULL;
}


void SetEOF(void) {
	if (!g_Current) { return; }
	g_Current->type = TOKEN_EOF;
	g_Current->terminal = T_EOF;
	g_Current = NULL;
}


void TokenCleanup(void) {
	//Uvolnime pamet
	if (g_Data.integers) {
		free(g_Data.integers);
		g_Data.integers = NULL;
	}
	if (g_Data.doubles) {
		free(g_Data.doubles);
		g_Data.doubles = NULL;
	}
	if (g_Data.textBuffer) {
		free(g_Data.textBuffer);
		g_Data.textBuffer = NULL;
	}
	if (g_Stash.tokenArray) {
		free(g_Stash.tokenArray);
		g_Stash.tokenArray = NULL;
	}

	//Reinicializujeme hodnoty pro pripadnou pristi alokaci
	g_Data.intsUsed = g_Data.intsSize = 0;
	g_Data.doublesUsed = g_Data.doublesSize = 0;
	g_Data.bufferIndex = g_Data.bufferSize = 0;
	g_Stash.activeToken = g_Stash.arrayUsed = g_Stash.arraySize = 0;
}

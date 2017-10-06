#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "CompilationErrors.h"
#include "LLtable.h"
#include "Stack.h"

#define ITEMS_TOTAL 36
#define ITEMS_PREDICT 20

typedef u_char Rule;

/* Nemenit poradi, zavisi na tom indexovani. Lepsi zpusob me bohuzel zatim nenapadl */
static const char* const Terminals[] = {
		  "DECLARE", "DIM", "DO", "DOUBLE", "ELSE", "END", "FUNCTION", "IF",
		  "INPUT", "INTEGER", "LOOP", "PRINT", "RETURN", "SCOPE", "STRING",
		  ",", ")", "=", "\n", "ID", "(", "AS", "WHILE", "THEN", ";"
};

static Rule LLTable[11][20] = {
		  {2, 0,  0,  0,  0,  0,  3, 0,  0,  0,  0,  0,  0,  1, 0,  0, 0, 0,  0,  0},
		  {0, 0,  0,  0,  0,  0,  4, 0,  0,  0,  0,  0,  0,  0, 0,  0, 0, 0,  0,  0},
		  {0, 0,  0,  0,  0,  0,  5, 0,  0,  0,  0,  0,  0,  0, 0,  0, 0, 0,  0,  0},
		  {0, 0,  0,  0,  0,  0,  0, 0,  0,  0,  0,  0,  0,  0, 0,  0, 0, 0,  0,  6},
		  {0, 0,  0,  0,  0,  0,  0, 0,  0,  0,  0,  0,  0,  0, 0,  7, 8, 0,  0,  0},
		  {0, 9,  9,  0,  10, 10, 0, 9,  9,  0,  10, 9,  9,  0, 0,  0, 0, 0,  0,  9},
		  {0, 11, 15, 0,  0,  0,  0, 17, 13, 0,  0,  14, 16, 0, 0,  0, 0, 0,  0,  12},
		  {0, 0,  0,  0,  18, 19, 0, 0,  0,  0,  0,  0,  0,  0, 0,  0, 0, 0,  0,  0},
		  {0, 0,  0,  0,  0,  0,  0, 0,  0,  0,  0,  0,  0,  0, 0,  0, 0, 0,  21, 0},
		  {0, 0,  0,  0,  0,  0,  0, 0,  0,  0,  0,  0,  0,  0, 0,  0, 0, 22, 23, 0},
		  {0, 0,  0,  26, 0,  0,  0, 0,  0,  25, 0,  0,  0,  0, 24, 0, 0, 0,  0,  0}
};

bool ProduceString(Symbol* nonterm, Token* token, Symbol** stackTop) {
	TokenType type = GetType(token);
	const void* value = GetValue(token);
	size_t column = 0;
	size_t row = nonterm->data.nonTerminal;

	switch (type) {
		case TOKEN_IDENTIFIER:
			column = 19;
			break;

		case TOKEN_OPERATOR:
			if (strcmp(value, "=") != 0) {
				return false;
			}
		case TOKEN_KEYWORD:
		case TOKEN_EOL:
		case TOKEN_R_BRACKET:
			for (size_t i = 0; i < ITEMS_PREDICT - 1; i++) {
				if (strcmp(value, Terminals[i]) == 0) {
					column = i;
				}
			}
			break;
		default:
			return false;
	}

	Symbol* newString;
	Rule rule = LLTable[row][column];
	switch (rule) {
		case 0:
			return false;
		case 1:
			//newString = CreateString()
			break;
		case 2:
			break;
		case 3:
			break;
		case 4:
			break;
		case 5:
			break;
		case 6:
			break;
		case 7:
			break;
		case 8:
			break;
		case 9:
			break;
		case 10:
			break;
		case 11:
			break;
		case 12:
			break;
		case 13:
			break;
		case 14:
			break;
		case 15:
			break;
		case 16:
			break;
		case 17:
			break;
		case 18:
			break;
		case 19:
			break;
		case 20:
			break;
		case 21:
			break;
		case 22:
			break;
		case 23:
			break;
		case 24:
			break;
		case 25:
			break;
		case 26:
			break;
	}


	return true;
}

void CreateString(Symbol** stackTop, size_t length) {
	//TODO: prepsat s vyuzitim noveho zpusobu alokace

//	Symbol* newSymbol;
//
//	//Abychom se vyhli zbytecne podmince v cyklu
//	if((newSymbol = malloc(sizeof(Symbol))) == NULL){
//		FatalError(ER_FATAL_INTERNAL);
//	}
//	if(*stackTop){
//		(*stackTop)->up = newSymbol;
//	}
//	newSymbol->down = *stackTop;
//	newSymbol->reduceEnd = false;
//	*stackTop = newSymbol;
//	length--;
//
//
//	/* Symboly nemohou byt alokovany vsechny naraz, protoze by nesly postupne
//	 * uvolnovat a dochazelo by k chybam uvolnovani pri predani ukazatele do
//	 * stredu pole symbolu funkci free (v poli NELZE uvolnit konkretni prvek)
//	 */
//	for(size_t index = 0; index < length; index++){
//		//Nove vytvorene symboly jsou jiz na zasobniku a jsou automaticky mazany pri chybe
//		if((newSymbol = malloc(sizeof(Symbol))) == NULL){
//			FatalError(ER_FATAL_INTERNAL);
//		}
//		(*stackTop)->up = newSymbol;
//		newSymbol->down = *stackTop;
//		newSymbol->reduceEnd = false;
//		*stackTop = newSymbol;
//	}
//	newSymbol->up = NULL;
}
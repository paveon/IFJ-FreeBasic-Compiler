//
// Created by Bobek on 14.10.2017.
//

#ifndef FREEBASIC_COMPILER_PRECEDENTTABLE_H
#define FREEBASIC_COMPILER_PRECEDENTTABLE_H

#include "Symbol.h"
#include "Token.h"
#include "Stack.h"
#include "symtable.h"
#include "CompilationErrors.h"

#define FINDING_FAILURE 10
#define MAX_ERR_COUNT 3


typedef struct IdxTerminalPair {
	size_t cell_value;
	Terminal incoming_term;
}IdxTerminalPair;


/* Cisla odpovidajici jednotlivym prioritam v g_PrecedentTable
 */
typedef enum OpPriority {
	EXPR_ERROR = 0,
	LOWER_PR = 1,
	SKIP_PR = 2,
	HIGHER_PR = 3,
}OpPriority;


/* Indexovani v g_PrecedentTable, kde jsou vypsany jednotlive priority operatoru
 */
typedef enum PrecOperators {
	OPERATOR_PLUS = 0,
	OPERATOR_MINUS = 1,
	OPERATOR_MULTIPLY = 2,
	OPERATOR_REAL_DIVIDE = 3,
	OPERATOR_INT_DIVIDE = 4,
	OPERATOR_LESS = 5,
	OPERATOR_LESS_EQ = 6,
	OPERATOR_GRT = 7,
	OPERATOR_GRT_EQ = 8,
	OPERATOR_NOT_EQ = 9,
	OPERATOR_EQ = 10,
	OPERATOR_L_BRACKET = 11,
	OPERATOR_R_BRACKET = 12,
	IDENTIFIER = 13,
	FUNCTION_IDENTIFIER = 14,
	OPERATOR_COMMA = 15,
	END_SYMBOL = 16,
};


/* Funkce hledajici pravidlo v precedencni tabulce. Hodnoty navraci do promenne "field" predane parametrem.
 * Pokud prisel token neodpovidajici zapisu vyrazu, funkce vraci na index TABLE_CELL(0) hodnotu FINDING_FAILURE(10).
 * Pri nedefinovane promenne vypise error, dale pokracuje, ale nerozlisuje zda byla promenna funkce ci identifikator.
 * Pokud nebyla definovana, tak se bere jako identifikator.
 */
void FindInTable(Stack *s, struct IdxTerminalPair *field, size_t line_num, bool is_in_func);


/* Funkce, ktera vybira ze stacku symboly a uklada je do bufferu, dokud nenarazi na reductionEnd. Po nalezeni
 * zkontroluje zda k vybranym symbolum sedi jedno z pravidel. Pokud ano vraci true bez chybove hlasky.
 * Pokud nenajde vhodne pravidlo vypise error, ze se jedna o neznamy zapis vyrazu a vraci false.
 */
bool ApplyPrecRule(Stack *s, bool is_in_func, size_t line_num);


#endif //FREEBASIC_COMPILER_PRECEDENTTABLE_H

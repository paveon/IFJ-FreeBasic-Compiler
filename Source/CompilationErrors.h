#include <stdio.h>
#include <stdlib.h>

#ifndef FREEBASIC_COMPILER_COMPILATIONERRORS_H
#define FREEBASIC_COMPILER_COMPILATIONERRORS_H

typedef enum ErrorCode {
	ER_FATAL_LEXICAL_ANALYSIS,
	ER_FATAL_SYNTAX_ANALYSIS,
	ER_FATAL_SEMANTIC_DEFINITIONS,
	ER_FATAL_SEMANTIC_TYPES,
	ER_FATAL_SEMANTIC_OTHER,
	ER_FATAL_INTERNAL, //Chyba pri alokaci pameti
	ER_SEMANTIC_VAR_REDECL, //Redeklarace lokalni promenne
	ER_SEMANTIC_DECL_AFTER_DEF, //Deklarace az po definici funkce
	ER_SEMANTIC_FUNC_REDECL, //Redeklarace funkce
	ER_SEMANTIC_FUNC_REDEF, //Redefinice funkce
	ER_SEMANTIC_PARAM_REDEF, //Dva parametry se stejnym nazvem
	ER_SEMANTIC_PARAM_COUNT, //Pocet parametru v definici funkce neodpovida deklaraci
	ER_SEMANTIC_PARAM_MISMATCH, //Nekompatibilita typu parametru u deklarace a defince
	ER_SEMANTIC_RETURN_MISMATCH //Nekompatibilita navratoveho typu u deklarace a definice
} ErrorCode;

void SemanticError(size_t line, ErrorCode errorCode);
void FatalError(const char* function, const char* sourceFile, int line, ErrorCode errorCode);

#define FatalError(errorCode) FatalError(__func__, __FILE__, __LINE__, errorCode)

#endif //FREEBASIC_COMPILER_COMPILATIONERRORS_H

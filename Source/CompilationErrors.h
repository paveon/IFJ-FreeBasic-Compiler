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
	ER_SMC_VAR_REDECL, //Redeklarace lokalni promenne
	ER_SMC_FUNC_DECL_AFTER_DEF, //Deklarace az po definici funkce
	ER_SMC_FUNC_REDECL, //Redeklarace funkce
	ER_SMC_FUNC_REDEF, //Redefinice funkce
	ER_SMC_FUNC_NO_DEF, //Chybejici definice funkce
	ER_SMC_FUNC_PARAM_REDEF, //Dva parametry se stejnym nazvem
	ER_SMC_FUNC_PARAM_COUNT, //Pocet parametru v definici funkce neodpovida deklaraci
	ER_SMC_FUNC_PARAM_TYPE, //Nekompatibilita typu parametru u deklarace a defince
	ER_SMC_FUNC_RETURN_TYPE, //Nekompatibilita navratoveho typu u deklarace a definice
	ER_SMC_VAR_UNDEF, // Nedefinovana promenna ve vyrazech
	ER_SMC_MISSING_OP, // Chybejici operator ve vyrazu
	ER_SMC_UNKNOWN_EXPR, // Nenalezeno vhodne pravidlo v tabulce
} ErrorCode;

void SemanticError(size_t line, ErrorCode errorCode, const char* extra);
void FatalError(const char* function, const char* sourceFile, int line, ErrorCode errorCode);

#define FatalError(errorCode) FatalError(__func__, __FILE__, __LINE__, errorCode)

#endif //FREEBASIC_COMPILER_COMPILATIONERRORS_H

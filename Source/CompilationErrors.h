#include <stdio.h>
#include <stdlib.h>

#ifndef FREEBASIC_COMPILER_COMPILATIONERRORS_H
#define FREEBASIC_COMPILER_COMPILATIONERRORS_H

typedef enum ErrorCode {
	ER_FATAL_INTERNAL, //Chyba pri alokaci pameti
	ER_SMC_VAR_REDECL, //Redeklarace lokalni promenne
	ER_SMC_VAR_UNDEF, //Nedefinovana promenna ve vyrazech
	ER_SMC_VAR_TYPE, //Typ promenne neni kompatibilni s vyslednym typem vyrazu
	ER_SMC_FUNC_DECL_AFTER_DEF, //Deklarace az po definici funkce
	ER_SMC_FUNC_REDECL, //Redeklarace funkce
	ER_SMC_FUNC_REDEF, //Redefinice funkce
	ER_SMC_FUNC_NO_DEF, //Chybejici definice funkce
	ER_SMC_FUNC_PARAM_REDEF, //Dva parametry se stejnym nazvem
	ER_SMC_FUNC_PARAM_COUNT, //Pocet parametru v definici funkce neodpovida deklaraci
	ER_SMC_FUNC_PARAM_TYPE, //Nekompatibilita typu parametru u deklarace a defince
	ER_SMC_FUNC_RETURN_TYPE, //Nekompatibilita navratoveho typu u deklarace a definice
	ER_SMC_FUNC_RETURN_EXPR, //Chybny vysledny typ vyrazu u volani return ve funkci
	ER_SMC_MISSING_OP, //Chybejici operator ve vyrazu
	ER_SMC_UNKNOWN_EXPR, //Nenalezeno vhodne pravidlo v tabulce
	ER_SMC_UNEXPECT_SYM, //Neocekavany symbol (';' bez printu apod.)
	ER_SMC_UNEXP_FUNC_SPACE, //Mezera za identifikatorem funkce
	ER_SMC_FUNC_UNDECL, //Nedeklarovana funkce
	ER_SMC_COMPARATIVE_EXPR, //Pouzivani porovnavacich operatoru v klasickych vyrazech
	ER_SMC_STR_AND_NUM, //Spojeni retezce a cisel do jednoho vyrazu
	ER_SMC_MANY_ARGS, //Prilis mnoho parametru pri zadavani funkce
	ER_SMC_LESS_ARGS, //Malo argumentu pri volani funkce
	ER_SMC_ARG_TYPES, //Spatny typ argumentu funkce
	ER_SMC_INT_DIV, //Pokus o celociselne deleni s necelociselnymi typy
} ErrorCode;

void SemanticError(size_t line, ErrorCode errorCode, const char* extra);

void FatalError(const char* function, const char* sourceFile, int line, ErrorCode errorCode);

#define FatalError(errorCode) FatalError(__func__, __FILE__, __LINE__, errorCode)

#endif //FREEBASIC_COMPILER_COMPILATIONERRORS_H

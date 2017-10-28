#include <stdio.h>
#include <stdlib.h>
#include "LLtable.h"
#include "Stack.h"
#include "TopDown.h"
#include "symtable.h"
#include "Lexical.h"
#include "BottomUp.h"


int main(int argc, char* argv[]) {
	printf("Test run started...\n");
	Token* token;
	freopen("test.txt", "r", stdin);
	bool lex = Lexical();
	bool bottom = BottomUp(1, T_PRINT);
	printf("brah");


	char keyword[] = "aSc";
	char id[] = "id_with_space ";

	CreateToken();
	SetComma();
	SetComma();
	CreateToken();
	SetSemicolon();

	CreateToken();
	SetIdentifier(keyword);
	SetIdentifier(keyword);

	CreateToken();
	SetString("");

	CreateToken();
	SetIdentifier(keyword);

	CreateToken();
	SetIdentifier(id);

	CreateToken();
	SetIdentifier(id);

	CreateToken();
	SetDouble("1.25");

	CreateToken();
	SetDouble("1.25");

	CreateToken();
	SetInteger("25231");

	/* test uvolneni pameti */
	//FatalError(ER_FATAL_INTERNAL);

	CreateToken();
	SetInteger("25231");

	CreateToken();
	SetString("RetezcovyLiteral");

	CreateToken();
	SetEOF();
	CreateToken();
	SetEOL();

	Variable* variable;
	Function* function;
	const char* name;
	while ((token = GetNextToken())) {
		if (GetTokenType(token) == TOKEN_IDENTIFIER) {
			name = GetTokenValue(token);
			BeginSubScope();

			function = InsertFunction(name, true, 0); //Funkce
			if (function)
				function->returnType = 's';

			variable = InsertVariable(name, true, 0); //Globalni promenna
			if (variable)
				variable->type = 'i';

			variable = InsertVariable(name, false, 0); //Lokalni promenna
			if (variable)
				variable->type = 'd';

			variable = LookupVariable(name, false); //Hledat pouze lokalni promenne
			variable = LookupVariable(name, true); //I globalni
			function = LookupFunction(name); //Funkce
			EndSubScope();
			variable = LookupVariable(name, false); //Pouze lokalni promenne
			variable = LookupVariable(name, true); //I globalni
		}
	}
	EndScope();

	/* test uvolneni pameti */
	//FatalError(ER_FATAL_INTERNAL);

	BeginSubScope();
	BeginSubScope();
	EndSubScope();
	BeginSubScope();
	BeginSubScope();
	EndSubScope();
	EndSubScope();
	EndSubScope();
	BeginSubScope();
	EndSubScope();


	TokenCleanup();
	TableCleanup();
	StackCleanup();
	TopDownCleanup();
	printf("Test run finished, allocated memory should be released...\n");
	return 0;
}
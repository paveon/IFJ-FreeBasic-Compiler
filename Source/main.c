#include <stdio.h>
#include <stdlib.h>
#include "LLtable.h"
#include "Stack.h"
#include "TopDown.h"
#include "symtable.h"
#include "Lexical.h"
#include "CodeGenerator.h"


int main(int argc, char* argv[]) {
	printf("Test run started...\n");
	Token* token;

	if (argc < 2) {
		printf("Expected source file name, exiting...\n");
		exit(0);
	}
	if (freopen(argv[1], "r", stdin) == NULL) {
		printf("Couldn't open specified source file (doesn't exist?), exiting...\n");
		exit(0);
	}

	bool result = Lexical();
	printf("Parsing simple program...\n");
	if (ParseProgram()) {
		printf("Program parsed...\n");
	}
	else {
		fprintf(stderr, "Program contains an error!\n");
	}


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
				function->returnType = T_STRING;

			variable = InsertVariable(name, true, 0); //Globalni promenna
			if (variable)
				variable->type = T_INTEGER;

			variable = InsertVariable(name, false, 0); //Lokalni promenna
			if (variable)
				variable->type = T_DOUBLE;

			variable = LookupVariable(name, false, false); //Hledat pouze lokalni promenne
			variable = LookupVariable(name, false, true); //I globalni
			function = LookupFunction(name); //Funkce
			EndSubScope();
			variable = LookupVariable(name, false, false); //Pouze lokalni promenne
			variable = LookupVariable(name, false, true); //I globalni
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

	OutputCode();

	TokenCleanup();
	TableCleanup();
	StackCleanup();
	TopDownCleanup();
	GeneratorCleanup();
	printf("Test run finished, allocated memory should be released...\n");
	return 0;
}
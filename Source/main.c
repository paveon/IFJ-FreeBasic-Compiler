#include <stdio.h>
#include <stdlib.h>
#include "LLtable.h"
#include "Stack.h"
#include "TopDown.h"
#include "symtable.h"
#include "Lexical.h"


int main() {
	printf("Test run started...\n");
	Token* token;
	char tokens[30][20] = {
			  [T_DECLARE] = "declare",
			  [T_END] = "end",
			  [T_SCOPE] = "scope",
			  [T_DIM] = "dim",
			  [T_AS] = "as",
			  [T_STRING] = "string",
			  [T_INTEGER] = "integer",
			  [T_DOUBLE] = "double",
			  [T_ID] = "tmpValue",
			  [T_RETURN] = "return",
					[T_FUNCTION] = "function"
	};

	/* Test prazdnych radku pred zacatkem programu */
//	CreateToken();
//	SetEOL();
//
//	/* Deklarace 'declare function Foo() as double' */
//	CreateToken();
//	SetIdentifier(tokens[T_DECLARE]);
//	CreateToken();
//	SetIdentifier(tokens[T_FUNCTION]);
//	CreateToken();
//	SetIdentifier(tokens[T_ID]);
//	CreateToken();
//	SetLeftBracket();
//	CreateToken();
//	SetRightBracket();
//	CreateToken();
//	SetIdentifier(tokens[T_AS]);
//	CreateToken();
//	SetIdentifier(tokens[T_DOUBLE]);
//	CreateToken();
//	SetEOL();
//
//	/* Vice prazdnych radku za deklaraci */
//	CreateToken();
//	SetEOL();
//
//	/* Redeklarace 'declare function Foo(var as string) as double' */
//	CreateToken();
//	SetIdentifier(tokens[T_DECLARE]);
//	CreateToken();
//	SetIdentifier(tokens[T_FUNCTION]);
//	CreateToken();
//	SetIdentifier(tokens[T_ID]);
//	CreateToken();
//	SetLeftBracket();
//	CreateToken();
//	SetIdentifier(tokens[T_ID]);
//	CreateToken();
//	SetIdentifier(tokens[T_AS]);
//	CreateToken();
//	SetIdentifier(tokens[T_STRING]);
//	CreateToken();
//	SetRightBracket();
//	CreateToken();
//	SetIdentifier(tokens[T_AS]);
//	CreateToken();
//	SetIdentifier(tokens[T_DOUBLE]);
//	CreateToken();
//	SetEOL();
//
//
//	/* Definice 'function Foo(var as string) as double'
//	 * Nekompatibilita typu
//	 */
//	CreateToken();
//	SetIdentifier(tokens[T_FUNCTION]);
//	CreateToken();
//	SetIdentifier(tokens[T_ID]);
//	CreateToken();
//	SetLeftBracket();
//	CreateToken();
//	SetIdentifier(tokens[T_ID]);
//	CreateToken();
//	SetIdentifier(tokens[T_AS]);
//	CreateToken();
//	SetIdentifier(tokens[T_STRING]);
//	CreateToken();
//	SetRightBracket();
//	CreateToken();
//	SetIdentifier(tokens[T_AS]);
//	CreateToken();
//	SetIdentifier(tokens[T_DOUBLE]);
//	CreateToken();
//	SetEOL();
//	CreateToken();
//	SetIdentifier(tokens[T_END]);
//	CreateToken();
//	SetIdentifier(tokens[T_FUNCTION]);
//	CreateToken();
//	SetEOL();
//
//
//	CreateToken();
//	SetIdentifier(tokens[T_SCOPE]);
//	CreateToken();
//	SetEOL();
//
//	//Statements
//	CreateToken();
//	SetIdentifier(tokens[T_DIM]);
//	CreateToken();
//	SetIdentifier(tokens[T_ID]);
//	CreateToken();
//	SetIdentifier(tokens[T_AS]);
//	CreateToken();
//	SetIdentifier(tokens[T_DOUBLE]);
//	CreateToken();
//	SetEOL();
//
//	CreateToken();
//	SetIdentifier(tokens[T_DIM]);
//	CreateToken();
//	SetIdentifier(tokens[T_ID]);
//	CreateToken();
//	SetIdentifier(tokens[T_AS]);
//	CreateToken();
//	SetIdentifier(tokens[T_DOUBLE]);
//	CreateToken();
//	SetEOL();
//
////	CreateToken();
////	SetIdentifier(tokens[T_RETURN]);
////	CreateToken();
////	SetEOL();
//
//	//End
//	CreateToken();
//	SetIdentifier(tokens[T_END]);
//	CreateToken();
//	SetIdentifier(tokens[T_SCOPE]);
//
//	/* Test prazdnych radku na konci programu */
//	CreateToken();
//	SetEOL();
//	CreateToken();
//	SetEOL();
//
//	/* Konec programu */
//	CreateToken();
//	SetEOF();

	if (freopen("sampleProgram", "r", stdin) == NULL) {
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

	Identifier* newID;
	while ((token = GetNextToken())) {
		if (GetTokenType(token) == TOKEN_IDENTIFIER) {
			BeginSubScope();
			InsertGlobalID(GetTokenValue(token));
			newID = InsertLocalID(GetTokenValue(token));
			newID = LookupID(GetTokenValue(token));
			EndSubScope();
			newID = LookupID(GetTokenValue(token));
			newID = LookupGlobalID(GetTokenValue(token));
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
	TableCleanup(true);
	StackCleanup();
	TopDownCleanup();
	printf("Test run finished, allocated memory should be released...\n");
	return 0;
}
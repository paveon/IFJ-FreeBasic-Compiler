#include <stdio.h>
#include "LLtable.h"
#include "Stack.h"
#include "TopDown.h"
#include "symtable.h"
#include "CompilationErrors.h"

int main() {
	printf("Test run started...\n");
	Token* token;
	SymbolType symbolType;
	TokenType tokenType;
	char tokens[][20] = {
			  "declare",
			  "end",
			  "scope",
			  "dim",
			  "as",
			  "string",
			  "integer",
			  "double",
			  "tmpValue",
			  "return",
			  "function",
			  //function id (<argument>) as <type> EOL
	};
	CreateToken();
	SetIdentifier(tokens[2]);
	CreateToken();
	SetEOL();

	//Statements
	CreateToken();
	SetIdentifier(tokens[3]);
	CreateToken();
	SetIdentifier(tokens[8]);
	CreateToken();
	SetIdentifier(tokens[4]);
	CreateToken();
	SetIdentifier(tokens[7]);
	CreateToken();
	SetEOL();

	CreateToken();
	SetIdentifier(tokens[9]);
	CreateToken();
	SetEOL();

	//End
	CreateToken();
	SetIdentifier(tokens[1]);
	CreateToken();
	SetIdentifier(tokens[2]);
	CreateToken();
	SetEOF();
	printf("Parsing simple program...\n");
	//printf("%s\n\t<epsilon>\n%s %s\n\n", tokens[2], tokens[1], tokens[2]);
	ParseProgram();
	printf("Program parsed...\n");



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

	//const void* tmp = GetTokenValue(token);
	//tmp[0] = '\0';

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
	printf("Test run finished, allocated memory should be released...\n");
	return 0;
}
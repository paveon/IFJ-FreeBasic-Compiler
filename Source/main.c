#include <stdio.h>
#include <stdlib.h>
#include "Token.h"
#include "symtable.h"

int main() {
	printf("Test run started...\n");
	char keyword[] = "aSc";
	char id[] = "id_with_space ";
	CreateToken();
	SetIdentifier(keyword);
	SetIdentifier(keyword);

	CreateToken();
	SetString("");

	CreateToken();
	SetIdentifier(keyword);

	CreateToken();
	SetIdentifier(id);

	//const void* tmp = GetValue(token);
	//tmp[0] = '\0';

	CreateToken();
	SetIdentifier(id);

	CreateToken();
	SetDouble("1.25");

	CreateToken();
	SetDouble("1.25");

	CreateToken();
	SetInteger("25231");

	CreateToken();
	SetInteger("25231");

	CreateToken();
	SetString("RetezcovyLiteral");

	CreateToken();
	SetEOF();
	CreateToken();
	SetEOL();

	Symbol* newSymbol;
	Token* token;
	while ((token = GetNextToken())) {
		if (GetType(token) == TOKEN_IDENTIFIER) {
			BeginSubScope();
			InsertGlobalSymbol(GetValue(token));
			newSymbol = InsertSymbol(GetValue(token));
			newSymbol = LookupSymbol(GetValue(token));
			EndSubScope();
			newSymbol = LookupSymbol(GetValue(token));
			newSymbol = LookupGlobalSymbol(GetValue(token));
		}
	}
	EndScope();

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
	printf("Test run finished, allocated memory should be released...\n");
	return 0;
}
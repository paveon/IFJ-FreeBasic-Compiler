#include <stdio.h>
#include <stdlib.h>
#include "Token.h"
#include "symtable.h"

int main() {
	printf("Test run started...\n");
	char keyword[] = "aSc";
	char id[] = "testIdentifikator";
	Token* token = CreateToken();
	SetIdentifier(token, keyword);

	token = CreateToken();
	SetString(token, "");

	token = CreateToken();
	SetIdentifier(token, keyword);

	token = CreateToken();
	SetIdentifier(token, id);

	//const void* tmp = GetValue(token);
	//tmp[0] = '\0';

	token = CreateToken();
	SetIdentifier(token, id);

	token = CreateToken();
	SetDouble(token, "1.25");

	token = CreateToken();
	SetDouble(token, "1.25");

	token = CreateToken();
	SetInteger(token, "25231");

	token = CreateToken();
	SetInteger(token, "25231");

	token = CreateToken();
	SetString(token, "RetezcovyLiteral");

	token = CreateToken();
	SetEOF(token);
	token = CreateToken();
	SetEOL(token);

	Symbol* newSymbol;
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
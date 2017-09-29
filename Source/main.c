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

	const void* tmp = GetValue(token);
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

	SymbolTable* scope = CreateScope(GetMainScope());
	while ((token = GetNextToken())) {
		scope = CreateScope(scope);
		if (GetType(token) == TOKEN_IDENTIFIER) {
			InsertSymbol(scope, GetValue(token));
		}
	}

	TokenCleanup();
	TableCleanup();
	printf("Test run finished, allocated memory should be released...\n");
	return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include "Token.h"
#include "LLtable.h"
#include "symtable.h"
#include "Stack.h"
#include "CompilationErrors.h"

int main() {
	printf("Test run started...\n");

	Stack* stack = GetStack();

	//Simulace derivace neterminalu <prog> do deklarace funkce
	PushNT(stack, NT_PROGRAM);
	PushNT(stack, NT_HEADER);
	PushT(stack, T_DECLARE);
	ReleaseStack(stack);

	//Mel by znovupouzit existujici stack
	stack = GetStack();

	//Simulace derivace neterminalu <prog> dalsim pravidlem
	PushT(stack, T_SCOPE);
	PushT(stack, T_END);
	PushNT(stack, NT_STATEMENT_LIST);

	/* test uvolneni pameti */
	//FatalError(ER_FATAL_INTERNAL);

	PushT(stack, T_EOL);
	PushT(stack, T_SCOPE);



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
	Token* token;
	while ((token = GetNextToken())) {
		if (GetType(token) == TOKEN_IDENTIFIER) {
			BeginSubScope();
			InsertGlobalID(GetValue(token));
			newID = InsertLocalID(GetValue(token));
			newID = LookupID(GetValue(token));
			EndSubScope();
			newID = LookupID(GetValue(token));
			newID = LookupGlobalID(GetValue(token));
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
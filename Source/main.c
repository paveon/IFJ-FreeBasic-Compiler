#include <stdio.h>
#include "LLtable.h"
#include "Stack.h"
#include "symtable.h"
#include "CompilationErrors.h"

int main() {
	printf("Test run started...\n");
	Token* token;
	SymbolType symbolType;
	TokenType tokenType;
	char keywords[][20] = {
			  "declare",
			  "end",
			  "scope"
	};
	CreateToken();
	SetIdentifier(keywords[2]);
	CreateToken();
	SetEOL();
	CreateToken();
	SetIdentifier(keywords[1]);
	CreateToken();
	SetIdentifier(keywords[2]);
	CreateToken();
	SetEOF();
	Stack* stack = GetStack();
	PushNT(stack, NT_PROGRAM);

	printf("Parsing simple program...\n");
	printf("%s\n\t<epsilon>\n%s %s\n\n", keywords[2], keywords[1], keywords[2]);
	token = GetNextToken();
	while (true) {
		symbolType = GetSymbolType(stack);
		tokenType = GetTokenType(token);
		if (symbolType == SYMBOL_BOTTOM) {
			if (tokenType == TOKEN_EOF) {
				printf("Simple program is correct!\n");
			}
			else {
				fprintf(stderr, "Syntax error!\n");
			}
			break;
		}
		else if (symbolType == SYMBOL_TERMINAL) {
			printf("-- Comparing terminals --\n");
			if (CompareTop(stack, token)) {
				printf("-- Terminals are equal --\n");
				PopSymbol(stack);
				token = GetNextToken();
			}
			else {
				fprintf(stderr, "Syntax error!\n");
				break;
			}
		}
		else if (symbolType == SYMBOL_NONTERMINAL) {
			while (symbolType == SYMBOL_NONTERMINAL) {
				printf("-- Derivating --\n");
				if (ExpandTop(stack, token)) {
					printf("-- Derivation successful --\n");
					symbolType = GetSymbolType(stack);
				}
				else {
					fprintf(stderr, "Syntax error!\n");
					break;
				}
			}
		}
		else {
			fprintf(stderr, "Unknown error!\n");
			break;
		}
	}
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
#include <stdio.h>
#include "Token.h"

int main() {
	printf("Hello, World!\n");
	Token* token = CreateToken();
	char keyword[] = "aSc";
	TokenSetOperator(token, ">=");
	TokenSetKeyword(token, keyword);
	TokenSetIdentifier(token, "testIdentifikator");
	return 0;
}
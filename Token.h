#ifndef FREEBASIC_COMPILER_TOKEN_H
#define FREEBASIC_COMPILER_TOKEN_H


typedef struct Token Token;

Token* CreateToken(void);

void TokenSetOperator(Token* token, const char* operator);

void TokenSetKeyword(Token* token, char* operator);

void TokenSetIdentifier(Token* token, const char* identifier);


#endif //FREEBASIC_COMPILER_TOKEN_H

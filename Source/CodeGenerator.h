#ifndef FREEBASIC_COMPILER_CODEGENERATOR_H
#define FREEBASIC_COMPILER_CODEGENERATOR_H

#include "Token.h"

/*
 * Pomocne premenne budu generovane ako globalne pre vypocet vyrazov
 */
typedef struct ExpressionVars {
	size_t number;
	char variables[5][20];
} ExpressionVars;


void InsertRule(size_t ruleID);

void PushToken(Token* token);

void PopToken(void);

void GenerateCode(void);

void OutputCode(void);

void GeneratorCleanup(void);



#endif //FREEBASIC_COMPILER_CODEGENERATOR_H

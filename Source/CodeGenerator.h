#ifndef FREEBASIC_COMPILER_CODEGENERATOR_H
#define FREEBASIC_COMPILER_CODEGENERATOR_H

#include "Token.h"

void InsertRule(size_t ruleID);

void PushToken(Token* token);

void PopToken(void);

void GenerateCode(void);

void OutputCode(void);

void GeneratorCleanup(void);

void PushString(char* newString);

char *TypeToStringForInit(Terminal type);

char *ScopeToString(bool global);

const void* FindID(int *tokenPos);

#endif //FREEBASIC_COMPILER_CODEGENERATOR_H

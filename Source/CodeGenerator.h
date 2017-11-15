#ifndef FREEBASIC_COMPILER_CODEGENERATOR_H
#define FREEBASIC_COMPILER_CODEGENERATOR_H

#include "Token.h"
#include "CompilationErrors.h"
#include "LLtable.h"
#include "symtable.h"
#include <string.h>
#include "PrecedentTable.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

/*
 * Pomocne premenne budu generovane ako globalne pre vypocet vyrazov
 */



void InsertRule(size_t ruleID);

void PushToken(Token* token);

void PopToken(void);

void GenerateCode(void);

void OutputCode(void);

void GeneratorCleanup(void);



#endif //FREEBASIC_COMPILER_CODEGENERATOR_H

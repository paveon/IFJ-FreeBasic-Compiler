//
// Created by Bobek on 20.10.2017.
//

#ifndef FREEBASIC_COMPILER_BOTTOMUP_H
#define FREEBASIC_COMPILER_BOTTOMUP_H

#include "PrecedentTable.h"

extern int g_err_counter;

bool BottomUp(size_t line_num);

int FuncParams(Stack *s, IdxTerminalPair value_pair, size_t line_num);

#endif //FREEBASIC_COMPILER_BOTTOMUP_H

//
// Created by Bobek on 20.10.2017.
//

#ifndef FREEBASIC_COMPILER_BOTTOMUP_H
#define FREEBASIC_COMPILER_BOTTOMUP_H

#include "PrecedentTable.h"


/* Funkce zpracovavajici syntaktickou analyzu zdola nahoru.
 * Prebira radek, na kterem se vyraz nachazi a take klicove slovo jemu predchazejici
 * (T_PRINT, T_IF, T_WHILE, T_EQUAL), aby byl schopny urcit s cim pracuje
 */
Terminal BottomUp(size_t line_num, Terminal keyword);

int FuncParams(Stack *s, IdxTerminalPair values, size_t line_num, Terminal keyword);

void FreeTypeBuffer(void);

#endif //FREEBASIC_COMPILER_BOTTOMUP_H

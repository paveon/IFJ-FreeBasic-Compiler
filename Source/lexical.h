//
// Created by Richard Gall on 03.10.2017.
//
#ifndef LEXICAL_H
#define LEXICAL_H
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include "lexical.h"
#include <stddef.h>

char* createBuffer(); //possibly redundant

void appendToBuff(char *buffPtr,int* lenght,char c);

void clearBuffer(char* buffPtr,int* lenght);

typedef enum {
    start,
    word,
    number,
    commentP,
    commentS,
    floatLit,
    stringLit,
    relat,
    fail,
}state;


int isEnd(char c);

int lexical();

void printLexError();

#endif //LEXICAL_H

//
// Created by Richard Gall on 03.10.2017.
//
#ifndef LEXICAL_H
#define LEXICAL_H

typedef enum {
   START,
   WORD,
   NUMBER,
   COMMENTP,
   COMMENTS,
   FLOAT,
   STRING,
   RELAT,
   FAIL,
}state;


char* CreateBuffer(); //possibly redundant

char* AppendToBuff(char *buffPtr,int* lenght,char c);

void ClearBuffer(char* buffPtr,int* lenght);

void MakeShortToken(int type, char c);

void SetLex(state* currentState,char chartype,char* buffer,int* lenght);

int IsEnd(char c);

int Lexical();

void PrintLexError();

#endif //LEXICAL_H

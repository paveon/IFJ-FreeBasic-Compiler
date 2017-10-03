//
// Created by Richard Gall on 03.10.2017.
//
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include "lexical.h"
#include <stddef.h>


/* Toto je dočasná práce, funkčnost některých funkcí je čistě náhodná, komentáře existují ale jsou velmi dobře ukryty
 * V absolutně žádném případě se nepokoušet číst,pochopit či použít jakoukoliv část této knihovny.*/

char* createBuffer() //possibly redundant
{
    char *ptr = malloc(sizeof(char)*100); //TODO kontrola správnosti
    ptr[99] = 0;
    return ptr;
}

void appendToBuff(char *buffPtr,int* lenght,char c)
{
    printf("%i - lenght\n",*lenght);
    if(*lenght%100  == 0)
    {
        buffPtr = realloc(buffPtr,(*lenght + 100) * sizeof(char));//TODO kontrola správnosti
        buffPtr[*lenght + 99]; // přepsat s proměnnýmy hodnotami
    }
    buffPtr[*lenght] = c;
    buffPtr[*lenght +1] = 0;
    *lenght = *lenght + 1;

    //temp
    printf("%s \n", buffPtr);

}

void clearBuffer(char* buffPtr,int* lenght)
{
    *lenght = 0;
    buffPtr[100] = 0; //dořešit pro reallokované
}

int isEnd(char c)
{
    switch (c)
    {//TODO odkomentovat tuhle funkci
        case '+':
            printf("+\n"); // poslat token s +
            return 1;
        case '-':
            printf("-\n"); // poslat token s -
            return 1;
        case '=':
            printf("=\n"); // poslat token s =
            return 1;
        case '/':
            printf("/\n"); // poslat token s /
            return 1;
        case '*':
            printf("*\n"); // poslat token s *
            return 1;
        case '(':
            printf("(\n"); // poslat token s *
            return 1;
        case ')':
            printf(")\n"); // poslat token s *
            return 1;
        case '}':
            printf("{\n"); // poslat token s *
            return 1;
        case '{':
            printf("}\n"); // poslat token s *
            return 1;
        case ']':
            printf("]\n"); // poslat token s *
            return 1;
        case '[':
            printf("[\n"); // poslat token s *
            return 1;
        case '"':
            printf("[\n"); // poslat token s *
            return 1;
        case '\'':
            printf("[\n"); // poslat token s *
            return 1;
        case '<':
            printf(">\n"); // poslat token s *
            return 2;
        case '>':
            printf("<\n"); // poslat token s *
            return 2;
        default:
            return 0;
            //TODO add additional things

    }

}

int lexical()
{
    char chartype;
    char *buffer = createBuffer();
    int assistVariable = 0;
    int lenght = 0;
    printf("start\n");
    state currentState = start;
    while((chartype = getchar()) != EOF)
    {
        if(isEnd(chartype) == 1)
        {
            //TODO create token
            currentState = start;
        }
        else if(isEnd(chartype) == 2) // byl zadan '<' nebo '>'
        {
            currentState = relat;
            appendToBuff(buffer,&lenght,chartype);
        }
        else
        {
            switch(currentState)
            {
                case start://nezadano
                    if(isspace(chartype))
                    {
                        //je whitespace pokracovat jako by nebyl
                    }
                    else if(isdigit(chartype)) // zacina cislem
                    {
                        currentState = number;
                        appendToBuff(buffer,&lenght,chartype);
                    }
                    else if(isalpha(chartype) || chartype == '_') // zacina pismenem
                    {
                        currentState = word;
                        appendToBuff(buffer,&lenght,chartype);
                    }
                    else if(chartype == '!')//je string a zacina
                    {
                        currentState = stringLit;
                    }
                    else if(chartype == '/')
                    {
                        currentState = commentS;
                    }
                    else if(chartype == '"')
                    {
                        currentState = commentP;
                    }
                    break;

                case word:

                    if(isalnum(chartype) || chartype == '_')
                    {
                        //append to buffer
                        appendToBuff(buffer,&lenght,chartype);

                    }
                    else if(isEnd(chartype))
                    {
                        //TODO create token
                        // clear buffer
                        clearBuffer(buffer,&lenght);
                        currentState = start;
                    }
                    else
                    {
                        currentState = fail;
                        printLexError(); // maybe add args;
                        return 1;
                    }
                    break;

                case number:

                    if(isdigit(chartype))
                    {
                        //append to buffer
                        appendToBuff(buffer,&lenght,chartype);

                    }
                    else if(chartype == '.' || toupper(chartype) == 'E')
                    {
                        //append to buffer
                        appendToBuff(buffer,&lenght,chartype);
                        currentState = floatLit;
                    }
                    else if(isEnd(chartype))
                    {
                        //TODO create Token
                        currentState = start;
                    }
                    else
                    {
                        currentState = fail;
                        printLexError(); // maybe add args;
                        return 1;
                    }
                case stringLit:
                {
                    if(chartype == '"')
                    {
                        if(assistVariable == 0)
                        {
                            assistVariable = 1;
                        }
                            //else if() previous char was \ count as part of string                   }
                        else
                        {
                            assistVariable = 0;
                            //TODO create token
                            //TODO clear buffer
                            currentState = start;
                        }
                    }
                    else
                    {
                        //TODO append to buffer

                    }
                }
                case relat:
                    if(chartype == '=')
                    {
                        //TODO create token
                        currentState = start;
                    }

            }//State switch

        }//wtf is this ?

    }//Main cycle that reads inputs
    //TODO send EOF token
    printf("end");
    return 0;
}

void printLexError()
{
    printf("this is a temp message\n");

}

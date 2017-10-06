//
// Created by Richard Gall on 03.10.2017.
//
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include "lexical.h"
#include <stddef.h>
#include "Token.h"


/* Toto je dočasná práce, funkčnost některých funkcí je čistě náhodná, komentáře existují ale jsou velmi dobře ukryty
 * V absolutně žádném případě se nepokoušet číst,pochopit či použít jakoukoliv část této knihovny.*/

char* CreateBuffer() //possibly redundant
{
    char *ptr = malloc(sizeof(char)*100); //TODO kontrola správnosti
    ptr[99] = 0;
    return ptr;
}

char* AppendToBuff(char* buffPtr,int* lenght,char c)
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
	return buffPtr;


}

void SetLex(state* currentState,char chartype,char* buffer,int* lenght)
{
	//rozrazeni do stavu
	if(isspace(chartype))
	{
		//je whitespace pokracovat jako by nebyl
	}
	else if(isdigit(chartype)) // zacina cislem
	{
		*currentState = NUMBER;
		AppendToBuff(buffer,lenght,chartype);
	}
	else if(isalpha(chartype) || chartype == '_') // zacina pismenem
	{
		*currentState = WORD;
		AppendToBuff(buffer,lenght,chartype);
	}
	else if(chartype == '!')//je string a zacina
	{
		*currentState = STRING;
	}
	else if(chartype == '/')
	{
		*currentState = COMMENTS;
	}
	else if(chartype == '\'')
	{
		*currentState = COMMENTP;
	}
	else if(chartype == '<' || chartype == '>')
	{
		*currentState = RELAT;
		AppendToBuff(buffer,lenght,chartype);
	}
	else if(chartype == ' ')
	{
		*currentState = START;
	}

}

void MakeShortToken(int type, char c)
{
	if (type != 8)
		CreateToken();
	switch(type)
	{
		case 1 :
			SetSemicolon();
		case 2:
			SetOperator(&c);
		case 3:
			SetComma();
		case 4:
			SetLeftBracket();
		case 5:
			SetRightBracket();
		case 6:
			SetEOL();
	}
}

void ClearBuffer(char* buffPtr,int* lenght)
{
    *lenght = 0;
    buffPtr[0] = 0; //dořešit pro reallokované
}

int IsEnd(char c)
{
    switch (c)
    {//TODO odkomentovat tuhle funkci
	     //jednoznake operatory
        case '+':
            return 2;
        case '-':
            return 2;
	     case '/':
		      return 2;
	    case '\\':
		     return 2;
	    case '*':
		     return 2;
	    case '=':
            return 2;
       //viceznake operatory

	    case '<':
		    return 8;
	    case '>':
		    return 8;

		//zavorky
	    case '(':
            return 4;
        case ')':
            return 5;

		//zacatek komentare
	    case '\'':
            return 7;

		//EOL
	    case '\n':
		    return 6;

		//strednik
	    case ';':
		    return 1;

        //carka
	    case ',':
		    return 3;

		 //whitespace
	    case ' ':
		    return 9;

	    default:
            return 0;

    }

}

int Lexical() // doresit / jako operand a eof, kteremu nepredchazi whitespace
{
    char chartype;
    char *buffer = CreateBuffer();
    int assistVariable = 0;
	int prevVariable = 0;
	int endVar = 0;
    int lenght = 0;
    printf("start\n");
    state currentState = START;

 	 while((chartype = getchar()) != EOF) {
	     CreateToken();
	     endVar = IsEnd(chartype);
	     switch (currentState) {
		     case START:
			     if (endVar) { //pokud je tohle konec lexemu posle token
				     MakeShortToken(endVar, chartype);
			     }
			     //rozrazeni do stavu
			     SetLex(&currentState,chartype,buffer,&lenght);
			     break;


		     case RELAT:
			     if(chartype == '=' || (chartype == '>' && buffer[0] == '<'))
			     {
				     AppendToBuff(buffer,&lenght,chartype);
				     CreateToken();
				     SetIdentifier(buffer);
				     ClearBuffer(buffer,&lenght);
				     currentState = START;

			     }
			     else
			     {
				     CreateToken();
				     SetIdentifier(buffer);
				     SetLex(&currentState,chartype,buffer,&lenght);
				     ClearBuffer(buffer,&lenght);
				     currentState = START;
			     }
			     break;


		     case WORD:
			     if (endVar)
			     {
				     CreateToken();
				     if (endVar == 9) //v pripade ukonceni mezerou ji prida na konec stringu
				     {
					     AppendToBuff(buffer, &lenght, ' ');
				     }
				     SetIdentifier(buffer);
				     ClearBuffer(buffer, &lenght);
				     CreateToken();
				     MakeShortToken(endVar,chartype);
				     currentState = START;
				     break;
			     }
			     else if(isalnum(chartype) || chartype == '_')
			     {
				     AppendToBuff(buffer, &lenght, chartype);
			     }
			     else
			     {
				     currentState = FAIL;
			     }
			     break;

		     case NUMBER:
			     if(endVar)
			     {
				     CreateToken();
				     SetInteger(buffer);
				     ClearBuffer(buffer, &lenght);
				     MakeShortToken(endVar,chartype);
				     currentState = START;

			     }
			     else if(chartype == '.' || chartype == 'e' || chartype == 'E')
			     {
				     AppendToBuff(buffer,&lenght,chartype);
				     currentState = FLOAT;
			     }
			     else if(isdigit(chartype))
			     {
				     AppendToBuff(buffer,&lenght,chartype);
			     }
			     else
			     {
				     currentState = FAIL;
			     }
			     break;

		     case FLOAT:
			     if (endVar)
			     {
				     CreateToken();
				     SetInteger(buffer);
				     ClearBuffer(buffer, &lenght);
				     MakeShortToken(endVar,chartype);
				     currentState = START;

			     }
			     else if(isdigit(chartype))
			     {
				     AppendToBuff(buffer,&lenght,chartype);
			     }
			     else
			     {
				     currentState = FAIL;
			     }
			     break;

		     case STRING :
			     if(chartype == '"')
			     {
				     if(assistVariable == 0)
					     assistVariable++;
				     else
				     {
					     assistVariable = 0;
					     CreateToken();
					     SetString(buffer);
					     ClearBuffer(buffer,&lenght);
					     currentState = START;
				     }
			     }
			     else
			     {
				     AppendToBuff(buffer,&lenght,chartype);
			     }
			     break;

		     case COMMENTP:
			     if (chartype == '\n')
			     {
				     currentState = START;
			     }
			     break;

		     case COMMENTS:
			     if(chartype == '/')
			     {
				     if(assistVariable == 0){
					     assistVariable = 1;
				     }
				     else if(assistVariable == 1 && prevVariable == 0){
					     prevVariable = 1;
				     }
					 else if(prevVariable == 1)
					 {
						 currentState = START;
					 }
			     }
			     else if(assistVariable == 0)
			     {
				     MakeShortToken(endVar,chartype);
				     currentState = START;
			     }
			     else
			     {
				     prevVariable = 0;
			     }

			     break;


		     case FAIL:
			     printf("f \n");
			     //nevim co presne tady delat
			     break;
	     }
     }//cycle that reads input
	CreateToken();
	SetEOF();
    printf("end");
	free(buffer); //TODO zkontrolovat uspech

    return 0;
}

void PrintLexError()
{
    printf("this is a temp message\n");

}

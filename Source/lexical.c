//
// Created by Richard Gall on 03.10.2017.
//
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include "lexical.h"
#include <stddef.h>



/* Toto je dočasná práce, funkčnost některých funkcí je čistě náhodná, komentáře existují ale jsou velmi dobře ukryty.
 * V absolutně žádném případě se nepokoušet číst,pochopit či použít jakoukoliv část této knihovny.*/

char* CreateBuffer()
{
	char *ptr = malloc(sizeof(char)*100);
	if (ptr == NULL)
	{
		//TODO set allocation error
		return NULL;
	}
	ptr[99] = 0; //null terminate
	return ptr;
}

char* AppendToBuff(char* buffPtr,int* lenght,char c)
{
	//printf("%i - lenght\n",*lenght);
	if(*lenght%100  == 0) //v pripade ze by velikost bufferu prekrocila zadanou hodnotu zvetsit
	{
		buffPtr = realloc(buffPtr,(*lenght + 100) * sizeof(char));
		if(buffPtr == NULL)
		{
			return NULL; //TODO set allocation error
		}
		//buffPtr[*lenght + 100]; // přepsat s proměnnýmy hodnotami
	}
	buffPtr[*lenght] = c;
	buffPtr[*lenght +1] = 0; //null terminate string
	*lenght = *lenght + 1;

	//temp
	//printf("%s \n", buffPtr);
	return buffPtr;


}


void ClearBuffer(char* buffPtr,int* lenght)
{
	*lenght = 0;
	buffPtr[0] = 0;
}


void SetLex(state* currentState,char chartype,char* buffer,int* lenght)
{
	//rozrazeni do stavu
	if(isspace(chartype))
	{
		*currentState = START;
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
	else if(chartype == '!')//je string
	{
		*currentState = STRING;
	}
	else if(chartype == '/') // jedna se budto o operand '/' nebo o zacatek escape sekvence
	{
		*currentState = SLASH;
	}
	else if(chartype == '\'')//zacina radkova escape sekvence
	{
		*currentState = COMMENTP;
	}
	else if(chartype == '<' || chartype == '>') //relacni operator
	{
		*currentState = RELAT;
		AppendToBuff(buffer,lenght,chartype);
	}

}

void MakeShortToken(int type, char c)
{
	if (type != 8 && type != 9 && type !=7 && type !=10 && type !=11) //je <, >, /, EOF, tab, nebo mezera, v tom pripade nevytvari token
		CreateToken();
	char tmpStr[2] = {c,0};
	switch(type)
	{
		case 1 :
			SetSemicolon(); // je strednik
			break;
		case 2:

			SetOperator(tmpStr); // je jeden z operatoru +,-,/,*,\
			break;
		case 3:
			SetComma(); // je carka
			break;
		case 4:
			SetLeftBracket(); // je levá závorka
			break;
		case 5:
			SetRightBracket(); // je pravá závorka
			break;
		case 6:
			SetEOL(); // je konce radku
			break;
		default:
			break;
	}
}
int IsEnd(char c)
{
	switch (c)
	{//TODO odkomentovat tuhle funkci
		//jednoznake operatory - 2
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
		case '\t':
			return 10;

			//EOF
		case EOF :
			return 11;

			//neni konec tokenu
		default:
			return 0;
	}

}


int TEST_TOKENS(Token* token)
{
	int type = GetTokenType(token);
	switch(type)
	{
		case 0 :
			printf("Token type is:\t,\t(comma)\n");
			break;
		case 1 :
			printf("Token type is:\t;\t(semicolon)\n");
			break;
		case 2 :
			printf("Token type is:\t(\t(l bracket)\n");
			break;
		case 3 :
			printf("Token type is:\t)\t(r bracket)\n");
			break;
		case 4 :
			printf("Token type is:\tEOL\n");
			break;
		case 5 :
			printf("Token type is:\tEOF\n");
			return 1;
		case 6 :
			printf("Token type is:\toperator :\t(%c) \n", *((char*) GetTokenValue(token)));
			break;
		case 7 :
			printf("Token type is:\tkeyword :\t(%s) \n", (char*) GetTokenValue(token));
			break;
		case 8 :
			printf("Token type is:\tidentifier :\t(%s) \n", (char*) GetTokenValue(token));
			break;
		case 9 :
			printf("Token type is:\tinteger :\t(%i) \n", *((int*) GetTokenValue(token)));
			break;
		case 10 :
			printf("Token type is:\tdouble :\t(%f) \n", *((double *) GetTokenValue(token)));
			break;
		case 11 :
			printf("Token type is:\tstring :\t(%s) \n", (char*) GetTokenValue(token));
			break;
		default :
			printf("Token type is:\tUnidentified \n");
			break;
	}

	return 0;

}

int Lexical()
{
	char chartype; //aktualne zadany znak
	char *buffer = CreateBuffer();
	int escapeFlag = 0; // urcuje zda se nachazi v escape sekvenci
	int prevFlag = 0; // urcuje zda je blokovy komentar a predchozy znak byl '\''
	int endFlag = 0; //znak ukoncuje lexem
	int lenght = 0; // delka bufferu
	int eofFlag = 0; //znaci prichod EOF
	state currentState = START; //aktualni stav


	while(!eofFlag)
	{
		chartype = getchar(); //nacte znak do chartype
		endFlag = IsEnd(chartype); // zkontroluje zda je konec tokenu
		if(chartype == EOF)
		{
			eofFlag = 1;
		}
		//TODO end temp
		switch (currentState) { //stavovy automat
			case START:
				SetLex(&currentState,chartype,buffer,&lenght); //rozradi podle znaku do stavu
				if (endFlag && currentState != SLASH) { //pokud je tohle konec lexemu posle token
					MakeShortToken(endFlag, chartype);
				}
				//rozrazeni do stavu

				break;


			case RELAT: //relacni stav(<,>,<=,>= apod.)
				if(chartype == '=' || (chartype == '>' && buffer[0] == '<'))
				{
					AppendToBuff(buffer,&lenght,chartype);
					CreateToken();
					SetIdentifier(buffer);
					ClearBuffer(buffer,&lenght);
					currentState = START;
				}
				else   //TODO osetrit chybne vstupy - resit pres endFlag
				{
					CreateToken();
					SetIdentifier(buffer);
					ClearBuffer(buffer,&lenght);
					SetLex(&currentState,chartype,buffer,&lenght);
				}
				break;


			case WORD: //Je identifikator nebo keyword
				if (endFlag) //znak pro ukonceni tokenu
				{
					if (endFlag == 9 || endFlag == 10 || endFlag == 6) //v pripade ukonceni mezerou,tabem ci EOL je prida na konec stringu
					{
						AppendToBuff(buffer, &lenght,chartype);
					}
					CreateToken();
					SetIdentifier(buffer);
					ClearBuffer(buffer, &lenght);
					MakeShortToken(endFlag,chartype);
					currentState = START;
					break;
				}
				else if(isalnum(chartype) || chartype == '_') // TODO osetrit __ a ____ ....viz forum
				{
					AppendToBuff(buffer, &lenght, chartype);
				}
				else
				{
					currentState = FAIL;
				}
				break;

			case NUMBER:
				if(endFlag)
				{
					CreateToken();
					SetInteger(buffer);
					ClearBuffer(buffer, &lenght);
					MakeShortToken(endFlag, chartype);
					currentState = START;

				}
				else if(chartype == '.' || chartype == 'e' || chartype == 'E') //v pripade ze je znak e nebo . prepne se do stavu double/float
				{
					AppendToBuff(buffer,&lenght,chartype); // TODO E nebo e
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

			case FLOAT: //TODO Doplnit moznost desetinych cisel
				if (endFlag)
				{
					CreateToken();
					SetDouble(buffer);
					ClearBuffer(buffer, &lenght);
					MakeShortToken(endFlag, chartype);
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
				if(chartype == '"') // urcuje zacatek/konec stringu
				{
					if(escapeFlag == 0)
						escapeFlag = 1;
					else // v pripade konce stringu vytvori jeho token
					{
						escapeFlag = 0;
						CreateToken();
						SetString(buffer);
						ClearBuffer(buffer,&lenght);
						currentState = START;
					}
				}
				else if(escapeFlag == 1)
				{
					AppendToBuff(buffer,&lenght,chartype);
				}
				else
				{
					currentState = FAIL;
				}
				break;

			case COMMENTP:
				if (chartype == '\n') //radkovy komentar, do konce radku ignoruje vsechny vstupy
				{
					currentState = START;
					CreateToken(); //na konci posle EOL Token
					SetEOL();
				}
				break;

			case SLASH: //stav po zadani '/' - muze se jedna o operator nebo o zacatek blokoveho komentare
				if(chartype == '\'')
				{
					currentState = COMMENTS;
				}
				else
				{
					char tempStr[2] = {chartype,0};
					CreateToken();
					SetIdentifier(tempStr);
					SetLex(&currentState,chartype,buffer,&lenght);
				}
				break;

			case COMMENTS: //blokovy komentar
				if(chartype == '\'' && prevFlag == 0)
				{
					prevFlag = 1; //v pripade ze prijde prvni ze znaku ukonceni " '\ " nastavi se flag, v pripade ze bude nasledovat jiny znak
					              //bude flag zase vypnuta
				}
				else if(chartype == '/' && prevFlag == 1)
				{
					prevFlag = 0; //v pripade ukonceni komentare se prepne do stavu start
					currentState = START;
				}
				else
				{
					prevFlag = 0;
				}
				break;

			//TODO Upravit
			case FAIL: //chybny stav
				if (endFlag)
				{
					ClearBuffer(buffer,&lenght);
					CreateToken();
					currentState = START;
				}
				//return 1;
				//nevim co presne tady delat
				break;
		}
	}//cycle that reads input
	CreateToken();
	SetEOF();
	free(buffer);
	return 0;
}

void PrintLexError() //This is useless
{
	printf("this is a temp message\n");

}

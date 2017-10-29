#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdbool.h>
#include "Lexical.h"
#include "Token.h"
#include "CompilationErrors.h"

#define CHUNK 100

/* Toto je dočasná práce, funkčnost některých funkcí je čistě náhodná, komentáře existují ale jsou velmi dobře ukryty.
 * V absolutně žádném případě se nepokoušet číst,pochopit či použít jakoukoliv část této knihovny.*/


/*
 * @brief enumerace stavu
 */
typedef enum State {
	START,
	WORD,
	NUMBER,
	COMMENTP,
	COMMENTS,
	FLOAT,
	STRING,
	RELAT,
	SLASH,
	FAIL,
} State;

typedef enum Type {
	LEX_NO_TYPE = 0,
	LEX_SEMICOLON = 1,
	LEX_SHORT_OP = 2,
	LEX_COMMA = 3,
	LEX_L_BRACKET = 4,
	LEX_R_BRACKET = 5,
	LEX_EOL = 6,
	LEX_COMMENT = 7,
	LEX_LONG_OP = 8,
	LEX_SPACE = 9,
	LEX_TAB = 10,
	LEX_EOF = 11
} Type;


typedef struct Buffer {
	char* data;
	size_t lenght;
	size_t index;
} Buffer;
static Buffer g_Buffer;


/*
 * * @brief Prida znak na konec bufferu
 *
 * Vlozi znak do bufferu, pri prekroceni velikosti bufferu se
 * automaticky zvetsi velikost
 */
void AppendToBuff(int c) {
	if (g_Buffer.index == g_Buffer.lenght) {
		char* tmp;
		g_Buffer.lenght += CHUNK;

		//Buffer o 1 vetsi nez velikost kvuli ukoncovaci nule
		if ((tmp = realloc(g_Buffer.data, sizeof(char) * (g_Buffer.lenght + 1))) == NULL) {
			FatalError(ER_FATAL_INTERNAL);
		}
		g_Buffer.data = tmp;
	}
	g_Buffer.data[g_Buffer.index++] = (char) c;
	g_Buffer.data[g_Buffer.index] = 0;
}


/*
 * @brief Resetuje pocitadlo bufferu
 */
void ClearBuffer() {
	g_Buffer.index = 0;
}


/*
 * @brief V pripade ukonceni stavu rozhodne do jakeho nasledujiciho stavu ma prejit
 * @param currentState ukazatel na aktualni stav, za ucelem zmeny
 * @param firstChar rozrazovaci znak, bude pripojen na zacatek bufferu
 */
void SetLex(State* currentState, int firstChar) {
	//rozrazeni do stavu
	if (isspace(firstChar)) {
		*currentState = START;
		//je whitespace pokracovat jako by nebyl
	}
	else if (isdigit(firstChar)) {
		// zacina cislem
		*currentState = NUMBER;
		AppendToBuff(firstChar);
	}
	else if (firstChar == '<' || firstChar == '>') {
		//relacni operator
		*currentState = RELAT;
		AppendToBuff(firstChar);
	}
	else if (isalpha(firstChar) || firstChar == '_') {
		// zacina pismenem
		*currentState = WORD;
		AppendToBuff(firstChar);
	}
	else if (firstChar == '!') {
		//je string
		*currentState = STRING;
	}
	else if (firstChar == '/') {
		// jedna se budto o operand '/' nebo o zacatek escape sekvence
		*currentState = SLASH;
	}
	else if (firstChar == '\'') {
		//zacina radkova escape sekvence
		*currentState = COMMENTP;
	}
	else {
		*currentState = START;
	}

}


/*
 * @brief Vytvori jednoznakovy token(';',',',+,-,(,*...)
 * @param type typ tokenu
 * @param firstChar v pripade operatoru, posle o jaky se jedna
 * */
void MakeShortToken(Type tokenType, int firstChar) {
	//Musi byt <, >, /, EOF, tab, nebo mezera
	if (tokenType == 0 || tokenType > LEX_EOL)
		return;

	char tmpStr[2] = {(char) firstChar, 0};
	CreateToken();
	switch (tokenType) {
		case LEX_SEMICOLON:
			SetSemicolon(); // je strednik
			return;
		case LEX_SHORT_OP:
			SetOperator(tmpStr); // je jeden z operatoru +,-,/,*,'\'
			return;
		case LEX_COMMA:
			SetComma(); // je carka
			return;
		case LEX_L_BRACKET:
			SetLeftBracket(); // je levá závorka
			return;
		case LEX_R_BRACKET:
			SetRightBracket(); // je pravá závorka
			return;
		case LEX_EOL:
			SetEOL(); // je konce radku
			return;

		default:
			return;
	}
}


/*
 * @brief zkontroluje ci znak ukoncuje lexem
 * @param c aktualni znak
 */
Type IsEnd(int currentChar) {
	switch (currentChar) {
		case ';':
			return LEX_SEMICOLON;
		case '+':
		case '-':
		case '/':
		case '\\':
		case '*':
		case '=':
			return LEX_SHORT_OP; //jednoznake operatory - 2
		case ',':
			return LEX_COMMA;
		case '(':
			return LEX_L_BRACKET;
		case ')':
			return LEX_R_BRACKET;
		case '\n':
			return LEX_EOL; //Konec radku
		case '\'':
			return LEX_COMMENT; //zacatek komentare
		case '<':
		case '>':
			return LEX_LONG_OP; //viceznake operatory
		case ' ':
			return LEX_SPACE;
		case '\t':
			return LEX_TAB;
		case EOF :
			return LEX_EOF; //Konec vstupu

		default:
			return LEX_NO_TYPE; //Neni ukoncujici znak
	}
}


bool Lexical() {
	State currentState = START; //aktualni stav
	bool commentFlag = false; // urcuje zda je blokovy komentar a predchozi znak byl '\''
	bool escapeFlag = false; // urcuje zda se nachazi v escape sekvenci
	bool eofFlag = false; //znaci prichod EOF
	Type endFlag; //znak ukoncuje lexem
	int currentChar; //aktualne zadany znak



	while (!eofFlag) {
		currentChar = getchar(); //nacte znak do currentChar
		endFlag = IsEnd(currentChar); // zkontroluje zda je konec tokenu
		if (currentChar == EOF) {
			eofFlag = true;
		}
		//TODO end temp
		switch (currentState) { //stavovy automat
			case START:
				SetLex(&currentState, currentChar); //rozradi podle znaku do stavu
				if (endFlag && currentState != SLASH) { //pokud je tohle konec lexemu posle token
					MakeShortToken(endFlag, currentChar);
				}
				//rozrazeni do stavu

				break;


			case RELAT: //relacni stav(<,>,<=,>= apod.)
				if (currentChar == '=' || (currentChar == '>' && g_Buffer.data[0] == '<')) {
					AppendToBuff(currentChar);
					CreateToken();
					SetOperator(g_Buffer.data);
					ClearBuffer();
					currentState = START;
				}
				else   //TODO osetrit chybne vstupy - resit pres endFlag
				{
					CreateToken();
					SetOperator(g_Buffer.data);
					ClearBuffer();
					SetLex(&currentState, currentChar);
				}
				break;


			case WORD: //Je identifikator nebo keyword
				if (endFlag) //znak pro ukonceni tokenu
				{
					if (endFlag == 9 || endFlag == 10 ||
							endFlag == 6) //v pripade ukonceni mezerou,tabem ci EOL je prida na konec stringu
					{
						AppendToBuff(currentChar);
					}
					CreateToken();
					SetIdentifier(g_Buffer.data);
					ClearBuffer();
					MakeShortToken(endFlag, currentChar);
					currentState = START;
					break;
				}
				else if (isalnum(currentChar) || currentChar == '_') // TODO osetrit __ a ____ ....viz forum
				{
					AppendToBuff(currentChar);
				}
				else {
					currentState = FAIL;
				}
				break;

			case NUMBER:
				if (endFlag) {
					CreateToken();
					SetInteger(g_Buffer.data);
					ClearBuffer();
					MakeShortToken(endFlag, currentChar);
					currentState = START;

				}
				else if (currentChar == '.' || currentChar == 'e' ||
								 currentChar == 'E') //v pripade ze je znak e nebo . prepne se do stavu double/float
				{
					AppendToBuff(currentChar); // TODO E nebo e
					currentState = FLOAT;
				}
				else if (isdigit(currentChar)) {
					AppendToBuff(currentChar);
				}
				else {
					currentState = FAIL;
				}
				break;

			case FLOAT: //TODO Doplnit moznost desetinych cisel
				if (endFlag) {
					CreateToken();
					SetDouble(g_Buffer.data);
					ClearBuffer();
					MakeShortToken(endFlag, currentChar);
					currentState = START;

				}
				else if (isdigit(currentChar)) {
					AppendToBuff(currentChar);
				}
				else {
					currentState = FAIL;
				}
				break;

			case STRING :
				if (currentChar == '"') // urcuje zacatek/konec stringu
				{
					if (!escapeFlag)
						escapeFlag = true;
					else // v pripade konce stringu vytvori jeho token
					{
						escapeFlag = false;
						CreateToken();
						SetString(g_Buffer.data);
						ClearBuffer();
						currentState = START;
					}
				}
				else if (escapeFlag) {
					AppendToBuff(currentChar);
				}
				else {
					currentState = FAIL;
				}
				break;

			case COMMENTP:
				//radkovy komentar, do konce radku ignoruje vsechny vstupy
				if (currentChar == '\n') {
					currentState = START;
					CreateToken(); //na konci posle EOL Token
					SetEOL();
				}
				break;

			case SLASH: //stav po zadani '/' - muze se jedna o operator nebo o zacatek blokoveho komentare
				if (currentChar == '\'') {
					currentState = COMMENTS;
				}
				else {
					char tempStr[2] = {(char) currentChar, 0};
					CreateToken();
					SetIdentifier(tempStr);
					SetLex(&currentState, currentChar);
				}
				break;

			case COMMENTS: //blokovy komentar
				if (currentChar == '\'' && !commentFlag) {
					commentFlag = true; //v pripade ze prijde prvni ze znaku ukonceni " '\ " nastavi se flag, v pripade ze bude nasledovat jiny znak
					//bude flag zase vypnuta
				}
				else if (currentChar == '/' && commentFlag) {
					commentFlag = false; //v pripade ukonceni komentare se prepne do stavu start
					currentState = START;
				}
				else {
					if (currentChar == '\n') {
						CreateToken();
						SetEOL();
					}
					commentFlag = false;
				}
				break;

				//TODO Upravit
			case FAIL: //chybny stav
				if (endFlag) {
					ClearBuffer();
					CreateToken();
					currentState = START;
				}
				//return false;
				//nevim co presne tady delat
				break;
		}
	}//Loop that reads input
	CreateToken();
	SetEOF();
	LexCleanup();
	return true;
}


void LexCleanup() {
	if (g_Buffer.data) {
		free(g_Buffer.data);
		g_Buffer.data = NULL;
	}
	g_Buffer.index = g_Buffer.lenght = 0;
}


/*
 * @brief testovaci funkce, vypise na stdout vsechny vyrvorene tokeny
 *
 */
int TEST_TOKENS(Token* token) {
	int type = GetTokenType(token);
	switch (type) {
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
			printf("Token type is:\tdouble :\t(%f) \n", *((double*) GetTokenValue(token)));
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

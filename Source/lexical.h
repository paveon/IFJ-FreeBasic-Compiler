//
// Created by Richard Gall on 03.10.2017.
//
#ifndef LEXICAL_H
#define LEXICAL_H
#include "Token.h"


//TODO napsat rozsahlejsi komentare
/*
 * @brief testovaci funkce, vypise na stdout vsechny vyrvorene tokeny
 *
 */
int TEST_TOKENS(Token* token);

/*
 * @brief enumerace stavu
 *
 */
typedef enum {
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
}state;

/*
 * Vytvori buffer, allokuje 100 charu.
 * @brief Vytvori buffer.
 *
 * @return pointer na buffer
 * */
char* CreateBuffer();

/*
 * Prida znak na konec bufferu, v pripade ze bude prekrocen limit, dojde k naviseni
 *
 * @brief Prida znak na konec bufferu
 *
 * @return vrati pointer na buffer
 * */
char* AppendToBuff(char *buffPtr,int* lenght,char c);

/*
 * @brief Vycisti buffer
 *
 * @param buffPtr Ukazatel na buffer
 * @param lenght Ukazatel na int lenght
 *
 */
void ClearBuffer(char* buffPtr,int* lenght);


/*
 * @brief Vytvori jednoznakovy token(';',',',+,-,(,*...)
 *
 * @param type typ tokenu
 * @param c v pripade operatoru, posle o jaky se jedna
 * */
void MakeShortToken(int type, char c);


/*
 * @brief V pripade ukonceni stavu rozhodne do jakeho nasledujiciho stavu ma prejit
 *
 * @param currentState ukazatel na aktualni stav, za ucelem zmeny
 * @param chartype posledni precteny znak, bude pripojen na zacatek bufferu
 * @param buffer ukazatel na buffer
 * @param lenght ukazatel na delku bufferu
 */
void SetLex(state* currentState,char chartype,char* buffer,int* lenght);


/*
 * @brief zkontroluje ci znak ukoncuje lexem
 *
 * @param c aktualni znak
 */
int IsEnd(char c);


/*
 * Primarni funkce pro lexikalni analyzu. Precte cely vstup na stdin a vytvori zretezeny seznam tokenu.
 *
 * @brief primarni funkce, vytvori seznam zretezenych tokenu, pracuje dokud neprijde EOF
 *
 * @return v pripade uspechu vrati 0 a v pripade chyby vraci 1
 */
int Lexical();


/*
 * probably obsolete
 */
void PrintLexError();

#endif //LEXICAL_H

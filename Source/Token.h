#ifndef FREEBASIC_COMPILER_TOKEN_H
#define FREEBASIC_COMPILER_TOKEN_H

/*
 * Nasledujici funkce by mely byt const-korektni.
 * Pri predavani hodnot si zkontrolujte zda je
 * formalni parametr const. Pokud neni, dochazi uvnitr
 * funkce k modifikaci dat na ktere ukazatel odkazuje.
 * Davejte si tedy predevsim pozor na funkci SetIdentifier, ktere NESMITE
 * predavat retezcove literaly (dojde k SEGFAULT)
 *
 * Set funkce automaticky modifikuji posledne vytvoreny token a to pouze jednou.
 * Nepodporuji tedy pozdejsi modifikaci. Funkce nic nevraci, protoze se predpoklada
 * ze dostanou korektni vstup a pripadne nevalidni lexemy osetri primo lexikalni analyzator.
 * Pri nevalidnim vstupu zustane typ tokenu UNDEFINED a ukazatel na data NULL
 *
 * Pri selhani alokace je program ukoncen pomoci exit(..) funkce a je vycistena pamet.
 */


/* Pouzivaji se pro indexaci, proto natvrdo zadane hodnoty */
typedef enum TokenType {
	TOKEN_COMMA = 0,
	TOKEN_SEMICOLON = 1,
	TOKEN_L_BRACKET = 2,
	TOKEN_R_BRACKET = 3,
	TOKEN_EOL = 4,
	TOKEN_EOF,
	TOKEN_OPERATOR,
	TOKEN_KEYWORD,
	TOKEN_IDENTIFIER,
	TOKEN_INTEGER,
	TOKEN_DOUBLE,
	TOKEN_STRING,
	TOKEN_UNDEFINED
} TokenType;
typedef struct Token Token;


/* Vraci ukazatel na dalsi nacteny token, VYHRAZENO PRO SYNTAKTICKY ANALYZATOR */
Token* GetNextToken(void);


/* Vrati nacteny token zpet. Ziskame stejny token pri pristim volani funkce GetNextToken */
void ReturnToken(void);


/* Vytvori novy token typu TOKEN_UNDEFINED. Modifikace tokenu se provadi pomoci Set funkci */
void CreateToken(void);


/* Vraci typ tokenu, pouzivejte enumeraci TokenType pro porovnavani */
TokenType GetTokenType(const Token*);


/* Vraci ukazatel na konstatni data tokenu. Nastavovat data tokenu
 * je povoleno pouze skrze odpovidajici funkce.
 * Pri potrebe menit data si vytvorte lokalni kopii dat
 */
const void* GetTokenValue(const Token*);


/* Vraci primo hodnotu, pokud token odkazuje na integer. Jinak vraci -1 */
int GetTokenInt(const Token*);


/* Vraci primo hodnotu, pokud token odkazuje na double. Jinak vraci NAN */
double GetTokenDouble(const Token*);


/* Set funkce modifikuji data tokenu maximalne 1, pote je potreba vytvorit novy token */

/* Pouzivat pokud je lexem operator */
void SetOperator(const char* operator);


/* Pouzivat pokud je lexem carka (v zakladni verzi se nemuze vyskytovat ve vyrazech) */
void SetComma(void);


/* Pouzivat pokud je lexem strednik */
void SetSemicolon(void);


/* Pouzivat pokud je lexem leva zavorka */
void SetLeftBracket(void);


/* Pouzivat pokud je lexem prava zavorka */
void SetRightBracket(void);


/* Pouzivat pokud je lexem identifikator / klicove slovo.
 * Je potreba posilat s ukoncujicim whitespace charakterem, pokud nejaky je
 * kvuli rozliseni, zda se muze jednat o volani funkce.
 * Pokud je ukoncen pomoci noveho radku, je pote potreba zaslat novy radek jakozto
 * dalsi token typu EOL.
 */
void SetIdentifier(char* symbol);


/* Pouzivat pokud je lexem celociselny literal, ocekava se korektni vstup */
void SetInteger(const char* number);


/* Pouzivat pokud je lexem desetinny literal, ocekava se korektni vstup */
void SetDouble(const char* number);


/* Pouzivat pokud je lexem retezcovy literal */
void SetString(const char* string);


/* Pouzivat pokud je lexem konec radku */
void SetEOL(void);


/* Pouzivat pokud lexikalni analyzator narazi na konec vstupu */
void SetEOF(void);


/* Nepouzivat, interni funkce pro vycisteni pameti */
void TokenCleanup(void);

#endif //FREEBASIC_COMPILER_TOKEN_H

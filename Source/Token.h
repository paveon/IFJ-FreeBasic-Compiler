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
 * Set funkce lze pouzit pouze na tokeny s typem TOKEN_UNDEFINED (novy token),
 * nepodporuji tedy pozdejsi modifikaci. Funkce nic nevraci, protoze se predpoklada
 * ze dostanou korektni vstup a pripadne nevalidni lexemy osetri primo lexikalni analyzator.
 * Pri nevalidnim vstupu zustane typ tokenu UNDEFINED a ukazatel NULL
 *
 * Pri selhani alokace je program ukoncen pomoci exit(..) funkce a je vycistena pamet.
 */

typedef enum TokenType {
	TOKEN_UNDEFINED,
	TOKEN_OPERATOR,
	TOKEN_KEYWORD,
	TOKEN_IDENTIFIER,
	TOKEN_INTEGER,
	TOKEN_DOUBLE,
	TOKEN_STRING,
	TOKEN_EOL,
	TOKEN_EOF
} TokenType;
typedef struct Token Token;


/* Vraci ukazatel na dalsi nacteny token, VYHRAZENO PRO SYNTAKTICKY ANALYZATOR */
Token* GetNextToken(void);


/* Vraci ukazatel na inicializovany token, defaultni typ je TOKEN_UNDEFINED */
Token* CreateToken(void);


/* Vraci typ tokenu, pouzivejte enumeraci TokenType pro porovnavani */
TokenType GetType(const Token*);


/* Vraci ukazatel na konstatni data tokenu. Nastavovat data tokenu
 * je povoleno pouze skrze odpovidajici funkce.
 * Pri potrebe menit data si vytvorte lokalni kopii dat
 */
const void* GetValue(const Token*);


/* Vraci primo hodnotu, pokud token odkazuje na integer. Jinak vraci -1 */
int GetInt(const Token*);


/* Vraci prio hodnotu, pokud token odkazuje na double. Jinak vraci NAN */
double GetDouble(const Token*);


/* Pouzivat pokud je lexem operator */
void SetOperator(Token* token, const char* operator);


/* Pouzivat pokud je lexem identifikator / klicove slovo */
void SetIdentifier(Token* token, char* symbol);


/* Pouzivat pokud je lexem celociselny literal, ocekava se korektni vstup */
void SetInteger(Token* token, const char* number);


/* Pouzivat pokud je lexem desetinny literal, ocekava se korektni vstup */
void SetDouble(Token* token, const char* number);


/* Pouzivat pokud je lexem retezcovy literal */
void SetString(Token* token, const char* string);


/* Pouzivat pokud je lexem konec radku */
void SetEOL(Token* token);


/* Pouzivat pokud lexikalni analyzator narazi na konec vstupu */
void SetEOF(Token* token);


/* Nepouzivat, interni funkce pro vycisteni pameti */
void TokenCleanup(void);

#endif //FREEBASIC_COMPILER_TOKEN_H

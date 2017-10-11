#ifndef FREEBASIC_COMPILER_SYMTABLE_H
#define FREEBASIC_COMPILER_SYMTABLE_H

#include <stdbool.h>
#include "Symbol.h"

#define MAX_ARGS 20

typedef enum Scope {
	SCOPE_LOCAL,
	SCOPE_GLOBAL
} Scope;

typedef struct Identifier {
	//TODO: popremyslet, co vsechno bude semanticky analyzator potrebovat
	//TODO: dynamicky spravovat pamet pro signatury
	bool declaration;
	char signature[MAX_ARGS];
	size_t argIndex;
	const char* name; //nazev identifikatoru
	Scope scope; //privatni promenna - nemenit rucne
} Identifier;


/* Ukazka prace s BeginSubScope a EndSubScope
 *
 * Sample program:
 *
 * main(){
 *    float x = 1 / 0;
 *
 *    //BeginSubScope
 *    for(...){
 *       double b = 0;
 *
 *       //BeginSubScope
 *       if(...){
 *          int a = 5;
 *       }
 *       //EndSubScope
 *    }
 *    //EndSubScope
 *
 *    //BeginSubScope
 *    if(...){
 *       int c = 42;
 *    }
 *    //EndSubScope
 * }
 * //EndScope
 *
 * Poznamka: hlavni scope kazde funkce (vcetne main)
 * existuje defaultne, neni potreba volat BeginSubScope
 */


/*
 * Vytvori novy podblok v aktualnim bloku a zanori se do nizsi urovne.
 * Zahrnuje vytvoreni nove lokalni tabulky.
 * Pouzivat pro vytvoreni noveho lokalniho scopu napr. v ridicich konstrukcich.
 */
void BeginSubScope(void);


/*
 * Ukonci aktualni podblok a vynori se o uroven vys. Promenne deklarovane v ukoncenem
 * podbloku se stavaji nedostupnymi (out of scope) a nejsou soucasti vyhledavani symbolu.
 */
void EndSubScope(void);


/*
 * Slouzi k ukonceni hlavniho bloku kodu funkce. Vymaze stare lokalni tabulky
 * vcetne vsech lokalnich symbolu, globalni symboly se nerusi.
 * Pro ukonceni main funkce slouzi spise funkce TableCleanup.
 */
void EndScope(void);


/*
 * Vytvori a vlozi novy symbol do globalni tabulky symbolu
 * (pouzivat pro funkce a globalni promenne).
 * Vraci ukazatel na novy symbol. Pokud symbol jiz existuje, vraci NULL.
 */
Identifier* InsertGlobalID(const char* name);


/*
 * Vytvori a vlozi novy symbol do tabulky prislusejici aktualnimu bloku.
 * Vraci ukazatel na novy symbol. Pokud symbol jiz existuje, vraci NULL.
 */
Identifier* InsertLocalID(const char* name);


/*
 * Hleda symbol v aktualnim bloku, nadrazenych blocich a nakonec i v globalnim scopu.
 * Vraci ukazatel na prislusny symbol, pokud jej nalezne, jinak NULL.
 */
Identifier* LookupID(const char* name);


/*
 * Hleda symbol pouze v globalnim scopu.
 * Vraci ukazatel na prislusny symbol, pokud jej nalezne, jinak NULL.
 */
Identifier* LookupGlobalID(const char* name);


bool SetSignature(Identifier* id, Terminal type, bool returnType);

bool CompareSignature(Identifier* id, Terminal type, size_t index);



/* Interni funkce pro uvolneni alokovane pameti souvisejici s tabulkami symbolu. !!NEPOUZIVAT!! */
void TableCleanup(bool allNodes);

#endif //FREEBASIC_COMPILER_SYMTABLE_H

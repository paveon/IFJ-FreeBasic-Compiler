#ifndef FREEBASIC_COMPILER_SYMTABLE_H
#define FREEBASIC_COMPILER_SYMTABLE_H

#include <stdbool.h>
#include "Symbol.h"

#define MAX_ARGS 20

typedef struct Variable {
	//TODO: popremyslet, co vsechno bude semanticky analyzator potrebovat
	//TODO: dynamicky spravovat pamet pro signatury
	const char* name; //Nazev promenne
	Terminal type; //Typ promenne
	bool staticVariable;
	size_t codeLine; //Na jakem radku byla promenna deklarovana / definovana
} Variable;


typedef struct Function {
	const char* name; //Nazev funkce
	Terminal returnType; //Navratovy typ funkce
	Terminal* parameters; //Pro ulozeni signatur funkci pomoci textu
	size_t argCount; //Pocitadlo aktualniho poctu parametru
	size_t codeLine; //Na jakem radku byla funkce deklarovana / definovana
	bool declaration; //Zda se jedna o deklaraci funkce
} Function;


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


/* Vytvori novy podblok v aktualnim bloku a zanori se do nizsi urovne.
 * Zahrnuje vytvoreni nove lokalni tabulky.
 * Pouzivat pro vytvoreni noveho lokalniho scopu napr. v ridicich konstrukcich.
 */
void BeginSubScope(void);


/* Ukonci aktualni podblok a vynori se o uroven vys. Promenne deklarovane v ukoncenem
 * podbloku se stavaji nedostupnymi (out of scope) a nejsou soucasti vyhledavani symbolu.
 */
void EndSubScope(void);


/* Slouzi k ukonceni hlavniho bloku kodu funkce. Vymaze stare lokalni tabulky
 * vcetne vsech lokalnich symbolu, globalni symboly se nerusi.
 * Pro ukonceni main funkce slouzi spise funkce TableCleanup.
 */
void EndScope(void);


/* Vytvori novou funkci v globalni tabulce identifikatoru.
 * Vraci ukazatel na novou funkci, nebo NULL pokud jiz existuje.
 */
Function* InsertFunction(const char* name, bool declaration, size_t line);


/* Vytvori novou promennou v aktivnim bloku kodu.
 * Pokud se ma jednat o globalni promennou, staci specifikovat pomoci
 * parametru 'global'.
 * Vraci ukazatel na nove vytvorenou promennou, nebo NULL pokud jiz existuje.
 */
Variable* InsertVariable(const char* name, bool global, size_t line);


/* Hleda symbol v aktualnim bloku a pote i nadrazenych blocich, pokud je argument
 * 'onlyCurrentScope' false.
 * Pokud promennou nenalezne, prohleda i globalni tabulku.
 * Vraci ukazatel na prislusny symbol, pokud jej nalezne, jinak NULL.
 */
Variable* LookupVariable(const char* name, bool onlyCurrentScope, bool allowGlobals);


/* Hleda identifikator funkce v globalni tabulce.
 * Vraci ukazatel na identifikator funkce pokud jej nalezne, jinak NULL.
 */
Function* LookupFunction(const char* name);


void AddParameter(Function* function, Terminal parameter);


/* Funkce pro vycisteni pameti spojene s tabulkami identifikatoru.
 * Provadi korektni reinicializaci, takze je mozne pozdeji znovu
 * automaticky alokovat pamet
 */
void TableCleanup();

#endif //FREEBASIC_COMPILER_SYMTABLE_H

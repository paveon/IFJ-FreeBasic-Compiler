#ifndef FREEBASIC_COMPILER_STACK_H
#define FREEBASIC_COMPILER_STACK_H

#include <stdbool.h>
#include "Token.h"
#include "Symbol.h"

/* Header poskytuje jednoduche rozhrani pro praci se zasobnikem
 * Snazi se nezatezovat uzivatele s implementacnimi detaily a
 * zajistuje korektni alokaci / dealokaci pameti
 */

typedef struct Stack Stack;

/* Vraci typ symbolu na vrcholu zasobniku.
 * Pokud je ukazatel NULL, vraci SYMBOL_UNDEFINED
 * Pokud je zasobnik prazdny, vraci SYMBOL_BOTTOM (signalizace prazdneho zasobniku)
 */
SymbolType GetSymbolType(const Stack* stack);


/* Vraci terminal z vrcholu zasobniku, programator zodpovida za kontrolu,
 * ze se na vrcholu opravdu nachazi terminal. Pokud je ukazatel NULL,
 * vraci T_UNDEFINED
 */
Terminal GetTopT(const Stack* stack);


/* Vraci neterminal z vrcholu zasobniku, programator zodpovida za kontrolu,
 * ze se na vrcholu opravdu nachazi neterminal. Pokud je ukazatel NULL,
 * vraci NT_UNDEFINED
 */
NTerminal GetTopNT(const Stack* stack);


/* Vraci symbol na vrcholu zasobniku, pokud je ukazatel NULL, vraci NULL */
const Symbol* GetTop(const Stack* stack);


/* Odstrani libovolny symbol z vrcholu zasobniku.
 * Pokud je ukazatel NULL, nebo je zasobnik prazdny, nestane se nic
 */
void PopSymbol(Stack* stack);


/* Provede derivaci neterminalu na vrcholu zasobniku
 * podle odpovidajiciho pravidla vybraneho na zaklade predict mnoziny.
 * Vraci "false" POUZE pokud neexistuje derivacni pravidlo v LL tabulce,
 * jinak vraci "true" (kvuli rozpoznani derivacni chyby).
 * Vzdy provede POP neterminalu na vrcholu zasobniku.
 */
bool ExpandTop(Stack* stack, const Token* token);


/* Porovna terminal na vrcholu zasobniku s poskytnutym tokenem.
 * Vraci "true" pri shode, jinak "false".
 * Pokud na vrcholu zasobniku neni terminal, vraci "false".
 * <<!!>>NEPROVADI<<!!>> POP neterminalu na vrcholu zasobniku,
 * tato povinnost je na programatorovi.
 */
bool CompareTop(const Stack* stack, const Token* token);


/* Vlozi terminal na vrchol zasobniku.
 * Pokud je ukazatel NULL, neprovede nic
 */
void PushT(Stack* stack, Terminal terminal);


/* Vlozi neterminal na vrchol zasobniku
 * Pokud je ukazatel NULL, neprovede nic
 */
void PushNT(Stack* stack, NTerminal nTerminal);


/* Poskytne prazdny zasobnik k pouziti */
Stack* GetStack(void);


/* Uvolni zasobnik, volat po ukonceni prace se zasobnikem.
 * Je zakazano dale pracovat s ukazatelem na zasobnik,
 * zasobnik jiz nepatri programatorovi (je mozne, ze je vyuzivan nekym jinym)
 */
void ReleaseStack(Stack* stack);


/* Interni funkce pro cisteni pameti  <!!>NEPOUZIVAT<!!> */
void StackCleanup(void);


#endif //FREEBASIC_COMPILER_STACK_H

#ifndef FREEBASIC_COMPILER_STACK_H
#define FREEBASIC_COMPILER_STACK_H

#include <stdbool.h>
#include "Token.h"
#include "LLtable.h"


typedef struct Stack Stack;

SymbolType TopSymbolType(const Stack* stack);

void PopItem(Stack* stack);

bool ExpandTop(Stack* stack, Token* token);

void PushT(Stack* stack, Terminal terminal);

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

#include <stdlib.h>
#include <stdbool.h>
#include "CompilationErrors.h"
#include "Stack.h"
#include "LLtable.h"

#define CHUNK 50

struct Stack {
	Symbol* top;
	bool inUse;
};

typedef struct StackStash {
	Stack* array;
	size_t size;
} StackStash;
static StackStash g_Stacks;

typedef struct SymbolStash {
	Symbol* allocated; //Pole vsech alokovanych symbolu
	Symbol** unused; //Pole ukazatelu na nepouzite symboly
	size_t size; //Velikost pole vsech symbolu
	size_t used; //Index do pole nepouzitych symbolu
} SymbolStash;
static SymbolStash g_Symbols;

/* Deklarace privatnich funkci */
Symbol* CreateSymbol(void);

void ReleaseSymbol(Symbol* symbol);


void PushT(Stack* stack, Terminal terminal) {
	if (!stack) { return; }
	Symbol* newSymbol = CreateSymbol();
	newSymbol->type = SYMBOL_TERMINAL;
	newSymbol->data.terminal = terminal;
	newSymbol->down = stack->top;
	if (stack->top) {
		stack->top->up = newSymbol;
	}
	stack->top = newSymbol;
}


void PushNT(Stack* stack, NTerminal nTerminal) {
	if (!stack) { return; }
	Symbol* newSymbol = CreateSymbol();
	newSymbol->type = SYMBOL_NONTERMINAL;
	newSymbol->data.nonTerminal = nTerminal;
	newSymbol->down = stack->top;
	if (stack->top) {
		stack->top->up = newSymbol;
	}
	stack->top = newSymbol;
}


/* Vytvori nove symboly, pouze pokud jiz nejsou k dispozici zadne
 * volne symboly. Nasledujici zpusob se snazi minimalizovat celkovy
 * pocet alokaci pomoci predalokovani vetsiho mnozstvi symbolu
 * a monitorovani dostupnych symbolu. Take umoznuje recyklaci symbolu
 */
Symbol* CreateSymbol(void) {
	if (g_Symbols.used == g_Symbols.size) {
		Symbol* array;
		Symbol** available;
		g_Symbols.size += CHUNK;
		if ((array = realloc(g_Symbols.allocated, sizeof(Symbol) * g_Symbols.size)) == NULL ||
		    (available = realloc(g_Symbols.unused, sizeof(Symbol*) * g_Symbols.size)) == NULL) {
			FatalError(ER_FATAL_INTERNAL);
		}
		g_Symbols.allocated = array;
		g_Symbols.unused = available;

		//Zkopirujeme ukazatele na nove symboly do pole dostupnych symbolu
		for (size_t i = g_Symbols.used; i < g_Symbols.size; i++) {
			g_Symbols.unused[i] = &g_Symbols.allocated[i];
			g_Symbols.unused[i]->used = false;
		}
	}

	//Pouzijeme nasledujici volny symbol a inkrementujeme pocitadlo pouzitych symbolu
	Symbol* freeSymbol = g_Symbols.unused[g_Symbols.used];
	g_Symbols.unused[g_Symbols.used++] = NULL;

	//Pred pouzitim (re)inicializujeme hodnoty
	freeSymbol->used = true;
	freeSymbol->reduceEnd = false;
	freeSymbol->down = freeSymbol->up = NULL;
	return freeSymbol;
}


/* interni funkce - nekontroluje se ukazatel */
void ReleaseSymbol(Symbol* symbol) {
	if (symbol->used == false || g_Symbols.used == 0) {
		return; //symbol nelze uvolnit (jiz byl uvolnen / vsechny symboly jsou nepouzite)
	}
	symbol->used = false;
	g_Symbols.unused[--g_Symbols.used] = symbol;
}


Stack* GetStack(void) {
	//Pokusime se najit nepouzity stack
	for (size_t i = 0; i < g_Stacks.size; i++) {
		if (g_Stacks.array[i].inUse == false) {
			return &g_Stacks.array[i]; //Volny stack existuje
		}
	}

	//Vytvorime novy stack
	Stack* tmp;
	g_Stacks.size++;
	if ((tmp = realloc(g_Stacks.array, sizeof(Stack) * g_Stacks.size)) == NULL) {
		FatalError(ER_FATAL_INTERNAL);
	}
	g_Stacks.array = tmp;
	tmp = &g_Stacks.array[g_Stacks.size - 1];
	tmp->top = NULL;
	tmp->inUse = true;
	return tmp;
}


/* Uvolni symboly a zasobnik pro opetovne pouziti (neprovadi dealokaci) */
void ReleaseStack(Stack* stack) {
	Symbol* current = stack->top;
	while (current) {
		stack->top = current->down;
		ReleaseSymbol(current);
		current = stack->top;
	}
	stack->inUse = false;
}

SymbolType TopSymbolType(const Stack* stack) {
	if (!stack) { return SYMBOL_UNDEFINED; }
	else if (!stack->top) { return SYMBOL_BOTTOM; }
	return stack->top->type;
}

void PopItem(Stack* stack) {
	if (!stack) { return; }
	Symbol* tmp = stack->top;
	if (tmp) {
		stack->top = tmp->down;
		if (stack->top) {
			stack->top->up = NULL;
		}
	}
	free(tmp);
}

bool ExpandTop(Stack* stack, Token* token) {
	Symbol* nterm;
	if (!token || !stack || !(nterm = stack->top) || nterm->type != SYMBOL_NONTERMINAL) {
		return false;
	}
	Symbol* symbolString;
	bool result = ProduceString(nterm, token, &symbolString);

	return result;
}


/* Provadi dealokaci vsech symbolu a zasobniku, slouzi
 * primarne pro uvolneni pameti pri ukonceni programu,
 * ale je prizpusobeno i pro pripadnou realokaci
 */
void StackCleanup(void) {
	if (g_Stacks.array != NULL) {
		free(g_Stacks.array);
	}
	g_Stacks.array = NULL;
	g_Stacks.size = 0;

	if (g_Symbols.allocated != NULL) {
		//alokuji se vzdy pohromade, staci zkontrolovat jeden z nich
		free(g_Symbols.allocated);
		free(g_Symbols.unused);
	}
	g_Symbols.allocated = NULL;
	g_Symbols.unused = NULL;
	g_Symbols.size = g_Symbols.used = 0;
}



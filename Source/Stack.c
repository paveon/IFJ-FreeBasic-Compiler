#include <stdlib.h>
#include <string.h>
#include "CompilationErrors.h"
#include "Stack.h"
#include "LLtable.h"
#include "Symbol.h"

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
	if (terminal == T_BOTTOM) {
		newSymbol->type = SYMBOL_BOTTOM;
	}
	else {
		newSymbol->type = SYMBOL_TERMINAL;
	}
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
 * volne. Nasledujici zpusob se snazi minimalizovat celkovy
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
	Stack* freeStack;
	//Pokusime se najit nepouzity stack
	for (size_t i = 0; i < g_Stacks.size; i++) {
		freeStack = &g_Stacks.array[i];
		if (freeStack->inUse == false) {
			PushT(freeStack, T_BOTTOM);
			return freeStack; //Volny stack existuje
		}
	}

	//Vytvorime novy stack
	g_Stacks.size++;
	if ((freeStack = realloc(g_Stacks.array, sizeof(Stack) * g_Stacks.size)) == NULL) {
		FatalError(ER_FATAL_INTERNAL);
	}
	g_Stacks.array = freeStack;
	freeStack = &g_Stacks.array[g_Stacks.size - 1];
	freeStack->top = NULL;
	freeStack->inUse = true;
	PushT(freeStack, T_BOTTOM);
	return freeStack;
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

SymbolType GetSymbolType(const Stack* stack) {
	if (!stack) { return SYMBOL_UNDEFINED; }
	return stack->top->type;
}

Terminal GetTopT(const Stack* stack) {
	if (!stack) { return T_UNDEFINED; }
	return stack->top->data.terminal;
}

NTerminal GetTopNT(const Stack* stack) {
	if (!stack) { return NT_UNDEFINED; }
	return stack->top->data.nonTerminal;
}

const Symbol* GetTop(const Stack* stack) {
	if (!stack) { return NULL; }
	return stack->top;
}

void PopSymbol(Stack* stack) {
	if (!stack) { return; }
	Symbol* tmp = stack->top;
	if (tmp) {
		stack->top = tmp->down;
		if (stack->top) {
			stack->top->up = NULL;
		}
		ReleaseSymbol(tmp);
	}
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

bool CompareTop(const Stack* stack, const Token* token) {
	Symbol* symbol;
	if (!stack || !token || !(symbol = stack->top) || symbol->type != SYMBOL_TERMINAL) {
		return false;
	}
	Terminal terminal = symbol->data.terminal;

	if (terminal == T_ID && GetTokenType(token) == TOKEN_IDENTIFIER) {
		return true;
	}
	else if (strcmp(GetTerminalValue(terminal), GetTokenValue(token)) == 0) {
		return true;
	}
	return false;
}

bool ExpandTop(Stack* stack, const Token* token) {
	Symbol* nterm;
	if (!stack || !token || !(nterm = stack->top) || nterm->type != SYMBOL_NONTERMINAL) {
		return true; //false hodnota vyhrazena pro derivacni chybu
	}

	Rule rule = GetLLRule(nterm->data.nonTerminal, token);
	PopSymbol(stack);
	switch (rule) {
		case 1:
			PushNT(stack, NT_LINE_BREAK);
			PushT(stack, T_SCOPE);
			PushT(stack, T_END);
			PushNT(stack, NT_STATEMENT_LIST);
			PushT(stack, T_EOL);
			PushT(stack, T_SCOPE);
			break;
		case 2:
			PushNT(stack, NT_PROGRAM);
			PushNT(stack, NT_HEADER);
			PushT(stack, T_DECLARE);
			break;
		case 3:
			PushNT(stack, NT_PROGRAM);
			PushNT(stack, NT_FUNCTION);
			break;
		case 4:
			PushNT(stack, NT_LINE_BREAK);
			PushT(stack, T_EOL);
			PushNT(stack, NT_TYPE);
			PushT(stack, T_AS);
			PushT(stack, T_RIGHT_BRACKET);
			PushNT(stack, NT_ARGUMENT);
			PushT(stack, T_LEFT_BRACKET);
			PushT(stack, T_ID);
			PushT(stack, T_FUNCTION);
			break;
		case 5:
			PushNT(stack, NT_LINE_BREAK);
			PushT(stack, T_EOL);
			PushT(stack, T_FUNCTION);
			PushT(stack, T_END);
			PushNT(stack, NT_STATEMENT_LIST);
			PushNT(stack, NT_HEADER);
			break;
		case 6:
			PushNT(stack, NT_NEXT_ARGUMENT);
			PushNT(stack, NT_TYPE);
			PushT(stack, T_AS);
			PushT(stack, T_ID);
			break;
		case 7:
			PushNT(stack, NT_ARGUMENT);
			PushT(stack, T_COMMA);
			break;
		case 9:
			PushNT(stack, NT_STATEMENT_LIST);
			PushNT(stack, NT_LINE_BREAK);
			PushT(stack, T_EOL);
			PushNT(stack, NT_STATEMENT);
			break;
		case 11:
			PushNT(stack, NT_INITIALIZATION);
			PushNT(stack, NT_TYPE);
			PushT(stack, T_AS);
			PushT(stack, T_ID);
			PushT(stack, T_DIM);
			break;
		case 12:
			PushNT(stack, NT_EXPRESSION);
			PushT(stack, T_EQUAL);
			PushT(stack, T_ID);
			break;
		case 13:
			PushT(stack, T_ID);
			PushT(stack, T_INPUT);
			break;
		case 14:
			PushNT(stack, NT_NEXT_ARGUMENT);
			PushT(stack, T_SEMICOLON);
			PushNT(stack, NT_EXPRESSION);
			PushT(stack, T_PRINT);
			break;
		case 15:
			PushT(stack, T_LOOP);
			PushNT(stack, NT_STATEMENT_LIST);
			PushNT(stack, NT_LINE_BREAK);
			PushT(stack, T_EOL);
			PushNT(stack, NT_EXPRESSION);
			PushT(stack, T_WHILE);
			PushT(stack, T_DO);
			break;
		case 16:
			PushNT(stack, NT_EXPRESSION);
			PushT(stack, T_RETURN);
			break;
		case 17:
			PushNT(stack, NT_ELSE);
			PushNT(stack, NT_STATEMENT_LIST);
			PushNT(stack, NT_LINE_BREAK);
			PushT(stack, T_EOL);
			PushT(stack, T_THEN);
			PushNT(stack, NT_EXPRESSION);
			PushT(stack, T_IF);
			break;
		case 18:
			PushT(stack, T_IF);
			PushT(stack, T_END);
			PushNT(stack, NT_STATEMENT_LIST);
			PushNT(stack, NT_LINE_BREAK);
			PushT(stack, T_EOL);
			PushT(stack, T_ELSE);
			break;
		case 19:
			PushT(stack, T_IF);
			PushT(stack, T_END);
			break;
		case 20:
			PushNT(stack, NT_EXPRESSION);
			PushT(stack, T_SEMICOLON);
			PushNT(stack, NT_EXPRESSION);
			break;
		case 22:
			PushNT(stack, NT_EXPRESSION);
			PushT(stack, T_EQUAL);
			break;
		case 24:
			PushT(stack, T_STRING);
			break;
		case 25:
			PushT(stack, T_INTEGER);
			break;
		case 26:
			PushT(stack, T_DOUBLE);
			break;
		case 28:
			PushNT(stack, NT_LINE_BREAK);
			PushT(stack, T_EOL);

		case 8:
		case 10:
		case 21:
		case 27:
		case 23:
		case 29:
			//epsilon pravidla
			break;

		case 0:
		default:
			//Neexistujici pravidlo - chyba syntaxe
			return false;
	}
	return true;
}


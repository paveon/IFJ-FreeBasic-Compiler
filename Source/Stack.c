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
	Stack** array;
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
		}
	}

	//Pouzijeme nasledujici volny symbol a inkrementujeme pocitadlo pouzitych symbolu
	Symbol* freeSymbol = g_Symbols.unused[g_Symbols.used];
	g_Symbols.unused[g_Symbols.used++] = NULL;

	//Pred pouzitim (re)inicializujeme hodnoty
	freeSymbol->reduceEnd = false;
	freeSymbol->down = freeSymbol->up = NULL;
	return freeSymbol;
}


Stack* GetStack(void) {
	Stack* freeStack;
	//Pokusime se najit nepouzity stack
	for (size_t i = 0; i < g_Stacks.size; i++) {
		freeStack = g_Stacks.array[i];
		if (freeStack->inUse == false) {
			PushT(freeStack, T_BOTTOM);
			return freeStack; //Volny stack existuje
		}
	}

	//Vytvorime novy stack
	Stack** tmp;
	g_Stacks.size++;
	if ((tmp = realloc(g_Stacks.array, sizeof(Stack*) * g_Stacks.size)) == NULL ||
			(freeStack = malloc(sizeof(Stack))) == NULL) {
		FatalError(ER_FATAL_INTERNAL);
	}
	g_Stacks.array = tmp;
	g_Stacks.array[g_Stacks.size - 1] = freeStack;
	freeStack->top = NULL;
	freeStack->inUse = true;
	PushT(freeStack, T_BOTTOM);
	return freeStack;
}


/* Uvolni symboly a zasobnik pro opetovne pouziti (neprovadi dealokaci) */
void ReleaseStack(Stack* stack) {
	Symbol* current = stack->top;
	while (current) {
		g_Symbols.unused[--g_Symbols.used] = current; //Zpristupnime symbol pro dalsi pouziti
		stack->top = current->down;
		current = stack->top; //Posuneme se na dalsi symbol v zasobniku
	}
	stack->inUse = false; //Zasobnik se nepouziva
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
		g_Symbols.unused[--g_Symbols.used] = tmp; //Zpristupnime symbol pro dalsi pouziti
	}
}


/* Ukazatel na stack nebude nikdy NULL a stack bude vzdy obsahovat terminal
 * (minimalne terminal znacici konec vyrazu => EOL nebo ;)
 */
Terminal GetFirstTerminal(Stack *stack){
	Symbol *tmp = stack->top;
	Terminal term;
	while(tmp->type != SYMBOL_TERMINAL){
		tmp = tmp->down;
	}
	term = tmp->data.terminal;
	return term;
}


/* TODO nevim jak to okomentovat
 */
bool IsEndOfReduction(Stack *stack){
	return stack->top->reduceEnd;
}


/* Sprostredkovava pruchod zasobnikem s cilem nalezt T_FUNCTION.
 * Nepocita s tim, ze ukazatel na zasobnik bude NULL
 */
bool ContainingFunction(Stack *stack){
	SymbolType symbolType = GetSymbolType(stack);
	Symbol *symbol = stack->top;
	while(symbolType == SYMBOL_BOTTOM){
		if(symbolType == SYMBOL_TERMINAL){
			if((symbol->data.terminal) == T_FUNCTION){
				return true;
			}
		}
		symbol = symbol->down;
		symbolType = symbol->type;
	}
	return false;
}


/* Prochazi stack a hleda zanoreni (jako index) prvniho terminalu v zasobniku.
 * Nekontroluje zda je ukazatel na stack NULL! A take nepocita s tim,
 * ze zasobnik neobsahuje terminal.
 */
size_t LastSymBeforeFirstTerm(Stack *stack){
	SymbolType symbolType = GetSymbolType(stack);
	Symbol *symbol = stack->top;
	size_t idx = 0;
	while(symbolType != SYMBOL_TERMINAL){
		symbol = symbol->down;
		symbolType = symbol->type;
		idx++;
	}
	return idx;
}


/* Provadi nastaveni priznaku reductionEnd podle zadaneho indexu zanoreni.
 * Nekontroluje NULL ukazatel.
 */
void SetReduction(Stack *stack, size_t idx){
	Symbol *symbol = stack->top;
	for(size_t i = 0; i < idx; i++){
		symbol = symbol->down;
	}
	symbol->reduceEnd = true;
}


/* Zjistuje pocet vyskytu terminalu T_FUNCTION v zasobniku.
 * Funkce nekontroluje zasobnik s ukazatelem NULL.
 */
int CountOfFunc(Stack *stack){
	int count = 0;
	Symbol *symbol = stack->top;
	SymbolType symbolType = symbol->type;
	while(symbolType != SYMBOL_BOTTOM){
		if(symbolType == SYMBOL_TERMINAL){
			if(symbol->data.terminal == T_FUNCTION){
				count++;
			}
		}
		symbol = symbol->down;
		symbolType = symbol->type;
	}
	return count;
}


/* Provadi dealokaci vsech symbolu a zasobniku, slouzi
 * primarne pro uvolneni pameti pri ukonceni programu,
 * ale je prizpusobeno i pro pripadnou realokaci
 */
void StackCleanup(void) {
	if (g_Stacks.array != NULL) {
		for (size_t i = 0; i < g_Stacks.size; i++) {
			free(g_Stacks.array[i]);
		}
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

bool CompareTop(const Stack* stack, Terminal tokenTerminal) {
	Symbol* symbol;
	if (!stack || !(symbol = stack->top) || symbol->type != SYMBOL_TERMINAL) {
		return false;
	}
	Terminal stackTerminal = symbol->data.terminal;

	if (stackTerminal == tokenTerminal) {
		return true;
	}
	return false;
}

bool ExpandTop(Stack* stack, Terminal tokenTerminal) {
	Symbol* nterm;
	if (!stack || !(nterm = stack->top) || nterm->type != SYMBOL_NONTERMINAL) {
		return true; //false hodnota vyhrazena pro derivacni chybu
	}

	Rule rule = GetLLRule(nterm->data.nonTerminal, tokenTerminal);
	PopSymbol(stack);
	switch (rule) {
		case RULE_MAIN_SCOPE:
			PushNT(stack, NT_LINE_BREAK);
			PushNT(stack, NT_SCOPE);
			break;
		case RULE_NEW_SCOPE:
			PushT(stack, T_SCOPE);
			PushT(stack, T_END);
			PushNT(stack, NT_STATEMENT_LIST);
			PushT(stack, T_EOL);
			PushT(stack, T_SCOPE);
			break;
		case RULE_FUNC_DECL:
			PushNT(stack, NT_PROGRAM);
			PushNT(stack, NT_HEADER);
			PushT(stack, T_DECLARE);
			break;
		case RULE_FUNC_DEF:
			PushNT(stack, NT_PROGRAM);
			PushNT(stack, NT_FUNCTION);
			break;
		case RULE_FUNC_HEADER:
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
		case RULE_FUNC_BODY:
			PushNT(stack, NT_LINE_BREAK);
			PushT(stack, T_EOL);
			PushT(stack, T_FUNCTION);
			PushT(stack, T_END);
			PushNT(stack, NT_STATEMENT_LIST);
			PushNT(stack, NT_HEADER);
			break;
		case RULE_FUNC_ARG:
			PushNT(stack, NT_NEXT_ARGUMENT);
			PushNT(stack, NT_TYPE);
			PushT(stack, T_AS);
			PushT(stack, T_ID);
			break;
		case RULE_FUNC_NEXT_ARG:
			PushNT(stack, NT_ARGUMENT);
			PushT(stack, T_COMMA);
			break;
		case RULE_ST_LIST:
			PushNT(stack, NT_STATEMENT_LIST);
			PushNT(stack, NT_LINE_BREAK);
			PushT(stack, T_EOL);
			PushNT(stack, NT_STATEMENT);
			break;
		case RULE_ST_VAR_DECL:
			PushNT(stack, NT_INITIALIZATION);
			PushNT(stack, NT_TYPE);
			PushT(stack, T_AS);
			PushT(stack, T_ID);
			PushT(stack, T_DIM);
			break;
		case RULE_ST_FUNC_CALL:
			PushNT(stack, NT_EXPRESSION);
			PushT(stack, T_OPERATOR_EQUAL);
			PushT(stack, T_ID);
			break;
		case RULE_ST_INPUT:
			PushT(stack, T_ID);
			PushT(stack, T_INPUT);
			break;
		case RULE_ST_PRINT:
			PushNT(stack, NT_NEXT_EXPRESSION);
			PushT(stack, T_SEMICOLON);
			PushNT(stack, NT_EXPRESSION);
			PushT(stack, T_PRINT);
			break;
		case RULE_ST_WHILE:
			PushT(stack, T_LOOP);
			PushNT(stack, NT_STATEMENT_LIST);
			PushNT(stack, NT_LINE_BREAK);
			PushT(stack, T_EOL);
			PushNT(stack, NT_EXPRESSION);
			PushT(stack, T_WHILE);
			PushT(stack, T_DO);
			break;
		case RULE_ST_RETURN:
			PushNT(stack, NT_EXPRESSION);
			PushT(stack, T_RETURN);
			break;
		case RULE_ST_IF:
			PushNT(stack, NT_ELSE);
			PushNT(stack, NT_ELSEIF);
			PushNT(stack, NT_STATEMENT_LIST);
			PushNT(stack, NT_LINE_BREAK);
			PushT(stack, T_EOL);
			PushT(stack, T_THEN);
			PushNT(stack, NT_EXPRESSION);
			PushT(stack, T_IF);
			break;
		case RULE_ELSEIF:
			PushNT(stack, NT_ELSEIF);
			PushNT(stack, NT_STATEMENT_LIST);
			PushNT(stack, NT_LINE_BREAK);
			PushT(stack, T_EOL);
			PushT(stack, T_THEN);
			PushNT(stack, NT_EXPRESSION);
			PushT(stack, T_ELSEIF);
			break;
		case RULE_ELSE:
			PushT(stack, T_IF);
			PushT(stack, T_END);
			PushNT(stack, NT_STATEMENT_LIST);
			PushNT(stack, NT_LINE_BREAK);
			PushT(stack, T_EOL);
			PushT(stack, T_ELSE);
			break;
		case RULE_END_IF:
			PushT(stack, T_IF);
			PushT(stack, T_END);
			break;
		case RULE_NEXT_EXPR:
			PushNT(stack, NT_EXPRESSION);
			PushT(stack, T_SEMICOLON);
			PushNT(stack, NT_EXPRESSION);
			break;
		case RULE_VAR_INIT:
			PushNT(stack, NT_EXPRESSION);
			PushT(stack, T_OPERATOR_EQUAL);
			break;
		case RULE_TYPE_STRING:
			PushT(stack, T_STRING);
			break;
		case RULE_TYPE_INT:
			PushT(stack, T_INTEGER);
			break;
		case RULE_TYPE_DOUBLE:
			PushT(stack, T_DOUBLE);
			break;
		case RULE_LINE_BREAK:
			PushNT(stack, NT_LINE_BREAK);
			PushT(stack, T_EOL);
			break;
		case RULE_VAR_GLOBAL:
			PushNT(stack, NT_PROGRAM);
			PushNT(stack, NT_LINE_BREAK);
			PushT(stack, T_EOL);
			PushNT(stack, NT_INITIALIZATION);
			PushNT(stack, NT_TYPE);
			PushT(stack, T_AS);
			PushT(stack, T_ID);
			PushT(stack, T_SHARED);
			PushT(stack, T_DIM);
			break;
		case RULE_ST_VAR_STATIC:
			PushNT(stack, NT_INITIALIZATION);
			PushNT(stack, NT_TYPE);
			PushT(stack, T_AS);
			PushT(stack, T_ID);
			PushT(stack, T_STATIC);
			break;

		case RULE_EPSILON:
			break;

		case RULE_MISSING:
		default:
			//Neexistujici pravidlo - chyba syntaxe
			return false;
	}
	return true;
}


#include <stdlib.h>
#include <stdbool.h>
#include "CompilationErrors.h"
#include "Stack.h"


struct StackItem {
	struct StackItem* up;
	struct StackItem* down;
	bool reduceEnd;
	ItemType type;

	//Nepotrebujeme vyuzivat obe promenne zaraz
	union {
		const char* term;
		NTerm nonterm;
	} data;
};

struct Stack {
	StackItem* top;
};


static Term const PredictTerminals[] = {
		  "DECLARE", "DIM", "DO", "DOUBLE", "ELSE", "END", "FUNCTION",
		  "IF", "INPUT", "INTEGER", "LOOP", "PRINT", "RETURN", "SCOPE",
		  "STRING", "ID", ",", ")", "=", "EOL"
};

StackItem* CreateItem(Stack* stack, ItemType type) {
	StackItem* item;
	if ((item = malloc(sizeof(StackItem))) == NULL) {
		FatalError(ER_FATAL_INTERNAL);
	}

	item->up = NULL;
	item->down = stack->top;
	item->type = type;
	item->reduceEnd = false;
	stack->top->up = item;
	stack->top = item;
	return item;
}


ItemType TopItemType(const Stack* stack) {
	if (!stack) { return TYPE_UNDEFINED; }
	else if (!stack->top) { return TYPE_BOTTOM; }
	return stack->top->type;
}

void PopItem(Stack* stack) {
	if (!stack) { return; }
	StackItem* tmp = stack->top;
	if (tmp) {
		stack->top = tmp->down;
		if (stack->top) {
			stack->top->up = NULL;
		}
	}
	free(tmp);
}

bool ExpandNT(Stack* stack, Token* token) {
	StackItem* nterm;
	if (!token || !stack || !(nterm = stack->top) || nterm->type != TYPE_NONTERMINAL) {
		return false;
	}

	return true;
}

Stack* CreateStack(void) {
	Stack* newStack = NULL;

	if ((newStack = malloc(sizeof(Stack))) == NULL) {
		FatalError(ER_FATAL_INTERNAL);
	}
	newStack->top = NULL;

	return newStack;
}


void DeleteStack(Stack* stack) {
	if (!stack) { return; }

	StackItem* current = stack->top;
	while (current) {
		stack->top = current->down;
		free(current);
		current = stack->top;
	}
	free(stack);
}

void PushNonTerminal(Stack* stack, NTerm nonterm) {
	if (!stack) { return; }

	StackItem* item = CreateItem(stack, TYPE_NONTERMINAL);
	item->data.nonterm = nonterm;
}

void PushTerminal(Stack* stack, const char* term) {
	if (!stack || !term) { return; }

	StackItem* item = CreateItem(stack, TYPE_TERMINAL);
	item->data.term = term;
}



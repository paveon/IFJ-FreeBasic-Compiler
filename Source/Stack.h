#ifndef FREEBASIC_COMPILER_STACK_H
#define FREEBASIC_COMPILER_STACK_H

#include "Token.h"

typedef enum ItemType {
	TYPE_UNDEFINED,
	TYPE_BOTTOM,
	TYPE_TERMINAL,
	TYPE_NONTERMINAL
} ItemType;

typedef enum NonTerminal {
	NT_PROGRAM,
	NT_HEADER,
	NT_FUNCTION,
	NT_ARGUMENT,
	NT_NEXT_ARGUMENT,
	NT_STATEMENT_LIST,
	NT_STATEMENT,
	NT_ELSE,
	NT_NEXT_EXPRESSION,
	NT_INITIALIZATION,
	NT_TYPE
} NTerm;

typedef const char* Term;
typedef struct StackItem StackItem;
typedef struct Stack Stack;

ItemType TopItemType(const Stack* stack);

void PopItem(Stack* stack);

bool ExpandNT(Stack* stack, Token* token);

Stack* CreateStack(void);

void DeleteStack(Stack* stack);

void PushNonTerminal(Stack* stack, NTerm nonterm);

void PushTerminal(Stack* stack, Term term);


#endif //FREEBASIC_COMPILER_STACK_H

#ifndef FREEBASIC_COMPILER_LLTABLE_H
#define FREEBASIC_COMPILER_LLTABLE_H

#include <stdbool.h>
#include "Symbol.h"
#include "Token.h"

typedef unsigned char Rule;

Rule GetLLRule(NTerminal nTerminal, const Token* token);

const char* GetTerminalValue(Terminal terminal);


#endif //FREEBASIC_COMPILER_LLTABLE_H

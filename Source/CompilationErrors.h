#include <stdio.h>
#include <stdlib.h>

#ifndef FREEBASIC_COMPILER_COMPILATIONERRORS_H
#define FREEBASIC_COMPILER_COMPILATIONERRORS_H

typedef enum ErrorCode {
	ER_FATAL_LEXICAL_ANALYSIS,
	ER_FATAL_SYNTAX_ANALYSIS,
	ER_FATAL_SEMANTIC_DEFINITIONS,
	ER_FATAL_SEMANTIC_TYPES,
	ER_FATAL_SEMANTIC_OTHER,
	ER_FATAL_INTERNAL
} ErrorCode;

void FatalError(const char* function, const char* sourceFile, int line, ErrorCode errorCode);
#define FatalError(errorCode) FatalError(__func__, __FILE__, __LINE__, errorCode)

#endif //FREEBASIC_COMPILER_COMPILATIONERRORS_H

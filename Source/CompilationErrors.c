#include "CompilationErrors.h"
#include "Token.h"
#include "symtable.h"
#include "Stack.h"

#undef FatalError

typedef enum InternalExitCode {
	EXIT_CODE_LEXICAL_ANALYSIS = 1,
	EXIT_CODE_SYNTAX_ANALYSIS = 2,
	EXIT_CODE_SEMANTIC_DEFINITIONS = 3,
	EXIT_CODE_SEMANTIC_TYPES = 4,
	EXIT_CODE_SEMANTIC_OTHER = 6,
	EXIT_CODE_INTERNAL = 99
} InternalExitCode;

typedef struct ErrorMetadata {
	const char* errorMessage;
	InternalExitCode exitCode;
} ErrorMetadata;

static ErrorMetadata errors[] = {
		  {"placeholder...\n",                                     EXIT_CODE_LEXICAL_ANALYSIS},
		  {"placeholder...\n",                                     EXIT_CODE_SYNTAX_ANALYSIS},
		  {"placeholder...\n",                                     EXIT_CODE_SEMANTIC_DEFINITIONS},
		  {"placeholder...\n",                                     EXIT_CODE_SEMANTIC_TYPES},
		  {"placeholder...\n",                                     EXIT_CODE_SEMANTIC_OTHER},
		  {"memory allocation failed",                             EXIT_CODE_INTERNAL},
		  {"variable redeclaration",                               EXIT_CODE_SEMANTIC_DEFINITIONS},
		  {"function was declaration after definition",            EXIT_CODE_SEMANTIC_DEFINITIONS},
		  {"function was already declared",                        EXIT_CODE_SEMANTIC_DEFINITIONS},
		  {"redefinition of existing function",                    EXIT_CODE_SEMANTIC_DEFINITIONS},
		  {"redefinition of function parameter",                   EXIT_CODE_SEMANTIC_DEFINITIONS},
		  {"function definition has a wrong number of parameters", EXIT_CODE_SEMANTIC_DEFINITIONS},
		  {"parameter type mismatch in function signatures",       EXIT_CODE_SEMANTIC_DEFINITIONS},
		  {"return type mismatch in function signatures",          EXIT_CODE_SEMANTIC_DEFINITIONS}
};


void SemanticError(size_t line, ErrorCode errorCode) {
	fprintf(stderr, "Error: %s\nLine no. %zd\n", errors[errorCode].errorMessage, line);
}

void FatalError(const char* function, const char* sourceFile, int line, ErrorCode index) {
	TokenCleanup();
	TableCleanup(true);
	StackCleanup();

	fprintf(stderr, "Fatal error: %s\nExiting from: %s\nSource file: %s\nLine no. %d\n",
	        errors[index].errorMessage, function, sourceFile, line);

	exit(errors[index].exitCode);
}


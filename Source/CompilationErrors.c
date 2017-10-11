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
				{"placeholder...\n",                                          EXIT_CODE_LEXICAL_ANALYSIS},
				{"placeholder...\n",                                          EXIT_CODE_SYNTAX_ANALYSIS},
				{"placeholder...\n",                                          EXIT_CODE_SEMANTIC_DEFINITIONS},
				{"placeholder...\n",                                          EXIT_CODE_SEMANTIC_TYPES},
				{"placeholder...\n",                                          EXIT_CODE_SEMANTIC_OTHER},
				{"memory allocation failed",                                  EXIT_CODE_INTERNAL},
				{"variable '%s' redeclaration",                               EXIT_CODE_SEMANTIC_DEFINITIONS},
				{"function '%s' was declaration after definition",            EXIT_CODE_SEMANTIC_DEFINITIONS},
				{"function '%s' was already declared",                        EXIT_CODE_SEMANTIC_DEFINITIONS},
				{"redefinition of existing function '%s'",                    EXIT_CODE_SEMANTIC_DEFINITIONS},
				{"redefinition of function '%s' parameter",                   EXIT_CODE_SEMANTIC_DEFINITIONS},
				{"function '%s' definition has a wrong number of parameters", EXIT_CODE_SEMANTIC_DEFINITIONS},
				{"parameter type mismatch in %s's signatures",                EXIT_CODE_SEMANTIC_DEFINITIONS},
				{"return type mismatch in %s's signatures",                   EXIT_CODE_SEMANTIC_DEFINITIONS}
};


void SemanticError(size_t line, ErrorCode errorCode, const char* extra) {
	char buffer[256];

	if (extra) {
		switch (errorCode) {
			case ER_SEMANTIC_VAR_REDECL:
			case ER_SEMANTIC_DECL_AFTER_DEF:
			case ER_SEMANTIC_FUNC_REDECL:
			case ER_SEMANTIC_FUNC_REDEF:
			case ER_SEMANTIC_PARAM_REDEF:
			case ER_SEMANTIC_PARAM_COUNT:
			case ER_SEMANTIC_PARAM_MISMATCH:
			case ER_SEMANTIC_RETURN_MISMATCH:
				sprintf(buffer, errors[errorCode].errorMessage, extra);
				break;

			default:
				buffer[0] = 0;
				break;
		}
		fprintf(stderr, "Error: %s\nLine no. %zd\n", buffer, line);
	}
	else {
		fprintf(stderr, "Error: %s\nLine no. %zd\n", errors[errorCode].errorMessage, line);
	}
}

void FatalError(const char* function, const char* sourceFile, int line, ErrorCode index) {
	TokenCleanup();
	TableCleanup(true);
	StackCleanup();

	fprintf(stderr, "Fatal error: %s\nExiting from: %s\nSource file: %s\nLine no. %d\n",
	        errors[index].errorMessage, function, sourceFile, line);

	exit(errors[index].exitCode);
}


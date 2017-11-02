#include "CompilationErrors.h"
#include "symtable.h"
#include "Stack.h"
#include "TopDown.h"
#include "Lexical.h"


#undef FatalError

typedef enum InternalExitCode {
	EC_LEXICAL = 1,
	EC_SYNTAX = 2,
	EC_SEMANTIC_DEFINITIONS = 3,
	EC_SEMANTIC_TYPES = 4,
	EC_SEMANTIC_OTHER = 6,
	EC_INTERNAL = 99
} InternalExitCode;

typedef struct ErrorMetadata {
	const char* errorMessage;
	InternalExitCode exitCode;
} ErrorMetadata;

static ErrorMetadata errors[] = {
				{"placeholder...\n",
								EC_LEXICAL},

				{"placeholder...\n",
								EC_SYNTAX},

				{"placeholder...\n",
								EC_SEMANTIC_DEFINITIONS},

				{"placeholder...\n",
								EC_SEMANTIC_TYPES},

				{"placeholder...\n",
								EC_SEMANTIC_OTHER},

				{"memory allocation failed",
								EC_INTERNAL},

				{"variable '%s' redeclaration",
								EC_SEMANTIC_DEFINITIONS},

				{"undefined variable '%s'",
								EC_SEMANTIC_DEFINITIONS},

				{"function '%s' was declared after definition",
								EC_SEMANTIC_DEFINITIONS},

				{"function '%s' was already declared",
								EC_SEMANTIC_DEFINITIONS},

				{"redefinition of existing function '%s'",
								EC_SEMANTIC_DEFINITIONS},

				{"missing definition of function '%s'",
								EC_SEMANTIC_DEFINITIONS},

				{"parameter of function '%s' was redefined",
								EC_SEMANTIC_DEFINITIONS},

				{"definition of function '%s' has a wrong number of parameters",
								EC_SEMANTIC_DEFINITIONS},

				{"parameter type mismatch in %s's signatures",
								EC_SEMANTIC_DEFINITIONS},

				{"return type mismatch in %s's signatures",
								EC_SEMANTIC_DEFINITIONS},

				{"missing operator in expression",
								EC_SEMANTIC_DEFINITIONS},

				{"unknown expression",
								EC_SEMANTIC_DEFINITIONS},

				{"unexpected symbol '%s' in expression",
								EC_SEMANTIC_DEFINITIONS},

				{"unexpected space after function identifier '%s'",
								EC_SEMANTIC_DEFINITIONS},

				{"function '%s' is undeclared",
								EC_SEMANTIC_DEFINITIONS},

				{"usage of comparative operators in non condition based expressions",
								EC_SEMANTIC_DEFINITIONS},

				{"invalid combination of string and number in one expression",
								EC_SEMANTIC_DEFINITIONS},
};


void SemanticError(size_t line, ErrorCode errorCode, const char* extra) {
	if (extra) {
		char buffer[256];

		switch (errorCode) {
			case ER_SMC_VAR_REDECL:
			case ER_SMC_FUNC_DECL_AFTER_DEF:
			case ER_SMC_FUNC_REDECL:
			case ER_SMC_FUNC_REDEF:
			case ER_SMC_FUNC_NO_DEF:
			case ER_SMC_FUNC_PARAM_REDEF:
			case ER_SMC_FUNC_PARAM_COUNT:
			case ER_SMC_FUNC_PARAM_TYPE:
			case ER_SMC_FUNC_RETURN_TYPE:
			case ER_SMC_VAR_UNDEF:
			case ER_SMC_UNEXPECT_SYM:
			case ER_SMC_FUNC_UNDECL:
			case ER_SMC_COMPARATIVE_EXPR:
			case ER_SMC_STR_AND_NUM:
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
	LexCleanup();
	TokenCleanup();
	TableCleanup();
	StackCleanup();
	TopDownCleanup();

	fprintf(stderr, "Fatal error: %s\nExiting from: %s\nSource file: %s\nLine no. %d\n",
					errors[index].errorMessage, function, sourceFile, line);

	exit(errors[index].exitCode);
}


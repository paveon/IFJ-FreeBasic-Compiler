#include "CompilationErrors.h"
#include "symtable.h"
#include "Stack.h"
#include "TopDown.h"
#include "Lexical.h"
#include "CodeGenerator.h"


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

static ErrorMetadata errors[25] = {
				[ER_FATAL_INTERNAL] = {"memory allocation failed",
								EC_INTERNAL},

				[ER_SMC_VAR_REDECL] = {"variable '%s' redeclaration",
															 EC_SEMANTIC_DEFINITIONS},

				[ER_SMC_VAR_UNDEF] = {"undefined variable '%s'",
															EC_SEMANTIC_DEFINITIONS},

				[ER_SMC_VAR_TYPE] = {"type of '%s' variable is not compatible with type of expression",
														 EC_SEMANTIC_TYPES},

				[ER_SMC_FUNC_DECL_AFTER_DEF] = {"function '%s' was declared after definition",
								EC_SEMANTIC_DEFINITIONS},

				[ER_SMC_FUNC_REDECL] = {"function '%s' was already declared",
																EC_SEMANTIC_DEFINITIONS},

				[ER_SMC_FUNC_REDEF] = {"redefinition of existing function '%s'",
															 EC_SEMANTIC_DEFINITIONS},

				[ER_SMC_FUNC_NO_DEF] = {"missing definition of function '%s'",
																EC_SEMANTIC_DEFINITIONS},

				[ER_SMC_FUNC_PARAM_REDEF] = {"parameter of function '%s' was redefined",
																		 EC_SEMANTIC_DEFINITIONS},

				[ER_SMC_FUNC_PARAM_COUNT] = {"definition of function '%s' has a wrong number of parameters",
																		 EC_SEMANTIC_DEFINITIONS},

				[ER_SMC_FUNC_PARAM_TYPE] = {"parameter type mismatch in %s's signatures",
																		EC_SEMANTIC_DEFINITIONS},

				[ER_SMC_FUNC_RETURN_TYPE] = {"return type mismatch in %s's signatures",
																		 EC_SEMANTIC_DEFINITIONS},

				[ER_SMC_FUNC_RETURN_EXPR] = {"returning value of invalid type from function '%s'",
																		 EC_SEMANTIC_TYPES},

				[ER_SMC_MISSING_OP] = {"missing operator in expression",
								EC_SEMANTIC_DEFINITIONS},

				[ER_SMC_UNKNOWN_EXPR] = {"unknown expression",
																 EC_SEMANTIC_DEFINITIONS},

				[ER_SMC_UNEXPECT_SYM] = {"unexpected symbol '%s' in expression",
																 EC_SEMANTIC_DEFINITIONS},

				[ER_SMC_UNEXP_FUNC_SPACE] = {"unexpected space after function identifier '%s'",
																		 EC_SEMANTIC_DEFINITIONS},

				[ER_SMC_FUNC_UNDECL] = {"function '%s' is undeclared",
																EC_SEMANTIC_DEFINITIONS},

				[ER_SMC_COMPARATIVE_EXPR] = {
								"usage of comparative operators in non condition based expressions",
								EC_SEMANTIC_DEFINITIONS},

				[ER_SMC_STR_AND_NUM] = {"invalid combination of string and number in one expression",
																EC_SEMANTIC_DEFINITIONS},

				[ER_SMC_MANY_ARGS] = {"too many parameters in function %s",
															EC_SEMANTIC_DEFINITIONS},

				[ER_SMC_LESS_ARGS] = {"too less parameters in function %s",
															EC_SEMANTIC_DEFINITIONS},

				[ER_SMC_ARG_TYPES] = {"wrong parameters types in function %s",
															EC_SEMANTIC_DEFINITIONS},

				[ER_SMC_INT_DIV] = {"integer division '\\' is only for integer types on both sides",
														EC_SEMANTIC_DEFINITIONS},
};


void SemanticError(size_t line, ErrorCode errorCode, const char* extra) {
	if (extra) {
		char buffer[256];

		switch (errorCode) {
			case ER_SMC_VAR_REDECL:
			case ER_SMC_VAR_UNDEF:
			case ER_SMC_VAR_TYPE:
			case ER_SMC_FUNC_DECL_AFTER_DEF:
			case ER_SMC_FUNC_REDECL:
			case ER_SMC_FUNC_REDEF:
			case ER_SMC_FUNC_NO_DEF:
			case ER_SMC_FUNC_PARAM_REDEF:
			case ER_SMC_FUNC_PARAM_COUNT:
			case ER_SMC_FUNC_PARAM_TYPE:
			case ER_SMC_FUNC_RETURN_TYPE:
			case ER_SMC_FUNC_RETURN_EXPR:
			case ER_SMC_UNEXPECT_SYM:
			case ER_SMC_FUNC_UNDECL:
			case ER_SMC_COMPARATIVE_EXPR:
			case ER_SMC_STR_AND_NUM:
			case ER_SMC_MANY_ARGS:
			case ER_SMC_LESS_ARGS:
			case ER_SMC_ARG_TYPES:
			case ER_SMC_INT_DIV:
				sprintf(buffer, errors[errorCode].errorMessage, extra);
				break;

			default:
				buffer[0] = 0;
				break;
		}
		fprintf(stderr, "Error: %s\nLine no. %d\n", buffer, (int) line);
	}
	else {
		fprintf(stderr, "Error: %s\nLine no. %d\n", errors[errorCode].errorMessage, (int) line);
	}
}

void FatalError(const char* function, const char* sourceFile, int line, ErrorCode index) {
	LexCleanup();
	TokenCleanup();
	TableCleanup();
	StackCleanup();
	TopDownCleanup();
	GeneratorCleanup();

	fprintf(stderr, "Fatal error: %s\nExiting from: %s\nSource file: %s\nLine no. %d\n",
					errors[index].errorMessage, function, sourceFile, line);

	exit(errors[index].exitCode);
}


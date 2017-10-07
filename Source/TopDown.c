#include <stdio.h>
#include "CompilationErrors.h"
#include "TopDown.h"
#include "Stack.h"
#include "LLtable.h"
#include "symtable.h"

typedef enum Code {
	ANALYSIS_CONTINUE,
	ANALYSIS_ERROR,
	ANALYSIS_SUCCESS
} Code;

const char* NTerminalText[12] = {
		  [NT_PROGRAM] = "<prog>",
		  [NT_HEADER] = "<header>",
		  [NT_FUNCTION] = "<func>",
		  [NT_ARGUMENT] = "<argument>",
		  [NT_NEXT_ARGUMENT] = "<next_arg>",
		  [NT_STATEMENT_LIST] = "<stat_list>",
		  [NT_STATEMENT] = "<stat>",
		  [NT_ELSE] = "<else>",
		  [NT_NEXT_EXPRESSION] = "<next_expr>",
		  [NT_INITIALIZATION] = "<init>",
		  [NT_TYPE] = "<type>",
		  [NT_EXPRESSION] = "<expr>",
};

bool ParseProgram(void) {
	/* Dodatecne informacni promenne */
	size_t currentLine = 1;

	/* Stavove flagy */
	bool mainScope = false;
	bool defineFunc = false;
	bool declareFunc = false;
	bool declareVar = false;
	bool declareArg = false;


	const char* value;
	Identifier* varID = NULL;
	Identifier* funcID = NULL;
	Stack* stack = GetStack();
	Token* token = GetNextToken();
	TokenType tokenType;
	SymbolType symbolType;
	NTerminal nTerminal;
	Terminal terminal;
	PushNT(stack, NT_PROGRAM);
	symbolType = GetSymbolType(stack);

	Code result = ANALYSIS_CONTINUE;
	while (result == ANALYSIS_CONTINUE) {
		switch (symbolType) {
			case SYMBOL_BOTTOM:
				tokenType = GetTokenType(token);
				if (tokenType == TOKEN_EOF) {
					printf("-- Syntactically correct --\n");
					result = ANALYSIS_SUCCESS;
				}
				else {
					result = ANALYSIS_ERROR;
				}
				break;

			case SYMBOL_TERMINAL:
				value = GetTerminalValue(GetTopT(stack));
				if (*value == '\n') {
					printf("-- Comparing terminal [EOL] --\n");
				}
				else {
					printf("-- Comparing terminal [%s] --\n", value);
				}

				if (CompareTop(stack, token)) {
					terminal = GetTopT(stack);

					//Dodatecne operace podle typu terminalu
					switch (terminal) {
						case T_SCOPE:
							//Nachazime se v hlavnim tele programu
							mainScope = true;
							break;

						case T_RETURN:
							//Volani return v hlavni funkci znamena chybu
							if (mainScope) { result = ANALYSIS_ERROR; }
							break;

						case T_ID:
							value = GetTokenValue(token);
							if (declareVar) {
								//Deklarace lokalni promenne
								varID = InsertLocalID(value);
							}
							else if (declareArg) {

							}
							else if (declareFunc) {
								funcID = LookupGlobalID(value);
								if (funcID) {
									//Chybove stavy
									if (funcID->declared) {
										//Pozdni deklarace jiz definovane funkce
										SemanticError(currentLine, ER_SEMANTIC_DECL_AFTER_DEF);
									}
									else {
										//Redeklarace
										SemanticError(currentLine, ER_SEMANTIC_REDECLARATION);
									}
								}
								else {
									//Funkce zatim nebyla deklarovana / definovana
									funcID = InsertGlobalID(value);
									funcID->declared = true; //Jedna se o deklaraci
								}
							}

							break;

						case T_INTEGER:
						case T_DOUBLE:
						case T_STRING:
							if (declareVar) {
								//Typ promenne
								SetSignature(varID, terminal, true);
							}
							else if (declareFunc || defineFunc) {
								if (declareArg) {
									//Typ argumentu funkce
									SetSignature(funcID, terminal, false);
								}
								else {
									//Navratovy typ funkce
									SetSignature(funcID, terminal, true);
								}
							}
							break;

						case T_DIM:
							//Zacatek deklarace promenne
							declareVar = true;
							break;

						case T_DECLARE:
							//Zacatek deklarace funkce
							declareFunc = true;
							break;

						case T_FUNCTION:
							if (!declareFunc) {
								//Zacatek definice funkce
								defineFunc = true;
							}
							break;

						case T_LEFT_BRACKET:
							//Zacatek deklarace parametru
							declareArg = true;
							break;

						case T_RIGHT_BRACKET:
							//Konec deklarace parametru
							declareArg = false;
							break;

						case T_END:
							if (defineFunc) {
								//Ukonceni definice funkce
								defineFunc = false;
								EndScope();
							}
							else if (mainScope) {
								//Ukonceni hlavniho tela programu
								//TODO: nejspis bude potreba dealokaci odlozit...
								TableCleanup(true);
							}
							else {
								//Ukonceni bloku (if, while...)
								EndSubScope();
							}
							break;

						case T_EOL:
							//Konec radku ukoncuje deklarace
							declareVar = false;
							declareFunc = false;
							declareArg = false;

							//Pocitadlo radku zdrojoveho kodu
							currentLine++;
							break;

						default:
							break;
					}

					PopSymbol(stack);
					symbolType = GetSymbolType(stack);
					token = GetNextToken();
				}
				else {
					result = ANALYSIS_ERROR;
				}
				break;

			case SYMBOL_NONTERMINAL:
				nTerminal = GetTopNT(stack);
				printf("-- Derivating [%s] --\n", NTerminalText[nTerminal]);
				if (ExpandTop(stack, token)) {
					symbolType = GetSymbolType(stack);
				}
				else {
					result = ANALYSIS_ERROR;
				}
				break;

			default:
				fprintf(stderr, "Undefined symbol type!\n");
				result = ANALYSIS_ERROR;
				break;
		}
	}
	ReleaseStack(stack);
	if (result == ANALYSIS_ERROR) {
		fprintf(stderr, "Syntax error!\n");
	}

	return result;
}
#include <stdio.h>
#include "CompilationErrors.h"
#include "TopDown.h"
#include "Stack.h"
#include "LLtable.h"
#include "symtable.h"

#define CHUNK 20

typedef enum Code {
	ANALYSIS_CONTINUE,
	ANALYSIS_ERROR,
	ANALYSIS_SUCCESS
} Code;

const char* NTerminalText[20] = {
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
				[NT_LINE_BREAK] = "<line_break>"
};


/* Struktura pro sledovani deklarovanych, ale nedefinovanych funkci.
 * Jedna se o semantickou chybu a deklarace nachazejici se ve strukture
 * pri vstupu do hlavniho scopu budou vypsany
 */
typedef struct Declarations {
	Identifier** array;
	size_t size;
	size_t used;
} Declarations;
static Declarations g_Decls;

/* Vlozi deklaraci funkce do struktury, pouzivat pri prvotni deklaraci funkce */
void AddDeclaration(Identifier* function, size_t line);

/* Odstrani deklaraci funkce ze struktury, pouzivat pri nalezeni definice funkce */
void ConfirmDeclaration(Identifier* function);


bool ParseProgram(void) {
	/* Dodatecne informacni promenne */
	size_t currentLine = 1;

	/* Stavove flagy */
	bool mainScope = false;
	bool defineFunc = false;
	bool declareFunc = false;
	bool declareVar = false;
	bool declareArg = false;
	size_t paramCount = 0;


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
	PushNT(stack, NT_LINE_BREAK);
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
					printf("-- Comparing terminal\t[EOL]\n");
				}
				else {
					printf("-- Comparing terminal\t[%s]\n", value);
				}

				if (CompareTop(stack, token)) {
					terminal = GetTopT(stack);

					//Dodatecne operace podle typu terminalu
					switch (terminal) {
						case T_SCOPE:
							//Nachazime se v hlavnim tele programu
							mainScope = !mainScope; //Toggle

							//Pri vstupu do main scopu zkontrolujeme, zda byly
							// vsechny deklarovane funkce definovany
							if (mainScope) {
								for (size_t i = 0; i < g_Decls.used; i++) {
									Identifier* func = g_Decls.array[i];
									SemanticError(func->declaredOnLine, ER_SMC_FUNC_NO_DEF, func->name);
								}
							}
							break;

						case T_RETURN:
							//Volani return v hlavni funkci znamena chybu
							if (mainScope) { result = ANALYSIS_ERROR; }
							break;

						case T_ID:
							value = GetTokenValue(token);
							if (declareVar) {
								varID = LookupID(value);
								if (varID) {
									//Promenna jiz existuje (redeklarace)
									SemanticError(currentLine, ER_SMC_VAR_REDECL, varID->name);
									varID = NULL; //Nechceme dale upravovat originalni deklaraci
								}
								else {
									//Deklarace lokalni promenne
									varID = InsertLocalID(value);
								}
							}
							else if (declareFunc && !declareArg) {
								funcID = LookupGlobalID(value);
								if (funcID) {
									//Chybove stavy
									if (funcID->declaration == false) {
										//Pozdni deklarace jiz definovane funkce
										SemanticError(currentLine, ER_SMC_FUNC_DECL_AFTER_DEF, funcID->name);
									}
									else {
										//Redeklarace
										SemanticError(currentLine, ER_SMC_FUNC_REDECL, funcID->name);
									}

									//Nechceme dale upravovat redeklaraci a pouzivame originalni verzi
									funcID = NULL;
								}
								else {
									//Funkce zatim nebyla deklarovana ani definovana
									funcID = InsertGlobalID(value);
									funcID->declaration = true; //Jedna se o deklaraci

									AddDeclaration(funcID, currentLine);
								}
							}
							else if (defineFunc) {
								if (declareArg) {
									//Identifikator parametru funkce
									varID = LookupID(value);
									if (varID) {
										//Dva parametry funkce se stejnym nazvem
										SemanticError(currentLine, ER_SMC_FUNC_PARAM_REDEF, funcID->name);
									}
									else if (funcID) {
										if (funcID->declaration) {
											//Pripadna neshoda poctu parametru bude
											//vypisem resena az pri ukonceni seznamu parametru
											// (T_RIGHT_BRACKET)
											paramCount++;

											//Parametry ovsem nevytvarime, pokud pocet nesedi
											if (paramCount <= (funcID->argIndex - 1)) {
												varID = InsertLocalID(value);
											}
										}
										else {
											//Neexistuje deklarace, muzeme parametry vytvaret libovolne
											varID = InsertLocalID(value);
										}
									}
								}
								else {
									funcID = LookupGlobalID(value);
									if (funcID) {
										if (funcID->declaration == false) {
											//Redefinice
											SemanticError(currentLine, ER_SMC_FUNC_REDEF, funcID->name);
										}
										else {
											//Jiz existuje deklarace, kterou timto stvrzujeme
											ConfirmDeclaration(funcID);
										}
									}
									else {
										//Definice bez existujici deklarace
										funcID = InsertGlobalID(value);
									}
								}
							}

							break;

						case T_INTEGER:
						case T_DOUBLE:
						case T_STRING:
							if (declareVar) {
								//Typ promenne
								if (varID) {
									SetSignature(varID, terminal, true);
								}
							}
							else if (funcID) {
								if (declareFunc) {
									if (declareArg) {
										//Specifikace typu parametru pri deklaraci
										SetSignature(funcID, terminal, false);
									}
									else {
										//Specifikace navratoveho typu pri deklaraci
										SetSignature(funcID, terminal, true);
									}
								}
								else if (defineFunc) {
									if (declareArg) {
										if (funcID->declaration) {
											//Je k dispozici deklarace, porovname typy parametru
											if (paramCount <= (funcID->argIndex - 1)) {
												//Muzeme porovnavat, pouze pokud parametr
												//existuje i u deklarace (pokud ne, chyba se projevi dale)
												if (!CompareSignature(funcID, terminal, paramCount)) {
													SemanticError(currentLine, ER_SMC_FUNC_PARAM_TYPE, funcID->name);
												}
											}
										}
										else {
											//Deklarace neexistuje, nastavujeme parametr automaticky
											SetSignature(funcID, terminal, false);
										}
									}
									else {
										if (funcID->declaration) {
											//Je k dispozici deklarace, porovname navratove typy
											if (!CompareSignature(funcID, terminal, 0)) {
												SemanticError(currentLine, ER_SMC_FUNC_RETURN_TYPE, funcID->name);
											}
										}
										else {
											//Deklarace neexistuje, nastavujeme navratovy typ automaticky
											SetSignature(funcID, terminal, true);
										}
									}
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
								defineFunc = !defineFunc; //Toggle
							}
							break;

						case T_LEFT_BRACKET:
							//Zacatek deklarace parametru
							declareArg = true;
							break;

						case T_RIGHT_BRACKET:
							//Konec deklarace parametru
							declareArg = false;
							if (defineFunc && funcID && funcID->declaration) {
								//Pri definici se musi pocet parametru rovnat poctu parametru v deklaraci
								if (paramCount != (funcID->argIndex - 1)) {
									SemanticError(currentLine, ER_SMC_FUNC_PARAM_COUNT, funcID->name);
								}
							}
							paramCount = 0;
							break;

						case T_END:
							if (defineFunc) {
								//Ukonceni definice funkce
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
				printf("-- Derivating\t\t\t[%s]\n", NTerminalText[nTerminal]);
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
		return false;
	}

	return true;
}


/* Interni funkce, neprobiha kontrola ukazatele */
void AddDeclaration(Identifier* function, size_t line) {
	//Automaticke sledovani velikosti a resize pole
	if (g_Decls.used == g_Decls.size) {
		g_Decls.size += CHUNK;
		Identifier** tmp;
		if ((tmp = realloc(g_Decls.array, sizeof(Identifier*) * g_Decls.size)) == NULL) {
			FatalError(ER_FATAL_INTERNAL);
		}
		g_Decls.array = tmp;
	}
	function->declaredOnLine = line;
	g_Decls.array[g_Decls.used++] = function;
}


/* Interni funkce, neprobiha kontrola ukazatele */
void ConfirmDeclaration(Identifier* function) {
	for (size_t i = 0; i < g_Decls.used;) {
		if (function == g_Decls.array[i]) {
			//Vyhodime deklaraci z pole a diru nahradime poslednim prvkem v poli
			//aby nevznikaly diry v poli a my vyuzivali pole efektivne
			g_Decls.array[i] = g_Decls.array[g_Decls.used - 1];
			g_Decls.array[g_Decls.used - 1] = NULL;
			g_Decls.used--;
		}
		else {
			i++;
		}
	}
}

/* Smazani bufferu, v budoucnu mozna dalsi akce */
void TopDownCleanup(void) {
	if (g_Decls.array) {
		free(g_Decls.array);
	}
	g_Decls.size = g_Decls.used = 0;
}
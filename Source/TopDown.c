#include <stdio.h>
#include "CompilationErrors.h"
#include "TopDown.h"
#include "Stack.h"
#include "BottomUp.h"

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
				[NT_LINE_BREAK] = "<line_break>",
				[NT_SCOPE] = "<scope>",
				[NT_ELSEIF] = "<elseif>"
};


static const char* const TerminalText[29] = {
				[T_DECLARE] = "DECLARE",
				[T_DIM] = "DIM",
				[T_DO] = "DO",
				[T_DOUBLE] = "DOUBLE",
				[T_ELSE] = "ELSE",
				[T_ELSEIF] = "ELSEIF",
				[T_END] = "END",
				[T_FUNCTION] = "FUNCTION",
				[T_IF] = "IF",
				[T_INPUT] = "INPUT",
				[T_INTEGER] = "INTEGER",
				[T_LOOP] = "LOOP",
				[T_PRINT] = "PRINT",
				[T_RETURN] = "RETURN",
				[T_SCOPE] = "SCOPE",
				[T_STRING] = "STRING",
				[T_SHARED] = "SHARED",
				[T_STATIC] = "STATIC",
				[T_COMMA] = ",",
				[T_LEFT_BRACKET] = "(",
				[T_OPERATOR_EQUAL] = "=",
				[T_EOL] = "\n",
				[T_ID] = "ID",
				[T_EOF] = "EOF",
				[T_RIGHT_BRACKET] = ")",
				[T_AS] = "AS",
				[T_WHILE] = "WHILE",
				[T_THEN] = "THEN",
				[T_SEMICOLON] = ";"
};


/* Struktura pro sledovani deklarovanych, ale nedefinovanych funkci.
 * Jedna se o semantickou chybu a deklarace nachazejici se ve strukture
 * pri vstupu do hlavniho scopu budou vypsany
 */
typedef struct Declarations {
	Function** array;
	size_t size;
	size_t used;
} Declarations;
static Declarations g_Decls;

/* Vlozi deklaraci funkce do struktury, pouzivat pri prvotni deklaraci funkce */
void AddDeclaration(Function* function);

/* Odstrani deklaraci funkce ze struktury, pouzivat pri nalezeni definice funkce */
void ConfirmDeclaration(Function* function);


bool ParseProgram(void) {
	/* Dodatecne informacni promenne */
	size_t currentLine = 1;

	/* Stavove flagy */
	bool mainScope = false;
	bool defineFunc = false;
	bool declareFunc = false;
	bool declareVar = false;
	bool declareArg = false;
	bool staticFlag = false;
	bool endFlag = false;
	size_t paramCount = 0;


	const char* value;
	Variable* variable = NULL;
	Function* function = NULL;
	Stack* stack = GetStack();
	Token* token = GetNextToken();
	TokenType tokenType;
	SymbolType symbolType;
	NTerminal nTerminal;
	Terminal terminal;
	Terminal preExp = T_WHILE;
	PushNT(stack, NT_PROGRAM);
	PushNT(stack, NT_LINE_BREAK);
	symbolType = GetSymbolType(stack);
	terminal = GetTokenTerminal(token);


	Code result = ANALYSIS_CONTINUE;
	while (result == ANALYSIS_CONTINUE) {
		switch (symbolType) {
			case SYMBOL_BOTTOM: //Dno zasobniku
				tokenType = GetTokenType(token);
				if (tokenType == TOKEN_EOF) {
					//Zasobnik je prazdny a token je EOF -> vse OK
					printf("-- Syntactically correct --\n");
					result = ANALYSIS_SUCCESS;
				}
				else {
					//Zasobnik je prazdny ale vstup neni ukoncen -> chyba
					result = ANALYSIS_ERROR;
				}
				break;

			case SYMBOL_TERMINAL: //Na vrcholu se nachazi terminal
				value = TerminalText[GetTopT(stack)];
				if (*value == '\n') {
					printf("-- Comparing terminal\t[EOL]\n");
				}
				else if (terminal == T_ID) {
					printf("-- Comparing terminal\t[%s - identifier]\n", (char*) GetTokenValue(token));
				}
				else {
					printf("-- Comparing terminal\t[%s]\n", value);
				}

				//Porovname terminal na vrcholu zasobniku s terminalem na vstupu
				if (CompareTop(stack, terminal)) {

					//Dodatecne operace podle typu terminalu
					switch (terminal) {
						case T_SCOPE:
							if (endFlag) {
								//Terminal END ve spojeni s terminalem SCOPE ukoncuje aktualni blok.
								EndSubScope();
							}
							else {
								if (defineFunc) {
									//Nachazime se ve funkci, provedeme zanoreni
									BeginSubScope();
								}
								else {
									if (!mainScope) {
										//Pri vstupu do hlavniho tela zkontrolujeme, zda byly
										// vsechny deklarovane funkce definovany
										for (size_t i = 0; i < g_Decls.used; i++) {
											Function* func = g_Decls.array[i];
											SemanticError(func->codeLine, ER_SMC_FUNC_NO_DEF, func->name);
										}
										mainScope = true;
									}
									else {
										//Jiz se nachazime v hlavnim tele, provedeme zanoreni
										BeginSubScope();
									}
								}
							}
							break;

						case T_RETURN:
							//Volani return v hlavni funkci znamena chybu
							if (mainScope) {
								result = ANALYSIS_ERROR;
								break;
							}
							preExp = T_RETURN;
							//BottomUp(currentLine, terminal);
							break;


						case T_IF:
							if (endFlag) {
								EndSubScope();
							}
							else {
								BeginSubScope();
							}
							preExp = T_IF;
							//BottomUp(currentLine, terminal);
							break;

						case T_WHILE:
						case T_PRINT:
						case T_OPERATOR_EQUAL:
							preExp = terminal;
							break;
						case T_SEMICOLON:
							preExp = T_PRINT;
							break;
						case T_ELSEIF:
							preExp = T_IF;
							break;

						case T_LOOP:
							EndSubScope();
							break;


							/* Prace s identifikatory funkci a promennych,
							 * semanticke kontroly spojene s identifikatory.
							 */
						case T_ID:
							value = GetTokenValue(token);
							if (declareVar) {
								if (mainScope || defineFunc) {
									//Deklarace lokalni promenne v hlavnim tele programu nebo
									//ve funkci. Muze prekryt globalni promenne
									variable = LookupVariable(value, true, false);
								}
								else {
									//Deklarace globalni promenne. Nachazime se na globalni
									//urovni a proto je lokalni tabulka prazdna -> nedojde ke kolizi
									variable = LookupVariable(value, true, true);
								}
								if (variable) {
									//Promenna jiz existuje (redeklarace)
									SemanticError(currentLine, ER_SMC_VAR_REDECL, variable->name);
									variable = NULL; //Nechceme dale upravovat originalni deklaraci
								}
								else {
									//Vytvorime novou promennou, pouzijeme informaci o tom, zda se
									//nachazime v hlavnim tele programu a na zaklade teto informace
									//vytvorime lokalni / globalni promennou
									variable = InsertVariable(value, (!defineFunc && !mainScope), currentLine);
									if (staticFlag) {
										variable->staticVariable = true;
										staticFlag = false;
									}
								}
							}
							else if (declareFunc && !declareArg) {
								function = LookupFunction(value);
								if (function) {
									//Chybove stavy
									if (function->declaration == false) {
										//Pozdni deklarace jiz definovane funkce
										SemanticError(currentLine, ER_SMC_FUNC_DECL_AFTER_DEF, function->name);
									}
									else {
										//Redeklarace
										SemanticError(currentLine, ER_SMC_FUNC_REDECL, function->name);
									}

									//Nechceme dale upravovat redeklaraci a pouzivame originalni verzi
									function = NULL;
								}
								else {
									//Funkce zatim nebyla deklarovana ani definovana
									function = InsertFunction(value, true, currentLine);
									AddDeclaration(function); //Jedna se o deklaraci
								}
							}
							else if (defineFunc) {
								if (declareArg) {
									//Identifikator parametru funkce, hledame pouze lokalni promenne
									variable = LookupVariable(value, true, false);
									if (variable) {
										//Dva parametry funkce se stejnym nazvem
										SemanticError(currentLine, ER_SMC_FUNC_PARAM_REDEF, function->name);
									}
									else if (function) {
										if (function->declaration) {
											//Pripadna neshoda poctu parametru bude
											//vypisem resena az pri ukonceni seznamu parametru
											// (T_RIGHT_BRACKET)
											paramCount++;

											//Parametry ovsem nevytvarime, pokud pocet nesedi
											if (paramCount <= (function->argCount)) {
												variable = InsertVariable(value, false, currentLine);
											}
										}
										else {
											//Neexistuje deklarace, muzeme parametry vytvaret libovolne
											variable = InsertVariable(value, false, currentLine);
										}
									}
								}
								else {
									function = LookupFunction(value);
									if (function) {
										if (function->declaration == false) {
											//Redefinice
											SemanticError(currentLine, ER_SMC_FUNC_REDEF, function->name);
										}
										else {
											//Jiz existuje deklarace, kterou timto stvrzujeme
											ConfirmDeclaration(function);
										}
									}
									else {
										//Definice bez existujici deklarace
										function = InsertFunction(value, false, currentLine);
									}
								}
							}

							break;




							/* Prace s datovymi typy navratovych hodnot, parametru a promennych.
							 * Semanticke kontroly spojene s datovymi typy.
							 */
						case T_INTEGER:
						case T_DOUBLE:
						case T_STRING:
							if (declareVar) {
								//Typy promennych
								if (variable) {
									//Nastavime typ promenne
									variable->type = TypeAsChar(terminal);
								}
							}
							else if (function) {
								if (declareFunc) {
									if (declareArg) {
										//Specifikace typu parametru pri deklaraci
										function->parameters[function->argCount++] = TypeAsChar(terminal);
									}
									else {
										//Specifikace navratoveho typu pri deklaraci
										function->returnType = TypeAsChar(terminal);
									}
								}
								else if (defineFunc) {
									if (declareArg) {
										if (function->declaration) {
											//Je k dispozici deklarace, porovname typy parametru
											if (paramCount <= (function->argCount)) {
												//Muzeme porovnavat, pouze pokud parametr
												//existuje i u deklarace (pokud ne, chyba se projevi dale)
												if (function->parameters[paramCount] != TypeAsChar(terminal)) {
													SemanticError(currentLine, ER_SMC_FUNC_PARAM_TYPE, function->name);
												}
											}
										}
										else {
											//Deklarace neexistuje, nastavujeme parametr automaticky
											function->parameters[function->argCount++] = TypeAsChar(terminal);
										}
									}
									else {
										if (function->declaration) {
											//Je k dispozici deklarace, porovname navratove typy
											if (function->returnType != TypeAsChar(terminal)) {
												SemanticError(currentLine, ER_SMC_FUNC_RETURN_TYPE, function->name);
											}
										}
										else {
											//Deklarace neexistuje, nastavujeme navratovy typ automaticky
											function->returnType = TypeAsChar(terminal);
										}
									}
								}
							}
							break;


						case T_STATIC:
							if (mainScope) {
								//Staticke promenne nelze deklarovat v hlavnim tele, jedna se o chybu syntaxe.
								//Pro ucely zjednoduseni gramatiky je tento problem resen podminkou
								result = ANALYSIS_ERROR;
								break;
							}
							staticFlag = true;
							//Propadnuti je zamerne
						case T_DIM:
							//Zacatek deklarace promenne
							declareVar = true;
							break;

						case T_DECLARE:
							//Zacatek deklarace funkce
							declareFunc = true;
							break;

						case T_FUNCTION:
							if (endFlag) {
								//Terminal END ve spojeni s terminalem FUNCTION ukoncuje definici funkce.
								//Provede se vycisteni lokalnich tabulek.
								defineFunc = false;
								EndScope();
							}
							else {
								if (!declareFunc) {
									//Zacatek definice funkce
									defineFunc = !defineFunc; //Toggle
								}
							}
							break;

						case T_LEFT_BRACKET:
							//Zacatek deklarace parametru
							declareArg = true;
							break;

						case T_RIGHT_BRACKET:
							//Konec deklarace parametru
							declareArg = false;
							if (defineFunc && function && function->declaration) {
								//Pri definici se musi pocet parametru rovnat poctu parametru v deklaraci
								if (paramCount != (function->argCount)) {
									SemanticError(currentLine, ER_SMC_FUNC_PARAM_COUNT, function->name);
								}
							}
							//Resetujeme pocitadlo parametru
							paramCount = 0;
							break;

						case T_END:
							endFlag = true;
							break;

						case T_EOL:
							//Konec radku ukoncuje deklarace
							declareVar = false;
							declareFunc = false;
							declareArg = false;
							endFlag = false;
							//Pocitadlo radku zdrojoveho kodu
							currentLine++;
							break;

						default:
							break;
					}

					//Popneme symbol ze zasobniku a nacteme novy token
					PopSymbol(stack);
					symbolType = GetSymbolType(stack);
					token = GetNextToken();
					terminal = GetTokenTerminal(token);
				}
				else {
					//Porovnani tokenu a vrcholu zasobniku selhalo -> syntakticka chyba
					result = ANALYSIS_ERROR;
				}
				break;

			case SYMBOL_NONTERMINAL:
				//Na vrcholu zasobniku se nachazi neterminal, pokusime se jej derivovat
				nTerminal = GetTopNT(stack);
				if (nTerminal == NT_EXPRESSION) {
					ReturnToken();
					BottomUp(currentLine, preExp);
					PopSymbol(stack);
					symbolType = GetSymbolType(stack);
					token = GetNextToken();
					terminal = GetTokenTerminal(token);
				}
				else {
					printf("-- Derivating\t\t\t[%s]\n", NTerminalText[nTerminal]);
					if (ExpandTop(stack, terminal)) {
						//Derivace uspela, ziskame typ noveho vrcholu zasobnik
						symbolType = GetSymbolType(stack);
					}
					else {
						//Derivace selhala -> neexistuje derivacni pravidlo
						result = ANALYSIS_ERROR;
					}
				}
				break;

			default:
				//Neocekavany druh symbolu, interni chyba
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
void AddDeclaration(Function* function) {
	//Automaticke sledovani velikosti a resize pole
	if (g_Decls.used == g_Decls.size) {
		g_Decls.size += CHUNK;
		Function** tmp;
		if ((tmp = realloc(g_Decls.array, sizeof(Variable*) * g_Decls.size)) == NULL) {
			FatalError(ER_FATAL_INTERNAL);
		}
		g_Decls.array = tmp;
	}

	//Ulozime deklaraci do pole pro pozdejsi kontrolu, zda byly vsechny funkce definovany
	g_Decls.array[g_Decls.used++] = function;
}


/* Interni funkce, neprobiha kontrola ukazatele */
void ConfirmDeclaration(Function* function) {
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
		g_Decls.array = NULL;
	}
	g_Decls.size = g_Decls.used = 0;
}
#include <stdio.h>
#include <string.h>
#include "CompilationErrors.h"
#include "CodeGenerator.h"
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
				[NT_ELSEIF] = "<elseif>",
				[NT_ASSIGN_OPERATOR] = "<assign_operator>"
};


static const char* const TerminalText[50] = {
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
				[T_OPERATOR_PLUS_EQ] = "+=",
				[T_OPERATOR_MINUS_EQ] = "-=",
				[T_OPERATOR_MULTIPLY_EQ] = "*=",
				[T_OPERATOR_REAL_DIVIDE_EQ]= "/=",
				[T_OPERATOR_INT_DIVIDE_EQ] = "\\=",
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
	bool funcScope = false; //Nachazime se v tele funkce
	bool mainScope = false; //Nachazime se v hlavnim tele programu
	bool defineFunc = false; //Definujeme hlavicku funkce za kterou nasleduje telo funkce
	bool declareFunc = false; //Deklarujeme hlavicku funkce
	bool declareVar = false; //Deklarujeme promennou ve funkci / hlavnim tele
	bool declareArg = false; //Deklarujeme parametr v hlavicce funkce (pri definici / deklaraci)
	bool staticFlag = false; //Bude nasledovat deklarace staticke promenne
	bool checkExprType = false; //Je potreba zkontrolovat typ vyrazu (prirazeni / return)
	bool endFlag = false; //Narazili jsme na terminal T_END
	size_t paramCount = 0; //Pomocne pocitadlo parametru funkce


	const char* value;
	Variable* variable = NULL;
	Function* function = NULL;
	Stack* stack = GetStack();
	Token* token = GetNextToken();
	TokenType tokenType;
	SymbolType symbolType;
	NTerminal nTerminal;
	Terminal terminal;
	Terminal preExpr = T_WHILE;
	Terminal exprType;
	unsigned char expansionRule;

	//Vlozeni zakladnich neterminalu na zasobnik
	PushNT(stack, NT_PROGRAM);
	PushNT(stack, NT_LINE_BREAK);
	symbolType = GetSymbolType(stack);
	terminal = GetTokenTerminal(token);
	PushToken(token);

	//Hlavni smycka syntakticke analyzy
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

				//Porovname terminal na vrcholu zasobniku s terminalem na vstupu
				if (CompareTop(stack, terminal)) {

					//Dodatecne operace podle typu terminalu
					switch (terminal) {
						case T_SCOPE:
							if (endFlag) {
								//Terminal END ve spojeni s terminalem SCOPE ukoncuje aktualni blok.
								GenerateCode();
								EndSubScope();
							}
							else {
								if (funcScope) {
									//Nachazime se ve funkci, provedeme zanoreni
									PopToken(); //Token scope jiz do bloku nepatri
									GenerateCode();
									PushToken(token); //Patri do nasledujiciho bloku
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
										PopToken(); //Token scope jiz do bloku nepatri
										GenerateCode();
										PushToken(token); //Patri do nasledujiciho bloku
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
							preExpr = T_RETURN;
							checkExprType = true;
							break;


						case T_IF:
							if (endFlag) {
								GenerateCode();
								EndSubScope();
							}
							else {
								PopToken();
								GenerateCode();
								PushToken(token);
								InsertRule(RULE_ST_IF);
								BeginSubScope();
							}
							preExpr = T_IF;
							break;


						case T_ELSEIF:
							preExpr = T_IF;
						case T_ELSE:
							//Pokud narazime na elseif nebo else, vygenerujeme kod po tento token,
							//ukoncime blok a vytvorime novy
							PopToken();
							GenerateCode();
							PushToken(token);
							EndSubScope();
							BeginSubScope();

							if (terminal == T_ELSE) {
								InsertRule(RULE_ELSE);
							}
							else {
								InsertRule(RULE_ELSEIF);
							}
							break;

						case T_OPERATOR_EQUAL:
						case T_OPERATOR_PLUS_EQ:
						case T_OPERATOR_MINUS_EQ:
						case T_OPERATOR_MULTIPLY_EQ:
						case T_OPERATOR_REAL_DIVIDE_EQ:
						case T_OPERATOR_INT_DIVIDE_EQ:
							checkExprType = true;
						case T_WHILE:
						case T_PRINT:
							preExpr = terminal;
							break;

						case T_SEMICOLON:
							preExpr = T_PRINT;
							break;

						case T_LOOP:
							GenerateCode();
							EndSubScope();
							break;

						case T_DO:
							PopToken();
							GenerateCode();
							PushToken(token);
							InsertRule(RULE_ST_WHILE);
							BeginSubScope();
							break;


							/* Prace s identifikatory funkci a promennych,
							 * semanticke kontroly spojene s identifikatory.
							 */
						case T_ID:
							value = GetTokenValue(token);
							if (declareVar) {
								if (mainScope || funcScope) {
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
									bool isGlobal = !funcScope && !mainScope;
									variable = InsertVariable(value, isGlobal, currentLine);
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


							if ((funcScope || mainScope) && !declareVar) {
								//Pristup k promenne. Je nutne zkontrolovat existenci
								variable = LookupVariable(value, false, true);
								if (variable == NULL) {
									//Neexistujici promenna
									SemanticError(currentLine, ER_SMC_VAR_UNDEF, value);
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
									variable->type = terminal;
								}
							}
							else if (function) {
								if (declareFunc) {
									if (declareArg) {
										//Specifikace typu parametru pri deklaraci
										AddParameter(function, terminal);
									}
									else {
										//Specifikace navratoveho typu pri deklaraci
										function->returnType = terminal;
									}
								}
								else if (defineFunc) {
									if (declareArg) {
										if (function->declaration) {
											//Je k dispozici deklarace, porovname typy parametru
											if (paramCount < (function->argCount)) {
												//Muzeme porovnavat, pouze pokud parametr
												//existuje i u deklarace (pokud ne, chyba se projevi dale)
												if (function->parameters[paramCount] != terminal) {
													SemanticError(currentLine, ER_SMC_FUNC_PARAM_TYPE, function->name);
												}
											}
										}
										else {
											//Deklarace neexistuje, nastavujeme parametr automaticky
											AddParameter(function, terminal);
										}
									}
									else {
										if (function->declaration) {
											//Je k dispozici deklarace, porovname navratove typy
											if (function->returnType != terminal) {
												SemanticError(currentLine, ER_SMC_FUNC_RETURN_TYPE, function->name);
											}
										}
										else {
											//Deklarace neexistuje, nastavujeme navratovy typ automaticky
											function->returnType = terminal;
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
								//Terminal END ve spojeni s terminalem FUNCTION ukoncuje telo funkce.
								//Provede se vycisteni lokalnich tabulek.
								funcScope = false;
								GenerateCode();
								EndScope();
							}
							else {
								if (!declareFunc) {
									//Zacatek definice funkce
									defineFunc = true;
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
							checkExprType = false;
							endFlag = false;
							if (defineFunc) {
								defineFunc = false;
								funcScope = true;
							}

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
					PushToken(token);
				}
				else {
					//Porovnani tokenu a vrcholu zasobniku selhalo -> syntakticka chyba
					if (*value == '\n') {
						printf("-- [EOL] comparison failed\n");
					}
					else if (terminal == T_ID) {
						printf("-- [%s - identifier] comparison failed\n", (char*) GetTokenValue(token));
					}
					else {
						printf("-- [%s] comparison failed\n", value);
					}
					result = ANALYSIS_ERROR;
				}
				break;

			case SYMBOL_NONTERMINAL:
				//Na vrcholu zasobniku se nachazi neterminal, pokusime se jej derivovat
				nTerminal = GetTopNT(stack);
				if (nTerminal == NT_EXPRESSION) {
					ReturnToken();
					PopToken();
					exprType = BottomUp(currentLine, preExpr);
					if (checkExprType) {
						if (preExpr == T_RETURN) {
							//Zkontrolujeme typ navratove hodnoty
							if (function == NULL) {
								//Interni chyba, nevim jestli k ni vubec muze dojit, nejspis ne
								fprintf(stderr, "Internal error: function reference not available!\n");
							}
							else if (exprType == T_STRING && function->returnType != T_STRING) {
								//Jazyk podporuje implicitni typove konverze mezi typy double a integer,
								//proto staci kontrolovat pouze pokud je vyraz typu string
								SemanticError(currentLine, ER_SMC_FUNC_RETURN_EXPR, function->name);
							}
						}
						else {
							//Zkontrolujeme typ vyrazu
							if (variable == NULL) {
								//Interni chyba, nevim jestli k ni vubec muze dojit, nejspis ne
								fprintf(stderr, "Internal error: variable reference not available!\n");
							}
							else if (exprType == T_STRING && variable->type != T_STRING) {
								//Jazyk podporuje implicitni typove konverze mezi typy double a integer,
								//proto staci kontrolovat pouze pokud je vyraz typu string
								SemanticError(currentLine, ER_SMC_VAR_TYPE, variable->name);
							}
						}
					}

					PopSymbol(stack);
					symbolType = GetSymbolType(stack);
					token = GetNextToken();
					terminal = GetTokenTerminal(token);
					PushToken(token);
				}
				else {
					expansionRule = ExpandTop(stack, terminal);
					if (expansionRule != RULE_MISSING) {
						//Derivace uspela, ziskame typ noveho vrcholu zasobnik
						switch (expansionRule) {
							case RULE_ST_IF:
							case RULE_ST_WHILE:
							case RULE_ELSEIF:
							case RULE_ELSE:
								break;

							default:
								InsertRule(expansionRule);
						}
						symbolType = GetSymbolType(stack);
					}
					else {
						//Derivace selhala -> neexistuje derivacni pravidlo
						printf("-- [%s] derivation failed\n", NTerminalText[nTerminal]);
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
			function->declaration = false; //Jiz se nejedna o deklaraci ale o plnohodnotnou funkci
			g_Decls.array[i] = g_Decls.array[g_Decls.used - 1];
			g_Decls.array[g_Decls.used - 1] = NULL;
			g_Decls.used--;
			return;
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
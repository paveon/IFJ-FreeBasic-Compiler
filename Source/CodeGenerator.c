#include "CodeGenerator.h"


#define RULE_CHUNK 100
#define CODE_CHUNK 1024
#define FUNC_CHUNK 10
#define TYPE_CHUNK 50
#define D_TYPE_TERM 1
#define D_TYPE_CHAR 0


typedef enum AssignOP {
	OP_EQUAL = 0,
	OP_PLUS,
	OP_MINUS,
	OP_MULTIPLY,
	OP_REAL_DIVIDE,
	OP_INT_DIVIDE,
} AssignOP;

typedef unsigned char uchar;

typedef struct Buffer {
	uchar* buffer;
	size_t size;
	size_t used;
	size_t index;
} Buffer;


typedef struct TokenArray {
	Token** array;
	size_t used;
	size_t size;
	size_t index;
} TokenArray;

typedef struct Labels {
	unsigned int* labels;
	unsigned int size;
	unsigned int count;
	unsigned int used;
} Labels;

typedef struct FuncANDTypeArray {
	union {
		const char** cArray;
		Terminal* tArray;
	} data;
	unsigned dataType;
	size_t idx;
	size_t size;
} FuncANDTypeArray;

//Operatory
const char* operators[] = {
				"=",
				"+=",
				"-=",
				"*=",
				"/=",
				"\\=",
};


void PushString(char* instruction, ...);
void PushStringData(char* instruction, ...);
char *TypeToStringForInit(Terminal type);
const char* TypeToString(Terminal type);
char *ScopeToString(bool global);
const void* FindID();
void PushWLabel (void);
unsigned int TopWLabel(void);
void PopWLabel(void);
void PushIfLabel(void);
unsigned int TopIfLabel(void);
void PopIfLabel(void);
void SetExpressionIndexesForOperators(void);
void FunctionArgumentsAdd(const char *newArgument);


void GenerateExpr(void);

void MakeArray(unsigned dataType);

void ResizeArray(unsigned dataType);

void CleanArray();


//Globalni staticka promenna pro jednodussi spravu pameti
static Buffer g_Rules;
static Buffer g_Code;
static Buffer g_DataCode;
static TokenArray g_Tokens;
static Labels g_WLabels;
static Labels g_IfLabels;
static FuncANDTypeArray g_funcField;
static FuncANDTypeArray g_typeArray;
const char ConditionVar[] = "GF@_eXpVariablE756158";
const char GlobalScopeLabel[] = "GF@_gLobalScopELabel7785";
const char TmpIOvar[] = "___tmpIOvar___";

/* Pomocne premenne pre vyrazy...asi ich bude treba viac, lebo na zaciatku neviem ake typy im dat... */
const char *ExpressionVars [] = {"GF@$ExpresioNVarNum1",
																"GF@$ExpresioNVarNum2",
																"GF@$ExpresioNVarNum3",
																"GF@$ExpresioNVarNum4",
																"GF@$ExpresioNVarNum5"
};
typedef struct FunctionArguments {
	const char **arguments;
	size_t maxsize;
	size_t used;
} FunctionArguments;

static FunctionArguments FunctionArgs;
bool isGlobal = true;



void GenerateCode(void) {
	int rule;
	int tmp_label;

	const void* value;
	Variable *var;
	bool elseWasUsed = false;
	printf("PICOVINA\n");

	if (g_Rules.used > 0) {
		printf("BLOCK START - TOKENS...size: %d, used: %d, RULES...size: %d, used: %d\n",(int)g_Tokens.size, (int)g_Tokens.used, (int)g_Rules.size, (int)g_Rules.used);
		//printf("RULES...index: %d, tTOKENS...index: %d\n", g_Rules.index, g_Tokens.index);
		for (size_t i = 0; i < g_Tokens.used; i++) {
			printf("%s ", (char*)GetTokenValue(g_Tokens.array[i])); // pomocne vypisy
		}
		putchar('\n');
		for (; g_Rules.index < g_Rules.used; g_Rules.index++) {
			rule = (int)g_Rules.buffer[g_Rules.index];
			printf ("pravidla: %d\n", rule);
			//printf ("g_Tokens.index: %d\n", g_Tokens.index);

			switch (rule) {
				case RULE_EPSILON: // 1
					break;
				case RULE_MAIN_SCOPE: // 2
					isGlobal = false;
					for (int i = 0; i < 5; i++) {
						PushStringData("DEFVAR GF@%s\n", ExpressionVars[i]); // TODO co dat do MOVE?
					}
					PushStringData("JUMP %s\n", GlobalScopeLabel);
					PushString("LABEL %s\n", GlobalScopeLabel);
					break;
				case RULE_FUNC_DECL: // 3
					while (GetTokenType(g_Tokens.array[g_Tokens.index]) != TOKEN_IDENTIFIER) g_Tokens.index++; // deklaracie nas nezaujimaju
					g_Tokens.index++;
					break;
				case RULE_FUNC_DEF: // 4
					value = FindID();
					PushString("LABEL GF@%s\n", (char*)value);
					PushString("CREATEFRAME\n");
					PushString("PUSHFRAME\n");

					break;
				case RULE_FUNC_ARG: // 7
					value = FindID();
					var = LookupVariable((const char*)value, true, false);
					FunctionArgumentsAdd((const char*)value);
					//printf("var name: %s %s, \n", ScopeToString(var->globalVariable), var->name);
					PushString("DEFVAR LF@%s_%d\n", var->name, (int)var->codeLine);
					if (g_Rules.buffer[g_Rules.index+2] != RULE_FUNC_NEXT_ARG) {
						while (FunctionArgs.used > 0) {
							var = LookupVariable(FunctionArgs.arguments[--FunctionArgs.used], true, false);
							PushString("POPS LF@%s_%d\n", var->name, var->codeLine);
							//sdadagrgsdfs
						}
					}
					break;
				case RULE_ST_LIST: // 9
					break;
				case RULE_VAR_GLOBAL: // 27 // TODO lokalna prepisuje globalnu!
					value = FindID ();
					var = LookupGlobalVariable((const char*)value);
					//sprintf (tmp, "DEFVAR GF@%s_%d\nMOVE GF@%s_%d %s\n", var->name, (int)var->codeLine, var->name, (int)var->codeLine, TypeToStringForInit(var->type));
					PushStringData("DEFVAR GF@%s_%d\nMOVE GF@%s_%d %s\n", var->name, (int)var->codeLine, var->name, (int)var->codeLine, TypeToStringForInit(var->type));
					break;

				case RULE_ST_VAR_DECL: // 10
					value = FindID ();
					var = LookupVariable((const char*)value, false, true);
					PushString("DEFVAR %s@%s_%d\nMOVE %s@%s_%d %s\n", ScopeToString(var->globalVariable), var->name,(int)var->codeLine ,
										 ScopeToString(var->globalVariable), var->name, (int)var->codeLine,TypeToStringForInit(var->type));
					break;
				case RULE_ST_VAR_STATIC: // 11
					value = FindID ();
					do {
						var = LookupVariable((const char*) value, false, true);
					} while(var != NULL && var->staticVariable == false);
					PushStringData("DEFVAR GF@%s_%d\nMOVE GF@%s_%d %s\n", var->name,(int)var->codeLine, var->name,(int)var->codeLine, TypeToStringForInit(var->type));

					break;
				case RULE_ST_INPUT: // 13
					value = FindID();
					var = LookupVariable((const char*) value, false, true);
					PushString("READ %s@%s_%d %s\n", ScopeToString(var->globalVariable), var->name, (int)var->codeLine, TypeToString(var->type)); // TODO aj bool? vid. zadanie
					break;
				case RULE_ST_PRINT: // 14
					while (GetTokenType(g_Tokens.array[g_Tokens.index]) != TOKEN_KEYWORD || strcmp (GetTokenValue(g_Tokens.array[g_Tokens.index]), "PRINT") != 0) g_Tokens.index++;
					g_Tokens.index++; //posunie na prvy token vyrazu
					g_Rules.index += 2; // preskoci na prve pravidlo vyrazu
					GenerateExpr();
					PushString("POPS GF@%s\n", TmpIOvar);
					PushString("WRITE GF@%s;\n", TmpIOvar);
					break;
				case RULE_ST_WHILE: // 15
					PushWLabel();
					PushString("LABEL LF@_wlabel_%d\n", TopWLabel());
					PushWLabel();
					g_Tokens.index += 2; // preskoci na prvy token vyrazu
					g_Rules.index += 2; // preskoci na prve pravidlo vyrazu
					GenerateExpr();
					PushString("PUSHS bool@false\n");
					PushString("JUMPIFEQS LF@_wlabel_%d\n", TopWLabel());
					/*PushString("POPS %s\n", ConditionVar);
					PushString("JUMPIFEQ LF@_wlabel_%d bool@false %s\n", TopWLabel(), ConditionVar);*/
					break;
				case RULE_ST_WHILE_END: // 35
					tmp_label = TopWLabel();
					PopWLabel();
					PushString("JUMP LF@_wlabel_%d\n", TopWLabel());
					PushString("LABEL LF@_wlabel_%d\n", tmp_label);
					PopWLabel();
					break;
				case RULE_ST_RETURN: // 16
					while (GetTokenType(g_Tokens.array[g_Tokens.index]) != TOKEN_KEYWORD ||  strcmp(GetTokenValue(g_Tokens.array[g_Tokens.index]), "RETURN") != 0) g_Tokens.index++;
					g_Tokens.index++; // preskocime token return;
					g_Rules.index += 2; // index je teraz na prvom pravidle pre vyrazy
					GenerateExpr();
					PushString("POPFRAME\n");
					PushString("RETURN\n");

					break;
				case RULE_ST_IF: // 17
					PushIfLabel();
					g_Rules.index += 2; // preskoci na prve pravidlo vyrazu
					g_Tokens.index++; // prvy token vyrazu
					GenerateExpr();
					PushString("PUSHS bool@false\n");
					PushString("JUMPIFEQS LF@_iflabel_%d\n", TopIfLabel());
					/*PushString("POPS %s\n", ConditionVar);
					PushString("JUMPIFEQ LF@_iflabel_%d bool@false %s\n", TopIfLabel(), ConditionVar);*/
					break;
				case RULE_ELSEIF: // 18
					//sprintf(tmp, "LABEL LF@_iflabel_%d\n", TopIfLabel());
					PushString("LABEL LF@_iflabel_%d\n",  TopIfLabel());
					PopIfLabel();
					PushIfLabel();
					g_Rules.index += 2; // preskoci na prve pravidlo vyrazu
					g_Tokens.index++; // prvy token vyrazu
					GenerateExpr();
					PushString("PUSHS bool@false\n");
					PushString("JUMPIFEQS LF@_iflabel_%d\n", TopIfLabel());
					/*PushString("POPS %s\n", ConditionVar);
					PushString("JUMPIFEQ LF@_iflabel_%d bool@false %s\n", TopIfLabel(), ConditionVar);*/
					break;
				case RULE_ELSE: // 19
					elseWasUsed = true;
					PushString("LABEL LF@_iflabel_%d\n", TopIfLabel());
					PopIfLabel();
					break;
				case RULE_END_IF: // 20
					if (g_IfLabels.used > 0 && !elseWasUsed) {
						PushString("LABEL LF@_iflabel_%d\n", TopIfLabel());
						PopIfLabel();
					}
					elseWasUsed = false;
					break;
				case RULE_NEXT_EXPR: // 21
					//g_Tokens.index += 2; //posunie na prvy token vyrazu
					g_Rules.index += 2; // preskoci na prve pravidlo vyrazu
					printf("token type %d, value: %d\n", GetTokenType(g_Tokens.array[g_Tokens.index]), GetTokenInt(g_Tokens.array[g_Tokens.index]));
					GenerateExpr();
					PushString("POPS GF@%s\n", TmpIOvar); // TODO pomocnu premennu
					PushString("WRITE GF@%s;\n", TmpIOvar); // TODO
					break;
				case	RULE_VAR_INIT: // 22
					SetExpressionIndexesForOperators();
					GenerateExpr();
					if (isGlobal) {
						PushStringData ("POPS GF@%s_%d\n", var->name, (int)var->codeLine);
					}
					else {
						PushString("POPS %s@%s_%d\n", ScopeToString(var->globalVariable), var->name, (int)var->codeLine);
					}
					break;
				case RULE_OP_EQ: // 29
				case RULE_OP_PLUS_EQ: // 30
				case RULE_OP_MINUS_EQ: // 31
				case RULE_OP_MULTIPLY_EQ: // 32
				case RULE_OP_INT_DIV_EQ: // 33
				case RULE_OP_REAL_DIV_EQ: // 34
					value = FindID();
					var = LookupVariable((const char*)value, false, true);
					SetExpressionIndexesForOperators();
					//printf("LADENIE: pravidlo %d, token: %d\n", g_Rules.buffer[g_Rules.index], GetTokenType(g_Tokens.array[g_Tokens.index]));
					GenerateExpr();
					PushString("POPS %s@%s_%d\n", ScopeToString(var->globalVariable), var->name, (int)var->codeLine);
					break;

				case 255:
					/*g_Rules.index++;
					while((int)g_Rules.buffer[g_Rules.index] != RULE_DELIMITER) g_Rules.index++;
					g_Rules.index++;*/
					break;
				default:
					break;

			}

		}
	}

	printf ("BLOCK END\n");

	g_Tokens.used = 0;
	g_Rules.used = 0;
	g_Tokens.index = 0;
	g_Rules.index = 0;



}


const char* TypeToString(Terminal type) {
	switch (type) {
		case T_STRING:
			return "string";
		case T_INTEGER:
			return "int";
		case T_DOUBLE:
			return "float";
		default:
			return "int";
	}
}

/************************************************************/

void GenerateExpr(void) {
	// indexy jsou -1 protoze se divame na token ktery predchazel vyrazu
	TokenType tokenType = GetTokenType(g_Tokens.array[g_Tokens.index - 1]);
	const char* tokenContent = GetTokenValue(g_Tokens.array[g_Tokens.index - 1]);

	bool isString = false;
	bool convertBack = false; // promenna zarucujici zpetne pretypovani na integer pokud je potreba
//	bool isPrint = false;
//	bool isWhile = false;
//	bool isIf = false;
	bool idRule = false; // pokud je idRule true znamena to hledani nejakeho TOKEN_IDENTIFIER v cyklu

	int intVal = 0; // ciselna hodnota TOKEN_INTEGER
	double dblVal = 0;  // ciselna hodnota TOKEN_DOUBLE

	int i = OP_EQUAL; // promenna obsahujici hodnotu prirazovace (defaultne '=')
	Variable* var = NULL;
	Function* func = NULL;

	MakeArray(D_TYPE_TERM);
	MakeArray(D_TYPE_CHAR);

	char* outField[40] = {'\0'};

	// switch podle ktereho se rozpozna token ktery predchazel nasemu tokenu
	switch (tokenType) {
		case TOKEN_OPERATOR:
			// rozpoznani o jaky prirazovac se jedna += -= apod. pole je na zacatku souboru
			for (; strcmp(tokenContent, operators[i]) != 0; i++);
			// kdyz se jednalo o operator, tak je potreba se take podivat na predesly token kvuli typu
			tokenType = GetTokenType(g_Tokens.array[g_Tokens.index - 2]);
			switch (tokenType) {
				case TOKEN_INTEGER:
					g_typeArray.data.tArray[g_typeArray.idx] = T_INTEGER;
					break;
				case TOKEN_DOUBLE:
					g_typeArray.data.tArray[g_typeArray.idx] = T_DOUBLE;
					break;
				case TOKEN_STRING:
					g_typeArray.data.tArray[g_typeArray.idx] = T_STRING;
					isString = true;
					break;
				case TOKEN_IDENTIFIER:
					var = LookupVariable(GetTokenValue(g_Tokens.array[g_Tokens.index - 2]), false, true);
					// kdyz se jedna o promennou je treba ji vlozit na zasobnik pokud byl pritomen operator
					// typu (+= -= apod.)
					if (i > OP_EQUAL) {
						if (var->globalVariable) {
							PushString("PUSHS GF@%s_%d\n", var->name, (int)var->codeLine);// TODO  PUSHS GF@name_line
						}
						else {
							PushString("PUSHS LF@%s_%d\n", var->name, (int)var->codeLine);// TODO  PUSHS LF@name_line
						}
					}
					tokenContent = GetTokenValue(g_Tokens.array[g_Tokens.index - 2]);
					var = LookupVariable(tokenContent, false, true);
					g_typeArray.data.tArray[g_typeArray.idx] = var->type;
					//printf ("LADENIE car type: %d\n", var->type);
					break;
				default:
					break;
			}
			g_typeArray.idx++;
			// po prvnim pricteni neni treba kontrolovat realokaci
			break;
//		case TOKEN_KEYWORD:
//			if(strcmp(tokenContent, "WHILE") == 0){
//				isWhile = true;
//			}
//			else if(strcmp(tokenContent, "IF") == 0){
//				isIf = true;
//			}
//			else{
//				isPrint = true;
//			}
//			break;
		default:
			break;
	}
	// cyklus vyhledavajici pravidlo 255 => konec expression pravidel
	while (g_Rules.buffer[g_Rules.index] != 255) {
		// pokud je pravidlo "vypocetni" a obsahuje cisla
		if ((g_Rules.buffer[g_Rules.index] >= ADD_RULE) &&
				(g_Rules.buffer[g_Rules.index] <= EQ_EXPR_RULE)) {
			// pokud se nerovnaji typy a nebo se jedna o nejaky typ deleni je treba pretypovat
			if ((g_Rules.buffer[g_Rules.index] == REAL_DIVIDE_RULE) ||
					(g_Rules.buffer[g_Rules.index] == INT_DIVIDE_RULE) ||
					(g_typeArray.data.tArray[g_typeArray.idx - 1] !=
					 g_typeArray.data.tArray[g_typeArray.idx - 2])) {

				// oba operandy prevedeme na float aby bylo spravne deleni
				if (g_typeArray.data.tArray[g_typeArray.idx - 1] != T_DOUBLE) {
					PushString("INT2FLOATS\n"); // TODO  INT2FLOATS
				}
				if (g_typeArray.data.tArray[g_typeArray.idx - 2] != T_DOUBLE) {
					PushString("POPS %s\n", ExpressionVars[0]); // TODO  POPS GF@1
					PushString("INT2FLOATS\n"); // TODO  INT2FLOATS
					PushString("PUSHS %s\n", ExpressionVars[0]); // TODO  PUSHS GF@1
				}
			}
		}
		switch (g_Rules.buffer[g_Rules.index]) {
			case ADD_RULE:
				PushString("ADDS\n"); // TODO  ADDS
				g_typeArray.idx--;
				break;
			case SUBTRACT_RULE:
				PushString("SUBS\n");	// TODO  SUBS
				g_typeArray.idx--;
				break;
			case MULTIPLY_RULE:
				PushString("MULS\n"); // TODO  MULS
				g_typeArray.idx--;
				break;
			case REAL_DIVIDE_RULE:
				PushString("DIVS\n"); // TODO  DIVS
				g_typeArray.idx--;
				break;
			case INT_DIVIDE_RULE:
				PushString("DIVS\n"); // TODO  DIVS
				PushString("FLOAT2INTS\n"); // TODO  FLOAT2INTS
				g_typeArray.idx--;
				break;
				// porovnavaci pravidla mohou byt stejna pro retezce i ciselne vyrazy
			case LESS_EXPR_RULE:
			case LESS_STR_RULE:
				PushString("LTS\n"); // TODO  LTS
				g_typeArray.idx--;
				break;
			case LESS_EQ_STR_RULE:
			case LESS_EQ_EXPR_RULE:
				PushString("GTS\n"); // TODO  GTS
				PushString("NOTS\n"); // TODO  NOTS
				g_typeArray.idx--;
				break;
			case GRT_STR_RULE:
			case GRT_EXPR_RULE:
				PushString("GTS\n"); // TODO  GTS
				g_typeArray.idx--;
				break;
			case GRT_EQ_STR_RULE:
			case GRT_EQ_EXPR_RULE:
				PushString("LTS\n"); // TODO  LTS
				PushString("NOTS\n"); // TODO  NOTS
				g_typeArray.idx--;
				break;
			case NOT_EQ_STR_RULE:
			case NOT_EQ_EXPR_RULE:
				PushString("EQS\n"); // TODO  EQS
				PushString("NOTS\n"); // TODO  NOTS
				g_typeArray.idx--;
				break;
			case EQ_STR_RULE:
			case EQ_EXPR_RULE:
				PushString("EQS\n"); // TODO  EQS
				g_typeArray.idx--;
				break;
				// unarni '-' je odecteni daneho cisla od 0.. je treba cislo vyndat a vlozit 0 od ktere se
				// bude odecitat.. take je treba davat pozor na typy
			case UNARY_MINUS_RULE:
				PushString("POPS %s\n", ExpressionVars[0]); // TODO  POPS GF@1
				if (g_typeArray.data.tArray[g_typeArray.idx - 1] == T_INTEGER) {
					PushString("PUSHS int@0\n"); // TODO  PUSHS int@0
				}
				else {
					PushString("PUSHS float@0.0\n"); // TODO  PUSHS float@0
				}
				PushString("PUSHS %s\n", ExpressionVars[0]); // TODO  PUSHS GF@1
				PushString("SUBS\n"); // TODO  SUBS
				break;
			case ID_RULE:
				idRule = true;
				break;
			case FUNCTION_NO_EXPR_RULE:
			case FUNCTION_STR_RULE:
			case FUNCTION_EXPR_RULE:
				// muze nastat pouze v pripade f( f( f( ... f_void()...)))
				// problem tedy nastava behem funkci bez parametru
				if (!(g_funcField.data.cArray[0])) {
					tokenType = GetTokenType(g_Tokens.array[g_Tokens.index]);
					while (tokenType != TOKEN_R_BRACKET) {
						if (tokenType == TOKEN_IDENTIFIER) {
							tokenContent = GetTokenValue(g_Tokens.array[g_Tokens.index]);
							func = LookupFunction(tokenContent);

							g_typeArray.data.tArray[g_typeArray.idx] = func->returnType;
							g_funcField.data.cArray[g_funcField.idx] = tokenContent;
							g_funcField.idx++;
							g_typeArray.idx++;
						}
						g_Tokens.index++;
						tokenType = GetTokenType(g_Tokens.array[g_Tokens.index]);
					}
				}
				g_funcField.idx--;
				PushString("CALL GF@%s\n", g_funcField.data.cArray[g_funcField.idx]);// TODO  CALL funcName => g_funcField.data.cArray[g_funcField.idx]
				g_funcField.data.cArray[g_funcField.idx] = NULL;

				g_typeArray.idx--;
				break;
			case STRING_RULE:
				idRule = true;
				break;
				// konkatenace se da provadet pouze v instanci promennych a ne na zasobniku
			case CONCATENATION_RULE:
				PushString("POPS %s\n", ExpressionVars[1]); // TODO  POPS GF@2
				PushString("POPS %s\n", ExpressionVars[0]); // TODO  POPS GF@1
				PushString("CONCAT %s %s %s\n", ExpressionVars[2], ExpressionVars[0], ExpressionVars[1]);// TODO  CONCAT GF@3 GF@1 GF@2
				PushString("PUSHS %s\n", ExpressionVars[2]); // TODO  PUSHS GF@3
			case COMMA_STR_RULE:
			case COMMA_EXPR_RULE:
			case COMMA_EXPR_STR_RULE:
			case COMMA_STR_EXPR_RULE:
				g_typeArray.idx--;
			default:
				break;
		}
		// cyklus hledajici prvni vyskyt promenne (funkce se nepocitaji, ale jejich navratove typy se
		// ukladaji pro dalsi vypocty)
		while (idRule) {
			tokenType = GetTokenType(g_Tokens.array[g_Tokens.index]);
			switch (tokenType) {
				case TOKEN_INTEGER:
					g_typeArray.data.tArray[g_typeArray.idx] = T_INTEGER;
					intVal = GetTokenInt(g_Tokens.array[g_Tokens.index]);
					PushString("PUSHS int@%d\n", intVal);// TODO  PUSHS int@%int
					idRule = false;
					g_typeArray.idx++;
					break;
				case TOKEN_DOUBLE:
					g_typeArray.data.tArray[g_typeArray.idx] = T_INTEGER;
					dblVal = GetTokenDouble(g_Tokens.array[g_Tokens.index]);
					PushString("PUSHS float@%f\n", dblVal);// TODO  PUSHS float@%dbl
					idRule = false;
					g_typeArray.idx++;
					break;
				case TOKEN_STRING:
					tokenContent = GetTokenValue(g_Tokens.array[g_Tokens.index]);
					PushString("PUSHS string@%s\n", tokenContent);// TODO  PUSHS string@str
					idRule = false;
					break;
				case TOKEN_IDENTIFIER:
					tokenContent = GetTokenValue(g_Tokens.array[g_Tokens.index]);
					var = LookupVariable(tokenContent, false, true);
					func = LookupFunction(tokenContent);
					if (func && var) {
						if (GetTokenType(g_Tokens.array[g_Tokens.index + 1]) == TOKEN_L_BRACKET) {
							g_typeArray.data.tArray[g_typeArray.idx] = func->returnType;
							g_funcField.data.cArray[g_funcField.idx] = tokenContent;
							g_funcField.idx++;
						}
						else {
							if (var->globalVariable) {
								PushString("PUSHS GF@%s_%d\n", var->name, (int)var->codeLine); // TODO  PUSHS GF@name_line
							}
							else {
								PushString("PUSHS LF@%s_%d\n", var->name, (int)var->codeLine); // TODO  PUSHS LF@name_line
							}
							g_typeArray.data.tArray[g_typeArray.idx] = var->type;
							idRule = false;
						}
					}
					else if (var) {
						if (var->globalVariable) {
							PushString("PUSHS GF@%s_%d\n", var->name, (int)var->codeLine); // TODO  PUSHS GF@name_line
						}
						else {
							PushString("PUSHS LF@%s_%d\n", var->name, (int)var->codeLine); // TODO  PUSHS LF@name_line
						}
						g_typeArray.data.tArray[g_typeArray.idx] = var->type;
						idRule = false;
					}
					else {
						g_typeArray.data.tArray[g_typeArray.idx] = func->returnType;
						g_funcField.data.cArray[g_funcField.idx] = tokenContent;
						g_funcField.idx++;
					}
					if (g_funcField.idx == g_funcField.size) {
						ResizeArray(D_TYPE_CHAR);
					}
					g_typeArray.idx++;
					break;
				default:
					break;
			}
			if (g_typeArray.idx == g_typeArray.size) {
				ResizeArray(D_TYPE_TERM);
			} // switch (tokenType)
			g_Tokens.index++;
		} // while(idRule)
		g_Rules.index++;
	} // 	while(g_Rules.buffer[g_Rules.index] != 255)

	// zaverecna podminka zda se jednalo o klasicke prirazeni ci nikoliv.. pokud ne je treba jeste
	// dalsich vypoctu
	if (i > OP_EQUAL) {
		// u stringu je mozna jen konktatenace += ...
		if (isString) {
			PushString("POPS %s\n", ExpressionVars[1]); // TODO  POPS GF@2
			PushString("POPS %s\n", ExpressionVars[0]); // TODO  POPS GF@1
			PushString("CONCAT %s %s %s\n", ExpressionVars[2], ExpressionVars[0], ExpressionVars[1]); // TODO  CONCAT GF@3 GF@1 GF@2
			PushString("PUSHS %s\n", ExpressionVars[2]);// TODO  PUSHS GF@3
			return;
		}
		// oba operandy prevedeme na float aby bylo spravne deleni
		if (g_typeArray.data.tArray[g_typeArray.idx - 1] != T_DOUBLE) {
			PushString("INT2FLOATS\n"); // TODO  INT2FLOATS
		}
		if (g_typeArray.data.tArray[g_typeArray.idx - 2] != T_DOUBLE) {
			convertBack = true;
			PushString("POPS %s\n", ExpressionVars[0]); // TODO  POPS GF@1
			PushString("INT2FLOATS\n"); // TODO  INT2FLOATS
			PushString("PUSHS %s\n", ExpressionVars[0]); // TODO  PUSHS GF@1 => ma promenna
		}
		switch (i) {
			case OP_PLUS:
				PushString("ADDS\n"); // TODO  ADDS
				break;
			case OP_MINUS:
				PushString("SUBS\n"); // TODO  SUBS
				break;
			case OP_MULTIPLY:
				PushString("MULS\n"); // TODO  MULS
				break;
			case OP_REAL_DIVIDE:
				PushString("DIVS\n"); // TODO  DIVS
				break;
			case OP_INT_DIVIDE:
				PushString("DIVS\n"); // TODO  DIVS
				PushString("FLOAT2INTS\n"); 		// TODO  FLOAT2INTS
				convertBack = false;
				break;
			default:
				break;
		}
		if (convertBack) {
			PushString("FLOAT2R2EINTS\n"); // TODO  FLOAT2R2EINTS
		}
	}
	g_typeArray.idx = 0;
	g_funcField.idx = 0;
	g_funcField.data.cArray[0] = NULL;
	// zaverecne posunuti poli tokenu a pravidel pro generovani kodu za vyrazem
	bool cycleBreak = true;
	while (cycleBreak) {
		tokenType = GetTokenType(g_Tokens.array[g_Tokens.index]);
		switch (tokenType) {
			case TOKEN_EOL:
			case TOKEN_SEMICOLON:
			case TOKEN_KEYWORD:
				cycleBreak = false;
				break;
			default:
				break;
		}
		g_Tokens.index++;
	}
	CleanArray();
}

/************************************************************/

void MakeArray(unsigned dataType) {
	if (dataType == D_TYPE_TERM) {
		g_typeArray.idx = 0;
		g_typeArray.size = TYPE_CHUNK;
		g_typeArray.dataType = dataType;
		g_typeArray.data.tArray = malloc(g_typeArray.size * sizeof(Terminal));
		if (!g_typeArray.data.tArray) {
			FatalError(ER_FATAL_ALLOCATION);
		}
		return;
	}
	g_funcField.idx = 0;
	g_funcField.size = FUNC_CHUNK;
	g_funcField.dataType = dataType;
	g_funcField.data.cArray = malloc(g_funcField.size * sizeof(char*));
	if (!g_funcField.data.cArray) {
		FatalError(ER_FATAL_ALLOCATION);
	}
	g_funcField.data.cArray[0] = NULL;
}

void ResizeArray(unsigned dataType) {
	if (dataType == D_TYPE_TERM) {
		g_typeArray.size += TYPE_CHUNK;
		Terminal* tmp = realloc(g_typeArray.data.tArray, g_typeArray.size * sizeof(Terminal));
		if (!tmp) {
			FatalError(ER_FATAL_ALLOCATION);
		}
		g_typeArray.data.tArray = tmp;
		return;
	}
	g_funcField.size += FUNC_CHUNK;
	const char** tmp = realloc(g_funcField.data.cArray, g_funcField.size * sizeof(char*));
	if (!tmp) {
		FatalError(ER_FATAL_ALLOCATION);
	}
	g_funcField.data.cArray = tmp;
}

void CleanArray(void) {
	if (g_funcField.data.cArray) {
		free(g_funcField.data.cArray);
		g_funcField.data.cArray = NULL;
	}
	if (g_typeArray.data.tArray) {
		free(g_typeArray.data.tArray);
		g_typeArray.data.tArray = NULL;
	}

}


void FunctionArgumentsAdd(const char* newArgument) {
	if (FunctionArgs.used == FunctionArgs.maxsize) {
		const char** tmp;
		if ((tmp = realloc(FunctionArgs.arguments, sizeof(const char*) * (FunctionArgs.maxsize + RULE_CHUNK))) == NULL) {
			FatalError(ER_FATAL_ALLOCATION);
		}
		FunctionArgs.maxsize *= 2;
		FunctionArgs.arguments = tmp;

	}
	FunctionArgs.arguments[FunctionArgs.used++] = newArgument;
}

/*
 * Pomocna funkcia pre preskocenie tokenov az za operator a posunutie indexu pravidiel na prve
 * pravidlo vyrazy
 */

void SetExpressionIndexesForOperators() {
	while (GetTokenType(g_Tokens.array[g_Tokens.index]) != TOKEN_OPERATOR) g_Tokens.index++;
	g_Tokens.index++; // preskoci token operator
	g_Rules.index += 2; // posunie index na prve pravidlo pre vyrazy
}


const void* FindID() {
	const void *tmp;
	while (GetTokenType(g_Tokens.array[g_Tokens.index]) != TOKEN_IDENTIFIER) (g_Tokens.index)++; // hladame identifikator
	tmp = GetTokenValue(g_Tokens.array[g_Tokens.index]);
	g_Tokens.index++; // preskocime identifikator
	return tmp;
}

char *ScopeToString(bool global) {
	if (global == true)
		return "GF";
	else
		return "LF";
}


char *TypeToStringForInit(Terminal type) {
	switch (type) {
		case T_DOUBLE:
			return "float@0.0";
		case T_INTEGER:
			return "int@0";
		case T_STRING:
			return "string@!\"\"";
		default:
			return "";
	}
}

void PushStringData(char *instruction, ...) {
	char newString[CODE_CHUNK];
	va_list arglist;
	va_start(arglist, instruction);
	vsprintf(newString, instruction, arglist);
	va_end(arglist);
	size_t length = strlen(newString);

	if (g_DataCode.size - g_DataCode.used <= length) {
		uchar* tmp;
		if ((tmp = realloc(g_DataCode.buffer, sizeof(uchar) * (g_DataCode.size + CODE_CHUNK))) == NULL) {
			FatalError(ER_FATAL_ALLOCATION);
		}
		g_DataCode.buffer = tmp;
		if (g_DataCode.size == 0) {
			sprintf((char*)g_DataCode.buffer,".IFJcode17\n");
		}
		g_DataCode.size += CODE_CHUNK;
	}


	strcat((char*)g_DataCode.buffer, newString);

	g_DataCode.used = g_DataCode.used + length;

}


void PushString(char *instruction, ...) {
	char newString[CODE_CHUNK];
	va_list arglist;
	va_start(arglist, instruction);
	vsprintf(newString, instruction, arglist);
	va_end(arglist);
	size_t length = strlen(newString);

	if (g_Code.size - g_Code.used <= length) {
		uchar* tmp;
		if ((tmp = realloc(g_Code.buffer, sizeof(uchar) * (g_Code.size + CODE_CHUNK))) == NULL) {
			FatalError(ER_FATAL_ALLOCATION);
		}
		g_Code.buffer = tmp;
		if (g_Code.size == 0) {
			memset((char*)g_Code.buffer, 0, g_Code.size + CODE_CHUNK);
			//sprintf((char*)g_Code.buffer, "");
		}
		g_Code.size += CODE_CHUNK;
	}


	strcat((char*)g_Code.buffer, newString);

	g_Code.used = g_Code.used + length;

}



void PushWLabel (void) {
	if (g_WLabels.used == g_WLabels.size) {
		//Zvetsime pole ukazatelu
		unsigned int* tmp;
		g_WLabels.size += RULE_CHUNK;
		if ((tmp = realloc(g_WLabels.labels, sizeof(unsigned int*) * g_WLabels.size)) == NULL) {
			FatalError(ER_FATAL_ALLOCATION);
		}
		g_WLabels.labels = tmp;
	}

	g_WLabels.labels[g_WLabels.used++] = g_WLabels.count;
	g_WLabels.count++;
}


unsigned int TopWLabel(void) {
	return g_WLabels.labels[g_WLabels.used-1];
}

void PopWLabel(void) {
	g_WLabels.used--;
}

void PushIfLabel (void) {
	if (g_IfLabels.used == g_IfLabels.size) {
		//Zvetsime pole ukazatelu
		unsigned int* tmp;
		g_IfLabels.size += RULE_CHUNK;
		if ((tmp = realloc(g_IfLabels.labels, sizeof(unsigned int*) * g_IfLabels.size)) == NULL) {
			FatalError(ER_FATAL_ALLOCATION);
		}
		g_IfLabels.labels = tmp;
	}

	g_IfLabels.labels[g_IfLabels.used++] = g_IfLabels.count;
	g_IfLabels.count++;
}


unsigned int TopIfLabel(void) {
	return g_IfLabels.labels[g_IfLabels.used-1];
}

void PopIfLabel(void) {
	g_IfLabels.used--;
}






void PushToken(Token* token) {
	if (g_Tokens.used == g_Tokens.size) {
		//Zvetsime pole ukazatelu
		Token** tmp;
		g_Tokens.size += RULE_CHUNK;
		if ((tmp = realloc(g_Tokens.array, sizeof(Token*) * g_Tokens.size)) == NULL) {
			FatalError(ER_FATAL_ALLOCATION);
		}
		g_Tokens.array = tmp;
	}

	g_Tokens.array[g_Tokens.used++] = token;
}

void PopToken(void) {
	if (g_Tokens.used > 0) {
		g_Tokens.used--;
	}
}


void OutputCode(void) {
	printf ("%s%s", g_DataCode.buffer, g_Code.buffer);
}


void InsertRule(size_t ruleID) {
	if (g_Rules.used == g_Rules.size) {
		uchar* newBuffer;
		g_Rules.size += RULE_CHUNK;
		if ((newBuffer = realloc(g_Rules.buffer, sizeof(uchar) * g_Rules.size)) == NULL) {
			FatalError(ER_FATAL_ALLOCATION);
		}
		g_Rules.buffer = newBuffer;
	}
	g_Rules.buffer[g_Rules.used++] = (uchar) ruleID;
}


void GeneratorCleanup(void) {
	if (g_Rules.buffer) {
		free(g_Rules.buffer);
		g_Rules.buffer = NULL;
		g_Rules.used = g_Rules.size = 0;
	}
	if (g_Code.buffer) {
		free(g_Code.buffer);
		g_Code.buffer = NULL;
		g_Code.used = g_Code.size = 0;
	}
	if (g_DataCode.buffer) {
		free(g_DataCode.buffer);
		g_DataCode.buffer = NULL;
		g_DataCode.used = g_Code.size = 0;
	}
	if (g_Tokens.array) {
		free(g_Tokens.array);
		g_Tokens.array = NULL;
		g_Tokens.used = g_Tokens.size = 0;
	}
	if (g_WLabels.labels) {
		free(g_WLabels.labels);
		g_WLabels.labels = NULL;
		g_WLabels.used = 0;
		g_WLabels.size = 0;
	}
	if (g_IfLabels.labels) {
		free(g_IfLabels.labels);
		g_IfLabels.labels = NULL;
		g_IfLabels.used = 0;
		g_IfLabels.size = 0;
	}
	if (FunctionArgs.arguments) {
		free(FunctionArgs.arguments);
		FunctionArgs.arguments = NULL;
		FunctionArgs.used = FunctionArgs.maxsize = 0;
	}
}
//
// Created by Bobek on 20.10.2017.
//

#include "BottomUp.h"
#include "CodeGenerator.h"

#define FUNC_NEST_LEAVE 100
#define FUNC_OK 1
#define T_BUFFER_CHUNK 50
#define NO_RULE_VALUE 40

static size_t nested = 0;

typedef struct TypeBufferStash {
	Terminal* allocated;
	size_t index;
	size_t size;
} TypeBufferStash;
static TypeBufferStash g_typeBufferStash;

void AllocTypeBuffer(void);
void ResizeTypeBuffer(void);

void PrecErrorCleaning(Stack* s) {
	Token* token = GetNextToken();
	if (!token) {
		ReturnToken();
		ReleaseStack(s);
		return;
	}
	TokenType tokenType = GetTokenType(token);
	while ((tokenType != TOKEN_EOL) && (tokenType != TOKEN_SEMICOLON)) {
		token = GetNextToken();
		tokenType = GetTokenType(token);
	}
	ReturnToken();
	ReleaseStack(s);
}

Terminal BottomUp(size_t lineNum, Terminal keyword) {
	Stack* stack = GetStack();
	PushT(stack, T_EOL);
	IdxTerminalPair values;
	size_t idx = 0;
	values.error = 0;
	values.cellValue = FINDING_FAILURE;
	values.incomTerm = T_UNDEFINED;
	values.rule = NO_RULE_VALUE;
	bool isCmp = false;
	int endFuncVal = 0;   // promenna obsahujici vystup z rekurzivni funkce kontroly funkci
	Terminal type = T_INTEGER;

	Token *token = NULL;

	AllocTypeBuffer();

	InsertRule(255);
	while (1) {
		FindInTable(stack, &values, lineNum, false, keyword);
		if ((values.rule >= GRT_EQ_EXPR_RULE) && (values.rule <= EQ_STR_RULE)) {
			isCmp = true;
		}
		if ((values.rule >= ADD_RULE) && (values.rule <= EQ_EXPR_RULE)) {
			// pokud je na jednom z predchozich 2 indexu pri zpracovani pravidla double, tak se ulozi
			// na pozici o 2 zpet a zaroven se index posune  o 1 zpatky
			g_typeBufferStash.index--;
			if(values.rule == INT_DIVIDE_RULE) {
				if((g_typeBufferStash.allocated[g_typeBufferStash.index] == T_DOUBLE) || (g_typeBufferStash.allocated[g_typeBufferStash.index-1] == T_DOUBLE)){
					SemanticError(lineNum, ER_SMC_INT_DIV, NULL);
					break;
				}
			}

			if((g_typeBufferStash.allocated[g_typeBufferStash.index] == T_DOUBLE) || (g_typeBufferStash.allocated[g_typeBufferStash.index-1] == T_DOUBLE)){
				g_typeBufferStash.allocated[g_typeBufferStash.index-1] = T_DOUBLE;
			}
			values.rule = NO_RULE_VALUE;
		}

		if ((values.incomTerm == T_ID) || (values.incomTerm == T_FUNCTION)) {
			g_typeBufferStash.allocated[g_typeBufferStash.index] = values.type;
			g_typeBufferStash.index++;
			if(g_typeBufferStash.index == g_typeBufferStash.size){
				ResizeTypeBuffer();
			}
		}
		/***************************************
		 *************** SEMANTIKA *************
		 ***************************************/
		if(values.rule == REAL_DIVIDE_RULE){
			type = T_DOUBLE;
		}
		if((values.type == T_DOUBLE) && (type == T_INTEGER)){
			type = T_DOUBLE;
		}
		else if((values.type == T_STRING) && (type == T_INTEGER)){
			type = T_STRING;
		}
		/**************************************************/

		if ((values.incomTerm == T_EOL) && (GetFirstTerminal(stack) == T_EOL) &&
				(LastSymBeforeFirstTerm(stack) == 1)) { // 1 protoze vysledny expr zredukovany musi mit tvar $E tedy velikost 1
			break;
		}
		if ((values.error == FINDING_FAILURE) || (values.error == EOF_FINDING_FAILURE)) {
			PrecErrorCleaning(stack);
			free(g_typeBufferStash.allocated);
			g_typeBufferStash.allocated = NULL;
			return T_UNDEFINED;
		}

		switch (values.cellValue) {
			case HIGHER_PR:
				idx = LastSymBeforeFirstTerm(stack);
				PushT(stack, values.incomTerm);
				SetReduction(stack, idx);

				ReturnToken();
				token = GetNextToken();
				PushToken(token);
				break;
			case SKIP_PR:
				PushT(stack, values.incomTerm);

				ReturnToken();
				token = GetNextToken();
				PushToken(token);
				break;
			case LOWER_PR:
				if (!ApplyPrecRule(stack, false, lineNum, &values)) {
					PrecErrorCleaning(stack);
					free(g_typeBufferStash.allocated);
					g_typeBufferStash.allocated = NULL;
					return T_UNDEFINED;
				}
				if ((values.incomTerm != T_EOL) && (values.incomTerm != T_SEMICOLON)) {
					ReturnToken();
				}
				InsertRule(values.rule);
				break;
			case EXPR_ERROR:  // nebylo pravidlo v tabulce
				SemanticError(lineNum, ER_SMC_UNKNOWN_EXPR, NULL);
				PrecErrorCleaning(stack);
				free(g_typeBufferStash.allocated);
				g_typeBufferStash.allocated = NULL;
				return T_UNDEFINED;
			default:
				break;
		}
		if (ContainingFunction(stack)) {
			endFuncVal = FuncParams(stack, values, lineNum, keyword);
			if ((endFuncVal == FUNC_NEST_LEAVE) || (endFuncVal == EOF_FINDING_FAILURE)) {
				PrecErrorCleaning(stack);
				free(g_typeBufferStash.allocated);
				g_typeBufferStash.allocated = NULL;
				return T_UNDEFINED;
			}
		}
	}

	ReleaseStack(stack);
	free(g_typeBufferStash.allocated);
	g_typeBufferStash.allocated = NULL;
	InsertRule(255);
	if (((keyword == T_WHILE) || (keyword == T_IF)) && (!isCmp)) {

	}
	printf("***************************************\n\n");
	printf("***************************************\n");
	return type;
}


int FuncParams(Stack* s, IdxTerminalPair values, size_t lineNum, Terminal keyword) {
	size_t paramCnt = values.argCnt;
	Terminal* params = values.funcParams;

	bool isInFunc = true;
	bool canCount = true;  // promenna urcujici zda muzeme pocitat prichozi zavorku (vyuziva se ReturnToken() => jedna zavorka by se mohla pocitat 2x)

	int endValue = 0;

	size_t idx = 0;
	size_t actParamCnt = 0;
	int leftBrackets = 0, rightBrackets = 0;

	const char* funcName = values.funcName; // promenna pouzivana u vypisu erroru (v jake funkci nastal)

	Token *token = NULL;

	nested++;

	while (1) {
		FindInTable(s, &values, lineNum, isInFunc, keyword);

		if ((values.rule >= ADD_RULE) && (values.rule <= EQ_EXPR_RULE)) {
			// pokud je na jednom z predchozich 2 indexu pri zpracovani pravidla double, tak se ulozi
			// na pozici o 2 zpet a zaroven se index posune  o 1 zpatky
			g_typeBufferStash.index--;
			if((g_typeBufferStash.allocated[g_typeBufferStash.index] == T_DOUBLE) ||
				 (g_typeBufferStash.allocated[g_typeBufferStash.index - 1] == T_DOUBLE)) {

				if(values.rule == INT_DIVIDE_RULE) {
					SemanticError(lineNum, ER_SMC_INT_DIV, NULL);
					break;
				}
				g_typeBufferStash.allocated[g_typeBufferStash.index-1] = T_DOUBLE;
			}
			else if(values.rule == REAL_DIVIDE_RULE){
				g_typeBufferStash.allocated[g_typeBufferStash.index-1] = T_DOUBLE;
			}
			else{
				g_typeBufferStash.allocated[g_typeBufferStash.index-1] = T_INTEGER;
			}
			values.rule = NO_RULE_VALUE; // resetovani pravidla kdyby se vracil token a pravidlo zustalo stejne...
		}

		// vlozeni typu do pole pro pripadnou kontrolu celociselneho deleni
		if ((values.incomTerm == T_ID) || (values.incomTerm == T_FUNCTION)) {
			g_typeBufferStash.allocated[g_typeBufferStash.index] = values.type;
			g_typeBufferStash.index++;
			if(g_typeBufferStash.index == g_typeBufferStash.size){
				ResizeTypeBuffer();
			}
		}

		// zacina novy argument funkce => vse se resetuje
		if((values.rule >= COMMA_EXPR_RULE) && (values.rule <= COMMA_STR_RULE)) {
			values.rule = NO_RULE_VALUE;
			g_typeBufferStash.index = 0;
		}
		if (values.error == FINDING_FAILURE) {
			break;
		}
		else if(values.error == EOF_FINDING_FAILURE){
			return EOF_FINDING_FAILURE;
		}
		// prichozi token byl identifikator, funkce a nebo retezec
		if ((values.incomTerm == T_ID) || (values.incomTerm == T_STRING) ||
				(values.incomTerm == T_FUNCTION)) {

			if (paramCnt == 0) {
				SemanticError(lineNum, ER_SMC_MANY_ARGS, funcName);
				break;
			}
			// mezi double -> int && int -> double ___ je implicitni konverze takze se
			// kontroluje pouze pokud neprichazi string a neni ocekavane neco jineho nebo naopak
			if(((values.type == T_STRING) && (params[actParamCnt] != T_STRING)) ||
				 ((values.type != T_STRING) && (params[actParamCnt] == T_STRING))) {
				SemanticError(lineNum, ER_SMC_ARG_TYPES, funcName);
				break;
			}
			// prilis mnoho parametru
			if(actParamCnt == paramCnt){
				SemanticError(lineNum, ER_SMC_MANY_ARGS, funcName);
				break;
			}
		}
		switch (values.cellValue) {
			case HIGHER_PR:
				idx = LastSymBeforeFirstTerm(s);
				PushT(s, values.incomTerm);
				SetReduction(s, idx);
				canCount = true;
				// vraceni tokenu a jeho nasledne ulozeni do pole pro generovani
				ReturnToken();
				token = GetNextToken();
				PushToken(token);
				break;
			case SKIP_PR:
				PushT(s, values.incomTerm);
				canCount = true;

				ReturnToken();
				token = GetNextToken();
				PushToken(token);
				break;
			case LOWER_PR:
				if (!ApplyPrecRule(s, isInFunc, lineNum, &values)) {
					free(g_typeBufferStash.allocated);
					return FUNC_NEST_LEAVE;
				}
				if ((values.incomTerm != T_EOL) && (values.incomTerm != T_SEMICOLON)) {
					ReturnToken();
				}
				canCount = false;
				InsertRule(values.rule);
				break;
			case EXPR_ERROR:
				SemanticError(lineNum, ER_SMC_UNKNOWN_EXPR, NULL);
				free(g_typeBufferStash.allocated);
				return FUNC_NEST_LEAVE;
			default:
				break;
		}

		/* Pocitani zavorek k zjisteni posledniho parametru */
		if ((values.incomTerm == T_LEFT_BRACKET) && canCount) {
			leftBrackets++;
		}
		else if ((values.incomTerm == T_RIGHT_BRACKET) && canCount) {
			rightBrackets++;
		}
		/*****************************************************/

		if (((values.incomTerm == T_COMMA) && canCount) ||
				(leftBrackets == rightBrackets && canCount)) {
			actParamCnt++;
			printf("func: %d, parameter: %d\n", (int) nested, (int) actParamCnt);
		}

		if (values.incomTerm == T_FUNCTION) {
			endValue = FuncParams(s, values, lineNum, keyword);
			if (endValue == FUNC_NEST_LEAVE) {
				break;
			}
		}
		if (nested > CountOfFunc(s)) {
			if ((actParamCnt != paramCnt) && (paramCnt != 0)) {
				SemanticError(lineNum, ER_SMC_LESS_ARGS, funcName);
				break;
			}
			nested--;
			return FUNC_OK;
		}
	}
	return FUNC_NEST_LEAVE;
}


void AllocTypeBuffer(void){
	if(!g_typeBufferStash.allocated){
		g_typeBufferStash.size = T_BUFFER_CHUNK;
		g_typeBufferStash.allocated = malloc(g_typeBufferStash.size* sizeof(Terminal));
		if(!g_typeBufferStash.allocated){
			FatalError(ER_FATAL_ALLOCATION);
		}
		g_typeBufferStash.index = 0;
	}
}

void ResizeTypeBuffer(){
	g_typeBufferStash.size += T_BUFFER_CHUNK;
	Terminal *tmp = realloc(g_typeBufferStash.allocated, g_typeBufferStash.size* sizeof(Terminal));
	if(!tmp){
		FatalError(ER_FATAL_ALLOCATION);
		return;
	}
	g_typeBufferStash.allocated = tmp;
}

void FreeTypeBuffer(void){
	if(g_typeBufferStash.allocated){
		free(g_typeBufferStash.allocated);
	}
}
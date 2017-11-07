//
// Created by Bobek on 20.10.2017.
//

#include "BottomUp.h"

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

Terminal BottomUp(size_t line_num, Terminal keyword) {
	Stack* stack = GetStack();
	PushT(stack, T_EOL);
	IdxTerminalPair values;
	bool return_val = true;
	size_t idx = 0;
	values.error = 0;
	values.cell_value = FINDING_FAILURE;
	values.incoming_term = T_UNDEFINED;
	values.rule = NO_RULE_VALUE; // neexistuje pravidlo 40 => pouze defaultni hodnota
	int end_func_val = 0;   // promenna obsahujici vystup z rekurzivni funkce kontroly funkci
	Terminal type = T_INTEGER;

	AllocTypeBuffer();


	while (1) {
		FindInTable(stack, &values, line_num, false, keyword);

		if((values.rule >= ADD_RULE) && (values.rule <= EQ_RULE)){
			// pokud je na jednom z predchozich 2 indexu pri zpracovani pravidla double, tak se ulozi
			// na pozici o 2 zpet a zaroven se index posune  o 1 zpatky
			g_typeBufferStash.index--;
			if(values.rule == INT_DIVIDE_RULE) {
				if((g_typeBufferStash.allocated[g_typeBufferStash.index] == T_DOUBLE) || (g_typeBufferStash.allocated[g_typeBufferStash.index-1] == T_DOUBLE)){
					SemanticError(line_num, ER_SMC_INT_DIV, NULL);
					break;
				}
			}

			if((g_typeBufferStash.allocated[g_typeBufferStash.index] == T_DOUBLE) || (g_typeBufferStash.allocated[g_typeBufferStash.index-1] == T_DOUBLE)){
				g_typeBufferStash.allocated[g_typeBufferStash.index-1] = T_DOUBLE;
			}
			values.rule = NO_RULE_VALUE;
		}

		if((values.incoming_term == T_ID) || (values.incoming_term == T_FUNCTION)){
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

		if ((values.incoming_term == T_EOL) && (GetFirstTerminal(stack) == T_EOL) &&
				(LastSymBeforeFirstTerm(stack) ==
				 1)) { // 1 protoze vysledny expr zredukovany musi mit tvar $E tedy velikost 1
			break;
		}
		if ((values.error == FINDING_FAILURE) || (values.error == EOF_FINDING_FAILURE)) {
			PrecErrorCleaning(stack);
			free(g_typeBufferStash.allocated);
			return T_UNDEFINED;
		}

		switch (values.cell_value) {
			case HIGHER_PR:
				idx = LastSymBeforeFirstTerm(stack);
				PushT(stack, values.incoming_term);
				SetReduction(stack, idx);
				break;
			case SKIP_PR:
				PushT(stack, values.incoming_term);
				break;
			case LOWER_PR:
				return_val = ApplyPrecRule(stack, false, line_num, &values);
				if (!return_val) {
					PrecErrorCleaning(stack);
					free(g_typeBufferStash.allocated);
					return T_UNDEFINED;
				}
				if ((values.incoming_term != T_EOL) && (values.incoming_term != T_SEMICOLON)) {
					ReturnToken();
				}
				break;
			case EXPR_ERROR:  // nebylo pravidlo v tabulce
				SemanticError(line_num, ER_SMC_UNKNOWN_EXPR, NULL);
				PrecErrorCleaning(stack);
				free(g_typeBufferStash.allocated);
				return T_UNDEFINED;
		}
		if (ContainingFunction(stack)) {
			end_func_val = FuncParams(stack, values, line_num, keyword);
			if ((end_func_val == FUNC_NEST_LEAVE) || (end_func_val == EOF_FINDING_FAILURE)) {
				PrecErrorCleaning(stack);
				free(g_typeBufferStash.allocated);
				return T_UNDEFINED;
			}
		}
	}

	ReleaseStack(stack);
	free(g_typeBufferStash.allocated);
	return type;
}

/*
 * TODO zkusit pak prepsat funkci celou
 */
int FuncParams(Stack* s, IdxTerminalPair values, size_t line_num, Terminal keyword) {
	size_t paramCnt = values.arg_cnt;
	Terminal *params = values.func_params;

	bool is_in_func = true;
	bool can_count = true;	// promenna urcujici zda muzeme pocitat prichozi zavorku (vyuziva se ReturnToken() => jedna zavorka by se mohla pocitat 2x)

	int end_value = 0;

	size_t idx = 0;
	size_t actParamCnt = 0;
	int l_brackets = 0, r_brackets = 0;

	const char *func_name = values.func_name;

	nested++;

	while (1) {
		FindInTable(s, &values, line_num, is_in_func, keyword);

		if((values.rule >= ADD_RULE) && (values.rule <= EQ_RULE)){
			// pokud je na jednom z predchozich 2 indexu pri zpracovani pravidla double, tak se ulozi
			// na pozici o 2 zpet a zaroven se index posune  o 1 zpatky
			g_typeBufferStash.index--;
			if(values.rule == INT_DIVIDE_RULE) {
				if((g_typeBufferStash.allocated[g_typeBufferStash.index] == T_DOUBLE) ||
								(g_typeBufferStash.allocated[g_typeBufferStash.index-1] == T_DOUBLE)){
					SemanticError(line_num, ER_SMC_INT_DIV, NULL);
					break;
				}
			}
			if((g_typeBufferStash.allocated[g_typeBufferStash.index] == T_DOUBLE) ||
							(g_typeBufferStash.allocated[g_typeBufferStash.index-1] == T_DOUBLE)){

				g_typeBufferStash.allocated[g_typeBufferStash.index-1] = T_DOUBLE;
			}
			else{
				g_typeBufferStash.allocated[g_typeBufferStash.index-1] = T_INTEGER;
			}
			values.rule = NO_RULE_VALUE;
		}

		if((values.incoming_term == T_ID) || (values.incoming_term == T_FUNCTION)){
			g_typeBufferStash.allocated[g_typeBufferStash.index] = values.type;
			g_typeBufferStash.index++;
			if(g_typeBufferStash.index == g_typeBufferStash.size){
				ResizeTypeBuffer();
			}
		}

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
		if((values.incoming_term == T_ID) || (values.incoming_term == T_STRING) ||
						(values.incoming_term == T_FUNCTION)){
			// pokud se prichozi typ nerovna ocekavanemu typu a nebo pokud neni prichozi typ integer a
			// ocekavany double(implicitni konverze), tak je chyba v parametru
			if((params[actParamCnt] != T_DOUBLE) && (values.type != T_INTEGER)){
				if(values.type != params[actParamCnt]) {
					SemanticError(line_num, ER_SMC_ARG_TYPES, func_name);
					break;
				}
			}
			if(actParamCnt == paramCnt){
				SemanticError(line_num, ER_SMC_LESS_ARGS, func_name);
				break;
			}
		}
		switch (values.cell_value) {
			case HIGHER_PR:
				idx = LastSymBeforeFirstTerm(s);
				PushT(s, values.incoming_term);
				SetReduction(s, idx);
				can_count = true;
				break;
			case SKIP_PR:
				PushT(s, values.incoming_term);
				can_count = true;
				break;
			case LOWER_PR:
				if (!ApplyPrecRule(s, is_in_func, line_num, &values)) {
					free(g_typeBufferStash.allocated);
					return FUNC_NEST_LEAVE;
				}
				if ((values.incoming_term != T_EOL) && (values.incoming_term != T_SEMICOLON)) {
					ReturnToken();
				}
				can_count = false;
				break;
			case EXPR_ERROR:
				SemanticError(line_num, ER_SMC_UNKNOWN_EXPR, NULL);
				free(g_typeBufferStash.allocated);
				return FUNC_NEST_LEAVE;
		}

		/* Pocitani zavorek k zjisteni posledniho parametru */
		if ((values.incoming_term == T_LEFT_BRACKET) && can_count) {
			l_brackets++;
		}
		else if ((values.incoming_term == T_RIGHT_BRACKET) && can_count) {
			r_brackets++;
		}
		/*****************************************************/

		if (((values.incoming_term == T_COMMA) && can_count) || (l_brackets == r_brackets && can_count)) {
			actParamCnt++;
			printf("func: %d, parameter: %d\n", (int) nested, (int) actParamCnt);
		}

		if (values.incoming_term == T_FUNCTION) {
			end_value = FuncParams(s, values, line_num, keyword);
			if (end_value == FUNC_NEST_LEAVE) {
				break;
			}
		}
		if (nested > CountOfFunc(s)) {
			if(actParamCnt != paramCnt){ // TODO mensi pocet parametru => napsat error
				SemanticError(line_num, ER_SMC_LESS_ARGS, func_name);
				break;
			}
			nested--;
			free(g_typeBufferStash.allocated);
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
			FatalError(ER_FATAL_INTERNAL);
		}
		g_typeBufferStash.index = 0;
	}
}

void ResizeTypeBuffer(){
	g_typeBufferStash.size += T_BUFFER_CHUNK;
	Terminal *tmp = realloc(g_typeBufferStash.allocated, g_typeBufferStash.size* sizeof(Terminal));
	if(!tmp){
		FatalError(ER_FATAL_INTERNAL);
		return;
	}
	g_typeBufferStash.allocated = tmp;
}

void FreeTypeBuffer(void){
	if(g_typeBufferStash.allocated){
		free(g_typeBufferStash.allocated);
	}
}
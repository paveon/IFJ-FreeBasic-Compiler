//
// Created by Bobek on 20.10.2017.
//

#include "BottomUp.h"

#define FUNC_NEST_LEAVE 100
#define FUNC_SMALL_ERROR 10
#define FUNC_OK 1

static size_t nested = 0;

void PrecErrorCleaning(Stack *s){
	Token *token = GetNextToken();
	TokenType tokenType = GetTokenType(token);
	while((tokenType != TOKEN_EOL) && (tokenType != TOKEN_SEMICOLON)) {
		token = GetNextToken();
		tokenType = GetTokenType(token);
	}
	ReturnToken();
	ReleaseStack(s);
}

bool BottomUp(size_t line_num, Terminal keyword){
	Stack *stack = GetStack();
	PushT(stack, T_EOL);
	IdxTerminalPair values;
	bool return_val = true;
	size_t idx = 0;
	values.cell_value = FINDING_FAILURE_NO_KEYWORD;
	values.incoming_term = T_UNDEFINED;
	int end_func_val = 0;

	if ((keyword != T_WHILE) && (keyword != T_IF) && (keyword != T_PRINT) &&
			(keyword != T_OPERATOR_EQUAL)) {
		SemanticError(line_num, ER_SMC_UNKNOWN_EXPR, NULL); // TODO co udelat kdyz prichozi token neni zadne z klicovych slov vyrazu
		PrecErrorCleaning(stack);
		return false;
	}
	while(1){
		if(g_err_counter >= MAX_ERR_COUNT){
			PrecErrorCleaning(stack);
			return false;
		}
		FindInTable(stack, &values, line_num, false, keyword);
//		if(values.incoming_term == T_STRING){  // TODO
//
//		}
		if((values.incoming_term == T_EOL) && (GetFirstTerminal(stack) == T_EOL) && (LastSymBeforeFirstTerm(stack) == 1)){ // 1 protoze vysledny expr zredukovany musi mit tvar $E tedy velikost 1
			break;
		}
		if(values.error == FINDING_FAILURE_SEMI){
			SemanticError(line_num, ER_SMC_UNEXPECT_SYM, ";");
			PrecErrorCleaning(stack);
			return false;
		}
		switch (values.cell_value){
			case HIGHER_PR:
				idx = LastSymBeforeFirstTerm(stack);
				PushT(stack, values.incoming_term);
				SetReduction(stack, idx);
				break;
			case SKIP_PR:
				PushT(stack, values.incoming_term);
				break;
			case LOWER_PR:
				return_val = ApplyPrecRule(stack, false, line_num);
				if(!return_val && (values.incoming_term == T_EOL) && (GetFirstTerminal(stack) == T_EOL)){
					SemanticError(line_num, ER_SMC_UNKNOWN_EXPR, NULL); // TODO co udelat kdyz prichozi token neni zadne z klicovych slov vyrazu
					PrecErrorCleaning(stack);
					return false;
				}
				if(values.incoming_term != T_EOL){
					ReturnToken();
				}
				break;
			case FINDING_FAILURE_NO_KEYWORD:
				SemanticError(line_num, ER_SMC_UNKNOWN_EXPR, NULL); // TODO co udelat kdyz prichozi token neni zadne z klicovych slov vyrazu
				PrecErrorCleaning(stack);
				return false;
			case FINDING_FAILURE_IF:
				SemanticError(line_num, ER_SMC_UNKNOWN_EXPR, NULL); // TODO co udelat kdyz prichozi token neni zadne z klicovych slov vyrazu
				PrecErrorCleaning(stack);
				return false;
			case EXPR_ERROR:
				SemanticError(line_num, ER_SMC_UNKNOWN_EXPR, NULL); // TODO co udelat kdyz prichozi token neni zadne z klicovych slov vyrazu
				PrecErrorCleaning(stack);
				return false;
		}
		if(ContainingFunction(stack)){
			end_func_val = FuncParams(stack, values, line_num, keyword);
			if(end_func_val == FUNC_NEST_LEAVE){
				PrecErrorCleaning(stack);
				return false;
			}
		}
	}

	ReleaseStack(stack);
	return return_val;
}


/*
 * TODO Nejake returny jdou upravit na FUNC_SMALL_ERROR a pokracovat v prohlizeni
 */
int FuncParams(Stack *s, IdxTerminalPair values, size_t line_num, Terminal keyword){
	size_t parametres_count = 0;
	int end_value = 0;
	bool is_in_func = true;
	size_t idx = 0;
	bool return_val = true;
	nested++;

	while (1) {
		if (g_err_counter >= MAX_ERR_COUNT) {
			PrecErrorCleaning(s);
			return FUNC_NEST_LEAVE;
		}
		FindInTable(s, &values, line_num, is_in_func, keyword);
		if(values.error == FINDING_FAILURE_SEMI){
			SemanticError(line_num, ER_SMC_UNEXPECT_SYM, ";");
			return FUNC_NEST_LEAVE;
		}
		switch (values.cell_value) {
			case HIGHER_PR:
				idx = LastSymBeforeFirstTerm(s);
				PushT(s, values.incoming_term);
				SetReduction(s, idx);
				break;
			case SKIP_PR:
				PushT(s, values.incoming_term);
				break;
			case LOWER_PR:
				return_val = ApplyPrecRule(s, is_in_func, line_num);
				if (!return_val && (values.incoming_term == T_EOL) && (GetFirstTerminal(s) == T_EOL)) {
					SemanticError(line_num, ER_SMC_UNKNOWN_EXPR, NULL); // TODO co udelat kdyz prichozi token neni zadne z klicovych slov vyrazu
					return FUNC_NEST_LEAVE;
				}
				break;
			case FINDING_FAILURE_NO_KEYWORD:
				SemanticError(line_num, ER_SMC_UNKNOWN_EXPR, NULL); // TODO co udelat kdyz prichozi token neni zadne z klicovych slov vyrazu
				return FUNC_NEST_LEAVE;
			case FINDING_FAILURE_SEMI:
				SemanticError(line_num, ER_SMC_UNKNOWN_EXPR, NULL); // TODO co udelat kdyz prichozi token neni zadne z klicovych slov vyrazu
				return FUNC_NEST_LEAVE;
			case FINDING_FAILURE_IF:
				SemanticError(line_num, ER_SMC_UNKNOWN_EXPR, NULL); // TODO co udelat kdyz prichozi token neni zadne z klicovych slov vyrazu
				return FUNC_NEST_LEAVE;
			case EXPR_ERROR:
				SemanticError(line_num, ER_SMC_UNKNOWN_EXPR, NULL); // TODO co udelat kdyz prichozi token neni zadne z klicovych slov vyrazu
				return FUNC_NEST_LEAVE;
		}
		if((values.incoming_term == T_COMMA) || (values.incoming_term == T_RIGHT_BRACKET)){ // TODO kontrola parametru funkce
			parametres_count++;
		}
		if (values.incoming_term == T_FUNCTION) {
			end_value = FuncParams(s, values, line_num, keyword);
			if(end_value == FUNC_NEST_LEAVE){
				return FUNC_NEST_LEAVE;
			}
		}
		if(nested > CountOfFunc(s)){
			return FUNC_OK;
		}
	}
}

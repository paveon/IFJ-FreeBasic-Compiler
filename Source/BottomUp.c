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

bool BottomUp(size_t line_num){
	Stack *stack = GetStack();
	PushT(stack, T_EOL);
	IdxTerminalPair value_pair;
	bool return_val = true;
	size_t idx = 0;
	value_pair.cell_value = FINDING_FAILURE;
	value_pair.incoming_term = T_UNDEFINED;
	int end_func_val = 0;

	while(1){
		if(g_err_counter >= MAX_ERR_COUNT){
			PrecErrorCleaning(stack);
			return false;
		}
		FindInTable(stack, &value_pair, line_num, false);
		if((value_pair.incoming_term == T_EOL) && (GetFirstTerminal(stack) == T_EOL) && (LastSymBeforeFirstTerm(stack) == 1)){ // 1 protoze vysledny expr zredukovany musi mit tvar $E tedy velikost 1
			break;
		}
		switch (value_pair.cell_value){
			case HIGHER_PR:
				idx = LastSymBeforeFirstTerm(stack);
				PushT(stack, value_pair.incoming_term);
				SetReduction(stack, idx);
				break;
			case SKIP_PR:
				PushT(stack, value_pair.incoming_term);
				break;
			case LOWER_PR:
				return_val = ApplyPrecRule(stack, false, line_num);
				if(!return_val && (value_pair.incoming_term == T_EOL) && (GetFirstTerminal(stack) == T_EOL)){
					SemanticError(line_num, ER_SMC_UNKNOWN_EXPR, NULL); // TODO co udelat kdyz prichozi token neni zadne z klicovych slov vyrazu
					PrecErrorCleaning(stack);
					return false;
				}
				break;
			case FINDING_FAILURE:
				SemanticError(line_num, ER_SMC_UNKNOWN_EXPR, NULL); // TODO co udelat kdyz prichozi token neni zadne z klicovych slov vyrazu
				PrecErrorCleaning(stack);
				return false;
			case EXPR_ERROR:
				SemanticError(line_num, ER_SMC_UNKNOWN_EXPR, NULL); // TODO co udelat kdyz prichozi token neni zadne z klicovych slov vyrazu
				PrecErrorCleaning(stack);
				return false;
		}
		if(ContainingFunction(stack)){
			end_func_val = FuncParams(stack, value_pair, line_num);
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
int FuncParams(Stack *s, IdxTerminalPair value_pair, size_t line_num){
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
		FindInTable(s, &value_pair, line_num, is_in_func);
		switch (value_pair.cell_value) {
			case HIGHER_PR:
				idx = LastSymBeforeFirstTerm(s);
				PushT(s, value_pair.incoming_term);
				SetReduction(s, idx);
				break;
			case SKIP_PR:
				PushT(s, value_pair.incoming_term);
				break;
			case LOWER_PR:
				return_val = ApplyPrecRule(s, is_in_func, line_num);
				if (!return_val && (value_pair.incoming_term == T_EOL) &&
				    (GetFirstTerminal(s) == T_EOL)) {
					SemanticError(line_num, ER_SMC_UNKNOWN_EXPR, NULL); // TODO co udelat kdyz prichozi token neni zadne z klicovych slov vyrazu
					PrecErrorCleaning(s);
					return FUNC_NEST_LEAVE;
				}
				break;
			case FINDING_FAILURE:
				SemanticError(line_num, ER_SMC_UNKNOWN_EXPR, NULL); // TODO co udelat kdyz prichozi token neni zadne z klicovych slov vyrazu
				PrecErrorCleaning(s);
				return FUNC_NEST_LEAVE;
			case EXPR_ERROR:
				SemanticError(line_num, ER_SMC_UNKNOWN_EXPR, NULL); // TODO co udelat kdyz prichozi token neni zadne z klicovych slov vyrazu
				PrecErrorCleaning(s);
				return FUNC_NEST_LEAVE;
		}
		if((value_pair.incoming_term == T_COMMA) || (value_pair.incoming_term == T_RIGHT_BRACKET)){ // TODO kontrola parametru funkce
			parametres_count++;
		}
		if (value_pair.incoming_term == T_FUNCTION) {
			end_value = FuncParams(s, value_pair, line_num);
			if(end_value == FUNC_NEST_LEAVE){
				return FUNC_NEST_LEAVE;
			}
		}
		if(nested > CountOfFunc(s)){
			return FUNC_OK;
		}
	}
}

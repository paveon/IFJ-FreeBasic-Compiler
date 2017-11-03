//
// Created by Bobek on 20.10.2017.
//

#include "BottomUp.h"

#define FUNC_NEST_LEAVE 100
#define FUNC_OK 1
#define COMMA_RULES 16

static size_t nested = 0;

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
	int end_func_val = 0;   // promenna obsahujici vystup z rekurzivni funkce kontroly funkci
	Terminal type = T_INTEGER;

	while (1) {
		FindInTable(stack, &values, line_num, false, keyword);
		if((values.type == T_DOUBLE) && (type == T_INTEGER)){
			type = T_DOUBLE;
		}
		else if((values.type == T_STRING) && (type == T_INTEGER)){
			type = T_STRING;
		}
		if ((values.incoming_term == T_EOL) && (GetFirstTerminal(stack) == T_EOL) &&
				(LastSymBeforeFirstTerm(stack) ==
				 1)) { // 1 protoze vysledny expr zredukovany musi mit tvar $E tedy velikost 1
			break;
		}
		if (values.error == FINDING_FAILURE) {
			PrecErrorCleaning(stack);
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
					return T_UNDEFINED;
				}
				if ((values.incoming_term != T_EOL) && (values.incoming_term != T_SEMICOLON)) {
					ReturnToken();
				}
				break;
			case EXPR_ERROR:  // nebylo pravidlo v tabulce
				SemanticError(line_num, ER_SMC_UNKNOWN_EXPR, NULL);
				PrecErrorCleaning(stack);
				return T_UNDEFINED;
		}
		if (ContainingFunction(stack)) {
			end_func_val = FuncParams(stack, values, line_num, keyword);
			if (end_func_val == FUNC_NEST_LEAVE) {
				PrecErrorCleaning(stack);
				return T_UNDEFINED;
			}
		}
	}

	ReleaseStack(stack);
	return type;
}

/*
 * TODO popremyslet o lepsim zpusobu pocitani parametru
 * TODO dopsat semanticke chyby
 */
int FuncParams(Stack* s, IdxTerminalPair values, size_t line_num, Terminal keyword) {
	size_t parametres_count = 0;
	int end_value = 0;
	bool is_in_func = true;
	size_t idx = 0;
	bool return_val;
	bool can_count = true;	// promenna urcujici zda muzeme pocitat prichozi zavorku (vyuziva se ReturnToken() => jedna zavorka by se mohla pocitat 2x)
	int l_brackets = 0, r_brackets = 0;
	Terminal *params = values.func_params;
	nested++;

	while (1) {
		FindInTable(s, &values, line_num, is_in_func, keyword);
		if (values.error == FINDING_FAILURE) {
			return FUNC_NEST_LEAVE;
		}
		if((values.incoming_term == T_ID) || (values.incoming_term == T_STRING) || (values.incoming_term == T_FUNCTION)){
			if(values.type != params[parametres_count]){
				return FUNC_NEST_LEAVE;
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
				return_val = ApplyPrecRule(s, is_in_func, line_num, &values);
				if (!return_val) {
					return FUNC_NEST_LEAVE;
				}
				if ((values.incoming_term != T_EOL) && (values.incoming_term != T_SEMICOLON)) {
					ReturnToken();
				}
				can_count = false;
				break;
			case EXPR_ERROR:
				SemanticError(line_num, ER_SMC_UNKNOWN_EXPR, NULL);
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

		if (((values.incoming_term == T_COMMA) && can_count) || (l_brackets == r_brackets && can_count)) { // TODO kontrola parametru funkce
			parametres_count++;
			printf("func: %d, parameter: %d\n", (int) nested, (int) parametres_count);
		}

		if (values.incoming_term == T_FUNCTION) {
			end_value = FuncParams(s, values, line_num, keyword);
			if (end_value == FUNC_NEST_LEAVE) {
				return FUNC_NEST_LEAVE;
			}
		}
		if (nested > CountOfFunc(s)) {
			nested--;
			return FUNC_OK;
		}
	}
}

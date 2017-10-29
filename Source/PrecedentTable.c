//
// Created by rober on 15.10.2017.
//

#include "PrecedentTable.h"
//
// Created by Bobek on 14.10.2017.
//

#include "PrecedentTable.h"
#include <string.h>

#define PREC_TABLE_SIZE 17
#define NUM_OF_OPERATORS 11
#define TERMINAL_OPERATOR_SHIFT 27
#define NO_NEXT_VAL_FOR_RULE 0
#define NUM_OF_RULES 16
#define RULE_ELEMENTS 4

const char* const OperatorField[NUM_OF_OPERATORS] = {"+", "-", "*", "/", "\\", "<", "<=", ">", ">=",
																										 "<>", "="};

//TODO kontrolovat z vnejsi funkce
int g_err_counter = 0; // promenna ve ktere se pocita pocet erroru pri zpracovavani vyrazu, pokud presahne hodnotu MAX_ERR_COUNT automaticky se vyraz vyhodnoti jako chybny a preskakuje se

// g_PrecedentTable[11][15] se muze rovnat i 3 (carka ve volani funkce)
static unsigned char g_PrecedentTable[PREC_TABLE_SIZE][PREC_TABLE_SIZE] = {
				{1, 1, 3, 3, 3, 1, 1, 1, 1, 1, 1, 3, 1, 3, 3, 1, 1},
				{1, 1, 3, 3, 3, 1, 1, 1, 1, 1, 1, 3, 1, 3, 3, 1, 1},
				{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 1, 3, 3, 1, 1},
				{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 1, 3, 3, 1, 1},
				{1, 1, 3, 3, 1, 1, 1, 1, 1, 1, 1, 3, 1, 3, 3, 1, 1},
				{3, 3, 3, 3, 3, 1, 1, 1, 1, 1, 1, 3, 1, 3, 3, 0, 1},
				{3, 3, 3, 3, 3, 1, 1, 1, 1, 1, 1, 3, 1, 3, 3, 0, 1},
				{3, 3, 3, 3, 3, 1, 1, 1, 1, 1, 1, 3, 1, 3, 3, 0, 1},
				{3, 3, 3, 3, 3, 1, 1, 1, 1, 1, 1, 3, 1, 3, 3, 0, 1},
				{3, 3, 3, 3, 3, 1, 1, 1, 1, 1, 1, 3, 1, 3, 3, 0, 1},
				{3, 3, 3, 3, 3, 1, 1, 1, 1, 1, 1, 3, 1, 3, 3, 0, 1},
				{3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 3, 3, 0, 0},
				{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 0, 1, 1},
				{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 0, 1, 1},
				{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0},
				{3, 3, 3, 3, 3, 0, 0, 0, 0, 0, 0, 3, 2, 3, 3, 1, 0},
				{3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 0, 3, 3, 0, 0},
};

static unsigned char g_PrecedentRules[NUM_OF_RULES][RULE_ELEMENTS] = {
				{NT_EXPRESSION, T_OPERATOR_PLUS, NT_EXPRESSION, NO_NEXT_VAL_FOR_RULE},
				{NT_EXPRESSION, T_OPERATOR_MINUS, NT_EXPRESSION, NO_NEXT_VAL_FOR_RULE},
				{NT_EXPRESSION, T_OPERATOR_MULTIPLY, NT_EXPRESSION, NO_NEXT_VAL_FOR_RULE},
				{NT_EXPRESSION, T_OPERATOR_REAL_DIVIDE, NT_EXPRESSION, NO_NEXT_VAL_FOR_RULE},
				{NT_EXPRESSION, T_OPERATOR_INT_DIVIDE, NT_EXPRESSION, NO_NEXT_VAL_FOR_RULE},
				{NT_EXPRESSION, T_OPERATOR_GRT, NT_EXPRESSION, NO_NEXT_VAL_FOR_RULE},
				{NT_EXPRESSION, T_OPERATOR_GRT_EQ, NT_EXPRESSION, NO_NEXT_VAL_FOR_RULE},
				{NT_EXPRESSION, T_OPERATOR_LESS, NT_EXPRESSION, NO_NEXT_VAL_FOR_RULE},
				{NT_EXPRESSION, T_OPERATOR_LESS_EQ, NT_EXPRESSION, NO_NEXT_VAL_FOR_RULE},
				{NT_EXPRESSION, T_OPERATOR_NOT_EQ, NT_EXPRESSION, NO_NEXT_VAL_FOR_RULE},
				{NT_EXPRESSION, T_OPERATOR_EQUAL, NT_EXPRESSION, NO_NEXT_VAL_FOR_RULE},
				{T_OPERATOR_MINUS, NT_EXPRESSION, NO_NEXT_VAL_FOR_RULE, NO_NEXT_VAL_FOR_RULE},
				{T_ID, NO_NEXT_VAL_FOR_RULE, NO_NEXT_VAL_FOR_RULE, NO_NEXT_VAL_FOR_RULE},
				{T_LEFT_BRACKET, NT_EXPRESSION, T_RIGHT_BRACKET, NO_NEXT_VAL_FOR_RULE},
				{T_FUNCTION, T_LEFT_BRACKET, NT_EXPRESSION, T_RIGHT_BRACKET},
				{NT_EXPRESSION, T_COMMA, NT_EXPRESSION, NO_NEXT_VAL_FOR_RULE},
};

bool ApplyPrecRule(Stack* s, bool is_in_func, size_t line_num) {
	SymbolType top_type = GetSymbolType(s);
	int buff_idx = 0;
	int buffer[10] = {NO_NEXT_VAL_FOR_RULE};
	int i = 0, j = 0;
	int main_counter = NUM_OF_RULES;
	// Pokud neni program ve funkci, tak se zpracovava pouze 15 pravidel
	if (!is_in_func) {
		main_counter--;
	}

	while (!IsEndOfReduction(
					s)) { // TODO otestovat zda se nemuze stat ze bude pouze ukonceni redukce bez zacatku
		if (top_type == SYMBOL_NONTERMINAL) {
			buffer[buff_idx] = GetTopNT(s);
		}
		else {
			buffer[buff_idx] = GetTopT(s);
		}
		buff_idx++;
		PopSymbol(s);
	}
	if (top_type == SYMBOL_NONTERMINAL) {
		buffer[buff_idx] = GetTopNT(s);
	}
	else {
		buffer[buff_idx] = GetTopT(s);
	}
	PopSymbol(s);

	for (; i < main_counter; i++) {
		for (; j < RULE_ELEMENTS; j++) {
			if (buffer[j] != g_PrecedentRules[i][j]) {
				break;
			}
		}
		if (j == RULE_ELEMENTS) {
			PushNT(s, NT_EXPRESSION);
			return true;
		}
	}
	if ((buffer[0] == NT_EXPRESSION) && (buffer[1] == NT_EXPRESSION)) {
		SemanticError(line_num, ER_SMC_MISSING_OP, NULL);
	}
	else {
		SemanticError(line_num, ER_SMC_UNKNOWN_EXPR, NULL); // TODO nevim jestli muze nastat
	}
	PushNT(s, NT_EXPRESSION);
	g_err_counter++;
	return false;
}


void
FindInTable(Stack* s, IdxTerminalPair* field, size_t line_num, bool is_in_func, Terminal keyword) {
	/* Promenne k pomocnym vypoctum hodnot prichoziho tokenu */
	Token* token = GetNextToken();
	TokenType tokenType = GetTokenType(token);
	const char* token_val = GetTokenValue(token);
	Function* id_func;
	Variable* id_id;
	unsigned char column, row;
	unsigned char op_field_idx;
	Terminal termType = GetFirstTerminal(s);

	// Switch obsahujici rozdeleni tokenu.
	// Z TOKEN_IDENTIFIER zjisti zda se jedna o funkci ci promennou
	// Z TOKEN_OPERATOR vybere spravny operator
	// Vse ulozi do pole poslaneho parametrem na index INCOMING_TERMINAL
	switch (tokenType) {
		case TOKEN_COMMA:
			column = OPERATOR_COMMA;
			field->incoming_term = T_COMMA;
			break;
		case TOKEN_IDENTIFIER:
			column = IDENTIFIER;
			field->incoming_term = T_ID;
//			id_func = LookupFunction(token_val);
//			id_id = LookupVariable(token_val, true);
//			if(id_func && id_id){
//				token = GetNextToken();
//				tokenType = GetTokenType(token);
//				if(tokenType == TOKEN_L_BRACKET){
//					column = FUNCTION_IDENTIFIER;
//					field->incoming_term = T_FUNCTION;
//				}
//				else{
//					column = IDENTIFIER;
//					field->incoming_term = T_ID;
//				}
//				ReturnToken();
//			}
//			else{
//				if(!id_func){ // neni to funkce
//					if(!id_id){ // neni to promenna ani funkce
//						token = GetNextToken();
//						tokenType = GetTokenType(token);
//						if(tokenType == TOKEN_L_BRACKET){ // pokud je nasledujici token leva zavorka, predpoklada se ze mel nasledovat zapis funkce
//							if(GetTrailSpace(token)){
//								field->error = FINDING_FAILURE_FUNC_SPACE;
//							}
//							else{
//								column = FUNCTION_IDENTIFIER; // TODO napsat error jelikoz nezname pocet parametru
//								field->incoming_term = T_FUNCTION;
//							}
//						}
//						else{ // pokud nenasledovala zavorka asi to bude klasicka promenna
//							column = IDENTIFIER;
//							field->incoming_term = T_ID;
//						}
//						ReturnToken();
//						SemanticError(line_num, ER_SMC_VAR_UNDEF, NULL);
//					}
//					else{ // neni funkce ale je promenna
//						column = IDENTIFIER;
//						field->incoming_term = T_ID;
//					}
//				}
//				else{ // je to funkce
//					if(GetTrailSpace(token)){
//						field->error = FINDING_FAILURE_FUNC_SPACE;
//					}
//					else{
//						column = FUNCTION_IDENTIFIER; // TODO napsat error jelikoz nezname pocet parametru
//						field->incoming_term = T_FUNCTION;
//					}
//				}
//			}
			break;
		case TOKEN_INTEGER:
			column = IDENTIFIER;
			field->incoming_term = T_ID;
			break;
		case TOKEN_DOUBLE:
			column = IDENTIFIER;
			field->incoming_term = T_ID;
			break;
//		case TOKEN_STRING:   // TODO
//			field->incoming_term = T_STRING;
//			return;
		case TOKEN_L_BRACKET:
			column = OPERATOR_L_BRACKET;
			field->incoming_term = T_LEFT_BRACKET;
			break;
		case TOKEN_R_BRACKET:
			column = OPERATOR_R_BRACKET;
			field->incoming_term = T_RIGHT_BRACKET;
			break;
		case TOKEN_OPERATOR:
			for (op_field_idx = 0; (op_field_idx < NUM_OF_OPERATORS) &&
														 (strcmp(token_val, OperatorField[op_field_idx]) != 0); op_field_idx++);
			if ((keyword != T_IF) && (keyword != T_WHILE) && (op_field_idx > 4)) {
				field->cell_value = FINDING_FAILURE_IF;
				return;
			}
			column = op_field_idx;
			field->incoming_term = TERMINAL_OPERATOR_SHIFT +
														 op_field_idx;   // pomoci shiftu a indexu v operatorech vlozi index spravneho terminalu operatoru do zasobniku
			break;
		case TOKEN_SEMICOLON:
			if (keyword != T_PRINT) {
				field->error = FINDING_FAILURE_SEMI;
			}
			column = END_SYMBOL;
			field->incoming_term = T_EOL;   // nehraje roli zda prijde ';' nebo EOL, oba ukoncuji expression a kontrola ukoncovaciho znaku neni na expression syntax kontrole
			ReturnToken();
			break;
		case TOKEN_EOL:
			column = END_SYMBOL;
			field->incoming_term = T_EOL;
			ReturnToken();
			break;
		default:
			field->cell_value = FINDING_FAILURE_NO_KEYWORD;
			g_err_counter++;
			return;
	}
	// Druhy switch rozdelujici pouze podle nejvrchnejsiho terminalu zasobniku
	switch (termType) {
		case T_COMMA:
			row = OPERATOR_COMMA;
			break;
		case T_FUNCTION:
			row = FUNCTION_IDENTIFIER;
			break;
		case T_ID:
			row = IDENTIFIER;
			break;
		case T_LEFT_BRACKET:
			row = OPERATOR_L_BRACKET;
			break;
		case T_RIGHT_BRACKET:
			row = OPERATOR_R_BRACKET;
			break;
		case T_OPERATOR_PLUS:
			row = OPERATOR_PLUS;
			break;
		case T_OPERATOR_MINUS:
			row = OPERATOR_MINUS;
			break;
		case T_OPERATOR_MULTIPLY:
			row = OPERATOR_MULTIPLY;
			break;
		case T_OPERATOR_REAL_DIVIDE:
			row = OPERATOR_REAL_DIVIDE;
			break;
		case T_OPERATOR_INT_DIVIDE:
			row = OPERATOR_INT_DIVIDE;
			break;
		case T_OPERATOR_LESS:
			row = OPERATOR_LESS;
			break;
		case T_OPERATOR_LESS_EQ:
			row = OPERATOR_LESS_EQ;
			break;
		case T_OPERATOR_GRT:
			row = OPERATOR_GRT;
			break;
		case T_OPERATOR_GRT_EQ:
			row = OPERATOR_GRT_EQ;
			break;
		case T_OPERATOR_NOT_EQ:
			row = OPERATOR_NOT_EQ;
			break;
		case T_OPERATOR_EQUAL:
			row = OPERATOR_EQ;
			break;
		case T_EOL:
			row = END_SYMBOL;
			break;
		case T_SEMICOLON :
			row = END_SYMBOL;
			break;
		default:
			break;
	}
	// Pokud se zadavaji parametry funkce je nutne zmenit pravidlo bunky g_PrecedentTable[11][15] na LOWER_PR(1)
	if (is_in_func) {
		field->cell_value = LOWER_PR;
		return;
	}
	field->cell_value = g_PrecedentTable[row][column];
}

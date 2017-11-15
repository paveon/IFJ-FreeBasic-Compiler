//
// Created by rober on 15.10.2017.
//

#include "PrecedentTable.h"
//
// Created by Bobek on 14.10.2017.
//

#include "PrecedentTable.h"
#include <string.h>

#define PREC_TABLE_SIZE 18
#define NUM_OF_OPERATORS 11
#define NO_NEXT_VAL_FOR_RULE 0
#define NUM_OF_RULES 30
#define RULE_ELEMENTS 4
#define CONDITION_OP_SKIP 4

const char* const OperatorField[NUM_OF_OPERATORS] = {"+", "-", "*", "/", "\\", "<", "<=", ">", ">=",
																										 "<>", "="};

// g_PrecedentTable[11][15] se muze rovnat i 3 (carka ve volani funkce)
static unsigned char g_PrecedentTable[PREC_TABLE_SIZE][PREC_TABLE_SIZE] = {
				// +  -  *  /  \  <  <= >  >= <> =  (  )  i  f  ,  s  $
				{1, 1, 3, 3, 3, 1, 1, 1, 1, 1, 1, 3, 1, 3, 3, 1, 3, 1}, // +
				{1, 1, 3, 3, 3, 1, 1, 1, 1, 1, 1, 3, 1, 3, 3, 1, 3, 1}, // -
				{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 1, 3, 3, 1, 3, 1}, // *
				{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 1, 3, 3, 1, 3, 1}, // /
				{1, 1, 3, 3, 1, 1, 1, 1, 1, 1, 1, 3, 1, 3, 3, 1, 3, 1}, // '\'
				{3, 3, 3, 3, 3, 1, 1, 1, 1, 1, 1, 3, 1, 3, 3, 0, 3, 1}, // <
				{3, 3, 3, 3, 3, 1, 1, 1, 1, 1, 1, 3, 1, 3, 3, 0, 3, 1}, // <=
				{3, 3, 3, 3, 3, 1, 1, 1, 1, 1, 1, 3, 1, 3, 3, 0, 3, 1}, // >
				{3, 3, 3, 3, 3, 1, 1, 1, 1, 1, 1, 3, 1, 3, 3, 0, 3, 1}, // >=
				{3, 3, 3, 3, 3, 1, 1, 1, 1, 1, 1, 3, 1, 3, 3, 0, 3, 1}, // <>
				{3, 3, 3, 3, 3, 1, 1, 1, 1, 1, 1, 3, 1, 3, 3, 0, 3, 1}, // =
				{3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 3, 3, 0, 3, 0}, // (
				{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 0, 1, 0, 1}, // )
				{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 0, 1, 0, 1}, // i
				{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0}, // f
				{3, 3, 3, 3, 3, 0, 0, 0, 0, 0, 0, 3, 1, 3, 3, 1, 3, 0}, // ,
				{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 0, 1, 0, 1}, // s
				{3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 0, 3, 3, 0, 3, 0}, // $
};

const char *const pole_err[] = {
				"plus",
				"minus",
				"krat",
				"deleno real",
				"deleno int",
				"vetsi vyraz",
				"vetsi rovno vyraz",
				"mensi vyraz",
				"mensi rovno vyraz",
				"nerovno vyraz",
				"rovno vyraz",
				"vetsi retezec",
				"vetsi rovno retezec",
				"mensi retezec",
				"mensi rovno retezec",
				"nerovno retezec",
				"rovno retezec",
				"unarni minus",
				"id",
				"zavorky vyraz",
				"zavorky retezec",
				"funkce no expr",
				"funkce expr",
				"funkce str",
				"retezec",
				"konkatenace",
				"parametry (carka) expr",
				"parametry (carka) expr str",
				"parametry (carka) str expr",
				"parametry (carka) str",
};

static unsigned char g_PrecedentRules[NUM_OF_RULES][RULE_ELEMENTS] = {
				{NT_EXPRESSION,   T_OPERATOR_PLUS,        NT_EXPRESSION,  NO_NEXT_VAL_FOR_RULE}, //0
				{NT_EXPRESSION,   T_OPERATOR_MINUS,       NT_EXPRESSION,  NO_NEXT_VAL_FOR_RULE}, //1
				{NT_EXPRESSION,   T_OPERATOR_MULTIPLY,    NT_EXPRESSION,  NO_NEXT_VAL_FOR_RULE}, //2
				{NT_EXPRESSION,   T_OPERATOR_REAL_DIVIDE, NT_EXPRESSION,  NO_NEXT_VAL_FOR_RULE}, //3
				{NT_EXPRESSION,   T_OPERATOR_INT_DIVIDE,  NT_EXPRESSION,  NO_NEXT_VAL_FOR_RULE}, //4
				{NT_EXPRESSION,   T_OPERATOR_GRT,         NT_EXPRESSION,  NO_NEXT_VAL_FOR_RULE}, //5
				{NT_EXPRESSION,   T_OPERATOR_GRT_EQ,      NT_EXPRESSION,  NO_NEXT_VAL_FOR_RULE}, //6
				{NT_EXPRESSION,   T_OPERATOR_LESS,        NT_EXPRESSION,  NO_NEXT_VAL_FOR_RULE}, //7
				{NT_EXPRESSION,   T_OPERATOR_LESS_EQ,     NT_EXPRESSION,  NO_NEXT_VAL_FOR_RULE}, //8
				{NT_EXPRESSION,   T_OPERATOR_NOT_EQ,      NT_EXPRESSION,  NO_NEXT_VAL_FOR_RULE}, //9
				{NT_EXPRESSION,   T_OPERATOR_EQUAL,       NT_EXPRESSION,  NO_NEXT_VAL_FOR_RULE}, //10
				{NT_STRING,       T_OPERATOR_GRT,         NT_STRING,      NO_NEXT_VAL_FOR_RULE}, //11
				{NT_STRING,       T_OPERATOR_GRT_EQ,      NT_STRING,      NO_NEXT_VAL_FOR_RULE}, //12
				{NT_STRING,       T_OPERATOR_LESS,        NT_STRING,      NO_NEXT_VAL_FOR_RULE}, //13
				{NT_STRING,       T_OPERATOR_LESS_EQ,     NT_STRING,      NO_NEXT_VAL_FOR_RULE}, //14
				{NT_STRING,       T_OPERATOR_NOT_EQ,      NT_STRING,      NO_NEXT_VAL_FOR_RULE}, //15
				{NT_STRING,       T_OPERATOR_EQUAL,       NT_STRING,      NO_NEXT_VAL_FOR_RULE}, //16
				{NT_EXPRESSION,   T_OPERATOR_MINUS, NO_NEXT_VAL_FOR_RULE, NO_NEXT_VAL_FOR_RULE}, //17
				{T_ID,     NO_NEXT_VAL_FOR_RULE,    NO_NEXT_VAL_FOR_RULE, NO_NEXT_VAL_FOR_RULE}, //18
				{T_RIGHT_BRACKET, NT_EXPRESSION,          T_LEFT_BRACKET, NO_NEXT_VAL_FOR_RULE}, //19
				{T_RIGHT_BRACKET, NT_STRING,              T_LEFT_BRACKET, NO_NEXT_VAL_FOR_RULE}, //20
				{T_RIGHT_BRACKET, T_LEFT_BRACKET,         T_FUNCTION,     NO_NEXT_VAL_FOR_RULE}, //21
				{T_RIGHT_BRACKET, NT_EXPRESSION,          T_LEFT_BRACKET, T_FUNCTION}, //22
				{T_RIGHT_BRACKET, NT_STRING,              T_LEFT_BRACKET, T_FUNCTION}, //23
				{T_STRING, NO_NEXT_VAL_FOR_RULE,    NO_NEXT_VAL_FOR_RULE, NO_NEXT_VAL_FOR_RULE}, //24
				{NT_STRING,       T_OPERATOR_PLUS,        NT_STRING,      NO_NEXT_VAL_FOR_RULE}, //25
				{NT_EXPRESSION,   T_COMMA,                NT_EXPRESSION,  NO_NEXT_VAL_FOR_RULE}, //26
				{NT_EXPRESSION,   T_COMMA,                NT_STRING,      NO_NEXT_VAL_FOR_RULE}, //27
				{NT_STRING,       T_COMMA,                NT_EXPRESSION,  NO_NEXT_VAL_FOR_RULE}, //28
				{NT_STRING,       T_COMMA,                NT_STRING,      NO_NEXT_VAL_FOR_RULE}, //29
};

bool ApplyPrecRule(Stack* s, bool isInFunc, size_t lineNum, IdxTerminalPair* field) {
	SymbolType topType = GetSymbolType(s);
	int buffIdx = 0;
	int buffer[4] = {NO_NEXT_VAL_FOR_RULE};
	unsigned char i = 0, j = 0;
	int mainCntr = NUM_OF_RULES;
	bool isString = false;
	// Pokud neni program ve funkci, tak se zpracovava pouze 17 pravidel
	if (!isInFunc) {
		mainCntr -= 4;
	}

	while (!IsEndOfReduction(s)) {
		if (buffIdx > 3) {
			SemanticError(lineNum, ER_SMC_UNKNOWN_EXPR, NULL);
			return false;
		}
		if (topType == SYMBOL_NONTERMINAL) {
			buffer[buffIdx] = GetTopNT(s);
			if (buffer[buffIdx] == NT_STRING) {
				isString = true;
			}
		}
		else {
			buffer[buffIdx] = GetTopT(s);
			if (buffer[buffIdx] == T_STRING) {
				isString = true;
			}
		}
		buffIdx++;
		PopSymbol(s);
	}
	if (topType == SYMBOL_NONTERMINAL) {
		buffer[buffIdx] = GetTopNT(s);
		if (buffer[buffIdx] == NT_STRING) {
			isString = true;
		}
	}
	else {
		buffer[buffIdx] = GetTopT(s);
		if (buffer[buffIdx] == T_STRING) {
			isString = true;
		}
	}
	PopSymbol(s);

	for (; i < mainCntr; i++) {
		for (j = 0; j < RULE_ELEMENTS; j++) {
			if (buffer[j] != g_PrecedentRules[i][j]) {
				break;
			}
		}
		if (j == RULE_ELEMENTS) {
			if (isString) {
				// reseni pouze u carky z duvodu mozne konkatenace v parametrech
				if(buffer[1] == T_COMMA){
					PushNT(s, NT_EXPRESSION);
				}
				else{
					PushNT(s, NT_STRING);
				}
			}
			else {
				PushNT(s, NT_EXPRESSION);
			}
			printf("***********Rule %s was used********\n", pole_err[i]);
			field->rule = i;
			return true;
		}
	}
	// String 'operator' Exptression
	if (((buffer[0] == NT_STRING) || (buffer[1] == NT_STRING) || (buffer[2] == NT_STRING)) &&
			((buffer[0] == NT_EXPRESSION) || (buffer[1] == NT_EXPRESSION) ||
			 (buffer[2] == NT_EXPRESSION))) {

		SemanticError(lineNum, ER_SMC_STR_AND_NUM, NULL);
	}
		// Missing operator
	else if (((buffer[0] == NT_EXPRESSION) && (buffer[1] == NT_EXPRESSION)) ||
					 ((buffer[1] == NT_EXPRESSION) && (buffer[2] == NT_EXPRESSION))) {
		SemanticError(lineNum, ER_SMC_MISSING_OP,
									NULL); // TODO momentalne nemuzes nastat kvuli hodnotam v tabulce
	}
	else {
		SemanticError(lineNum, ER_SMC_UNKNOWN_EXPR, NULL);
	}
	return false;
}


void
FindInTable(Stack* s, IdxTerminalPair* field, size_t lineNum, bool isInFunc, Terminal keyword) {
	/* Promenne k pomocnym vypoctum hodnot prichoziho tokenu */
	Token* token = GetNextToken();
	TokenType tokenType = GetTokenType(token);
	const char* tokenVal = GetTokenValue(token);
	Function* idFunc;
	Variable* idVar;
	unsigned char column = 0, row = 0;
	unsigned char opFieldIdx;
	Terminal termType = GetFirstTerminal(s);
	bool unaryMinus = false;

	// Switch obsahujici rozdeleni tokenu.
	// Z TOKEN_IDENTIFIER zjisti zda se jedna o funkci ci promennou
	// Z TOKEN_OPERATOR vybere spravny operator
	// Vse ulozi do pole poslaneho parametrem na index INCOMING_TERMINAL
	switch (tokenType) {
		case TOKEN_COMMA:
			column = OPERATOR_COMMA;
			field->incomTerm = T_COMMA;
			break;
		case TOKEN_IDENTIFIER:
			column = IDENTIFIER;
			field->incomTerm = T_ID;
			idFunc = LookupFunction(tokenVal);
			idVar = LookupVariable(tokenVal, false, true);
			if (idFunc && idVar) { // muze to byt promenna i funkce
				token = GetNextToken();
				tokenType = GetTokenType(token);
				if(tokenType == TOKEN_L_BRACKET){ // objevila se leva zavorka => pravdepodobne se jedna o funkci
					if(GetTrailSpace(token)){ // po leve zavorce se objevila mezera => chybi operator a nebo chybna mezera (pocitame s chybnou mezerou)
						SemanticError(lineNum, ER_SMC_UNEXP_FUNC_SPACE, tokenVal);
						field->error = FINDING_FAILURE;
						ReturnToken();
						return;
					}
					// jedna se o funkci
					column = FUNCTION_IDENTIFIER;
					field->type = idFunc->returnType;
					field->funcParams = idFunc->parameters;
					field->funcName = idFunc->name;
					field->argCnt = idFunc->argCount;
					field->incomTerm = T_FUNCTION;
				}
				// pokud predchozi if neplatil, tak zustava poprve nastavena hodnota teda 'column = IDENTIFIER' a 'field->incomTerm = T_ID'
				field->type = idVar->type;
				ReturnToken();
			}
			else{
				if (!idFunc) { // neni to funkce
					if (!idVar) { // neni to promenna ani funkce
						token = GetNextToken();
						tokenType = GetTokenType(token);
						if(tokenType == TOKEN_L_BRACKET){ // pokud je nasledujici token leva zavorka, predpoklada se ze mel nasledovat zapis funkce
							if(GetTrailSpace(token)){ // kdyz se objevi mezera po zavorce znovu se muze jednat o chybnou mezeru a nebo chybejici operator (pocitame s chybnou mezerou)
								SemanticError(lineNum, ER_SMC_UNEXP_FUNC_SPACE,
															tokenVal); // error znacici chybne zapsanou mezeru (nepripustnou pri zapisu funkce)
							}
							SemanticError(lineNum, ER_SMC_FUNC_UNDECL, tokenVal);
						}
						else{ // pokud nenasledovala zavorka asi to bude klasicka promenna
							SemanticError(lineNum, ER_SMC_VAR_UNDEF, tokenVal);
						}
						field->error = FINDING_FAILURE;
						ReturnToken(); // kdyby byl nasledujici znak EOL nebo ';' uz by se to nikdo nedozvedel
						return;
					}
					// nenasleduje else{} protoze defaultni hodnoty jsou 'column = IDENTIFIER' a 'field->incomTerm = T_ID'
					field->type = idVar->type;
				}
				else{ // je to funkce
					if(GetTrailSpace(token)){
						SemanticError(lineNum, ER_SMC_UNEXP_FUNC_SPACE, tokenVal);
						field->error = FINDING_FAILURE;
						return;
					}
					else{
						column = FUNCTION_IDENTIFIER;
						field->type = idFunc->returnType;
						field->funcParams = idFunc->parameters;
						field->funcName = idFunc->name;
						field->argCnt = idFunc->argCount;
						field->incomTerm = T_FUNCTION;
					}
				}
			}
			break;
		case TOKEN_INTEGER:
			column = IDENTIFIER;
			field->type = T_INTEGER;
			field->incomTerm = T_ID;
			break;
		case TOKEN_DOUBLE:
			column = IDENTIFIER;
			field->type = T_DOUBLE;
			field->incomTerm = T_ID;
			break;
		case TOKEN_STRING:
			column = STRING;
			field->type = T_STRING;
			field->incomTerm = T_STRING;
			break;
		case TOKEN_L_BRACKET:
			column = OPERATOR_L_BRACKET;
			field->incomTerm = T_LEFT_BRACKET;
			break;
		case TOKEN_R_BRACKET:
			column = OPERATOR_R_BRACKET;
			field->incomTerm = T_RIGHT_BRACKET;
			break;
		case TOKEN_OPERATOR:
			for (opFieldIdx = 0; (opFieldIdx < NUM_OF_OPERATORS) &&
													 (strcmp(tokenVal, OperatorField[opFieldIdx]) != 0); opFieldIdx++);
			if ((keyword != T_IF) && (keyword != T_WHILE) && (opFieldIdx > CONDITION_OP_SKIP)) {
				SemanticError(lineNum, ER_SMC_COMPARATIVE_EXPR, NULL);
				field->error = FINDING_FAILURE;
				return;
			}
			// podminka resici pokud je na zasobniku minus a po nem zacatek a nebo leva zavorka, tak
			// je minus unarni
			if ((GetFirstTerminal(s) == T_OPERATOR_MINUS) && ((field->preTerm == T_LEFT_BRACKET)
																												|| (field->preTerm == T_EOL) ||
																												((field->preTerm >= T_OPERATOR_LESS) &&
																												 (field->preTerm <= T_OPERATOR_NOT_EQ)) ||
																												(field->preTerm == T_OPERATOR_EQUAL))) {
				unaryMinus = true;
			}

			// pomoci shiftu a indexu v operatorech vlozi index spravneho terminalu operatoru do zasobniku
			column = opFieldIdx;
			switch (opFieldIdx) {
				case OPERATOR_PLUS:
					field->incomTerm = T_OPERATOR_PLUS;
					break;
				case OPERATOR_MINUS:
					field->incomTerm = T_OPERATOR_MINUS;
					break;
				case OPERATOR_MULTIPLY:
					field->incomTerm = T_OPERATOR_MULTIPLY;
					break;
				case OPERATOR_REAL_DIVIDE:
					field->incomTerm = T_OPERATOR_REAL_DIVIDE;
					break;
				case OPERATOR_INT_DIVIDE:
					field->incomTerm = T_OPERATOR_INT_DIVIDE;
					break;
				case OPERATOR_LESS:
					field->incomTerm = T_OPERATOR_LESS;
					break;
				case OPERATOR_LESS_EQ:
					field->incomTerm = T_OPERATOR_LESS_EQ;
					break;
				case OPERATOR_GRT:
					field->incomTerm = T_OPERATOR_GRT;
					break;
				case OPERATOR_GRT_EQ:
					field->incomTerm = T_OPERATOR_GRT_EQ;
					break;
				case OPERATOR_NOT_EQ:
					field->incomTerm = T_OPERATOR_NOT_EQ;
					break;
				case OPERATOR_EQ:
					field->incomTerm = T_OPERATOR_EQUAL;
					break;
				default:
					break;
			}
			Terminal tmp = GetSymbolOneDown(s);
			if ((tmp >= T_OPERATOR_MULTIPLY) && (tmp <= T_OPERATOR_INT_DIVIDE) &&
					(field->incomTerm == T_OPERATOR_MINUS)) {
				field->cellValue = HIGHER_PR;
				return;
			}
			// prichozi terminal je minus => do preTerm se zapise symbol pod znakem '-' =>
			// T_UNDEFINED pokud je zde neterminal a pokud je terminal tak prislusny terminal
			if (field->incomTerm == T_OPERATOR_MINUS) {
				field->preTerm = GetSymbolOneDown(s);
			}
			break;
		case TOKEN_SEMICOLON:
			if (keyword != T_PRINT) {
				SemanticError(lineNum, ER_SMC_UNEXPECT_SYM, ";");
				field->error = FINDING_FAILURE;
			}
			column = END_SYMBOL;
			field->incomTerm = T_EOL;
			ReturnToken();
			break;
		case TOKEN_KEYWORD:
			if (keyword != T_IF || GetTokenTerminal(token) != T_THEN) {
				SemanticError(lineNum, ER_SMC_UNEXPECT_SYM, tokenVal);
				field->error = FINDING_FAILURE;
			}
			column = END_SYMBOL;
			field->incomTerm = T_EOL;
			ReturnToken();
			break;
		case TOKEN_EOL:
			if (keyword == T_PRINT) {
				SemanticError(lineNum, ER_SMC_UNEXPECT_SYM, "EOL");
				field->error = FINDING_FAILURE;
			}
			column = END_SYMBOL;
			field->incomTerm = T_EOL;
			ReturnToken();
			break;
		default:
			if (tokenType == TOKEN_EOF) {
				ReturnToken();
				field->error = EOF_FINDING_FAILURE;
				return;
			}
			else if (tokenType == TOKEN_UNDEFINED) {
				//TODO dopsat lexikalni error
			}
			SemanticError(lineNum, ER_SMC_UNEXPECT_SYM, tokenVal);
			field->error = FINDING_FAILURE;
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
		case T_STRING:
			row = STRING;
			break;
		default:
			break;
	}
	// Pokud se zadavaji parametry funkce je nutne zmenit pravidlo bunky g_PrecedentTable[11][15] na HIGHER_PR(3)
	if (isInFunc && (row == OPERATOR_L_BRACKET) && (column == OPERATOR_COMMA)) {
		field->cellValue = HIGHER_PR;
		return;
	}
	field->cellValue = g_PrecedentTable[row][column];

	if (unaryMinus && (field->cellValue != EXPR_ERROR)) {
		field->cellValue = LOWER_PR;
	}
}

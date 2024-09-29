#ifndef _PARSER_H_
#define _PARSER_H_

#include <string>
#include <vector>
#include "lexer.h"
#include "inputbuf.h"

void syntax_error();
Token expect(TokenType);
void parse_program();
void parse_var_section();
void parse_id_list();
void parse_body();
void parse_stmt_list();
void parse_stmt();
InstructionNode* parse_assign_stmt();
void parse_expr();
std::string parse_primary();
ArithmeticOperatorType parse_op();
InstructionNode* parse_output_stmt();
InstructionNode* parse_input_stmt();
std::vector<int> getInputs();
void parse_while_stmt();
void parse_if_stmt();
void parse_condition();
void parse_relop();
void parse_switch_stmt();
void parse_for_stmt();
void parse_case_list();
void parse_case();
void parse_default_case();
void parse_inputs();
void parse_num_list();


struct InstructionNode * parse_generate_intermediate_representation();

#endif 
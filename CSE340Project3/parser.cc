#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <cctype>
#include <cstring>
#include <string>
#include "compiler.h"
#include "lexer.h"
#include "parser.h"
#include "inputbuf.h"
#include <iostream>
#include <unordered_map>

using namespace std;
LexicalAnalyzer lexer;

int nextLocation = 0;
unordered_map<string, int> locationTable; // Location table: variable name -> memory location
//vector<int> memory; // store values of each variable in different memory location
//vector<int> inputsList; // store values of inputs at end

InstructionNode* first_instruction;
InstructionNode* last_instruction;



// Function to allocate memory locations to variables
void allocate_memory_location(const string& variable, int value) {
   // If the variable already exists, update its value
    if (locationTable.find(variable) != locationTable.end()) {
        mem[locationTable[variable]] = value;
    } else {
        // Allocate a new memory location for the variable
        int location = nextLocation++;
        locationTable[variable] = location;
        mem[location] = value;
    }
    
    //locationTable[variable] = nextLocation++;
}

// Function to get memory location for a variable
int get_memory_location(const string& variable) {
    return locationTable[variable];
}

void link_instruction_node(InstructionNode* new_node) {
    if (last_instruction != nullptr) {
        // If there is a previous node, link it to the new node
        last_instruction->next = new_node;
    } else {
        // If there is no previous node, the new node becomes the first node in the linked list
        first_instruction = new_node;
    }
    // Update the last instruction node pointer to the new node
    last_instruction = new_node;

}

// Function to create an assignment instruction node
InstructionNode* create_assign_instruction(int lhs_index, int op1_index, int op2_index, ArithmeticOperatorType op) {
    InstructionNode* node = new InstructionNode;
    node->type = ASSIGN;
    node->assign_inst.left_hand_side_index = lhs_index;
    node->assign_inst.operand1_index = op1_index;
    node->assign_inst.operand2_index = op2_index;
    node->assign_inst.op = op;
    node->next = nullptr;
    return node;
}

// Function to create an input instruction node
InstructionNode* create_input_instruction(int var_index) {
    InstructionNode* node = new InstructionNode;
    node->type = IN;
    node->input_inst.var_index = var_index;
    node->next = nullptr;
    return node;
}

// Function to create an output instruction node
InstructionNode* create_output_instruction(int var_index) {
    InstructionNode* node = new InstructionNode;
    node->type = OUT;
    node->output_inst.var_index = var_index;
    node->next = nullptr;
    return node;
}

// Function to create a conditional jump instruction node
InstructionNode* create_cjmp_instruction(ConditionalOperatorType op, int op1_index, int op2_index, InstructionNode* target) {
    InstructionNode* node = new InstructionNode;
    node->type = CJMP;
    node->cjmp_inst.condition_op = op;
    node->cjmp_inst.operand1_index = op1_index;
    node->cjmp_inst.operand2_index = op2_index;
    node->cjmp_inst.target = target;
    node->next = nullptr;
    return node;
}

// Function to create an unconditional jump instruction node
InstructionNode* create_jmp_instruction(InstructionNode* target) {
    InstructionNode* node = new InstructionNode;
    node->type = JMP;
    node->jmp_inst.target = target;
    node->next = nullptr;
    return node;
}

vector<string> temp; //all non-terminals on first pass

vector<string> terminals;
vector<string> nonTerminals;

//grammar rule has a non terminal on the LHS and a list of strings on the RHS
struct grammarRule {
    string LHS;
    vector<string> RHS;
};

void syntax_error()
{
	// outputs error message and exits the program
	//cout << "SYNTAX ERROR !!!\n";
	exit(0);
}

// function that returns a token if it matches the expected type
Token expect(TokenType expected)
{
	Token t = lexer.GetToken();
	if (t.token_type != expected)
	{
		//cout << "expect syntax";
		syntax_error();

	}
	return t;
}

//program → var_section body inputs
void parse_program(){
    parse_var_section();
    parse_body();
    parse_inputs();
}

// Function to parse var section
//var_section → id_list SEMICOLON
void parse_var_section() {
    // Parse id list
    parse_id_list();
    expect(SEMICOLON);
    //cout << "SEMICOLON ";
    // Allocate memory locations to variables
}

//id_list → ID COMMA id_list | ID
void parse_id_list(){
    Token temp = expect(ID);
    allocate_memory_location(temp.lexeme, 0);
    //cout << temp.lexeme << " ";
    Token t = lexer.peek(1);
    // first definition
	if (t.token_type == COMMA)
	{
        //rule.RHS.push_back(t.lexeme);
        expect(COMMA);
        //cout << "COMMA ";

        parse_id_list();
		return;
	}
    
    // second definition
    else if (t.token_type == SEMICOLON) {
        return;
    }
    else {
        // syntax error
		syntax_error();
    }

}

//body → LBRACE stmt_list RBRACE
void parse_body(){
    expect(LBRACE);
    //cout << "LBRACE ";
    parse_stmt_list();
    expect(RBRACE);
    //cout << "RBRACE ";
}

// stmt_list → stmt stmt_list | stmt
void parse_stmt_list(){
    parse_stmt();
    Token t = lexer.peek(1);
    if(t.token_type == RBRACE){
        return;
    }else if(t.token_type == ID || t.token_type == WHILE || t.token_type == IF || t.token_type == SWITCH || 
            t.token_type == FOR || t.token_type == OUTPUT || t.token_type == INPUT){
        parse_stmt_list();
        return;
    }else{
        syntax_error();
    }
}

std::string op1;
std::string op2;
ArithmeticOperatorType op;

//stmt → assign_stmt | while_stmt | if_stmt | switch_stmt | for_stmt
//stmt → output_stmt | input_stmt
void parse_stmt(){

    Token t = lexer.peek(1);
    //assign_stmt
    if (t.token_type == ID || t.token_type == NUM)
	{
        InstructionNode* node = new InstructionNode;
        node = parse_assign_stmt();
        //cout << node->assign_inst.left_hand_side_index << " = " << node->assign_inst.operand1_index << ";\n";
        link_instruction_node(node);
		return;
	}
    //while_stmt
    else if(t.token_type == WHILE){
        parse_while_stmt();
        return;
    }
    //if_stmt
    else if(t.token_type == IF){
        parse_if_stmt();
        return;
    }
    //switch_stmt
    else if(t.token_type == SWITCH){
        parse_switch_stmt();
        return;
    }
    //for_stmt
    else if(t.token_type == FOR){
        parse_for_stmt();
        return;
    }
    //output_stmt
    else if(t.token_type == OUTPUT){
        InstructionNode* node = new InstructionNode;
        node = parse_output_stmt();
        link_instruction_node(node);
        return;
    }
    //input_stmt
    else if(t.token_type == INPUT){
        InstructionNode* node = new InstructionNode;
        node = parse_input_stmt();
        link_instruction_node(node);
        return;
    }else{
        syntax_error();
    }


}

//assign stmt → ID EQUAL primary SEMICOLON
//assign stmt → ID EQUAL expr SEMICOLON
InstructionNode* parse_assign_stmt(){

    Token temp = expect(ID);
    allocate_memory_location(temp.lexeme, 0);
    //cout << temp.lexeme << " ";
    expect(EQUAL);
    //cout << "EQUAL ";
    Token t = lexer.peek(2);
    //first def
    if (t.token_type == SEMICOLON)
	{
        std::string RHS = parse_primary();
        //cout << "RHS: " << RHS;
        expect(SEMICOLON);
        //cout << "SEMICOLON ";

        int LHS_index = get_memory_location(temp.lexeme); // Get the memory location of the variable
        int RHS_index = get_memory_location(RHS);

        //create_assign_instruction(int lhs_index, int op1_index, int op2_index, ArithmeticOperatorType op)
        InstructionNode* assign_node = create_assign_instruction(LHS_index, RHS_index, -1, OPERATOR_NONE);
        //cout << "Assign Node: " << assign_node->assign_inst.left_hand_side_index << " = " << assign_node->assign_inst.operand1_index << ";\n";

		return assign_node;
	}
    //second def
    else if (t.token_type == PLUS || t.token_type == MINUS || t.token_type == MULT || t.token_type == DIV)
	{
        parse_expr();
        expect(SEMICOLON);
        //cout << "SEMICOLON ";

        int LHS_index = get_memory_location(temp.lexeme);
        int op1_index = get_memory_location(op1);
        int op2_index = get_memory_location(op2);

        InstructionNode* assign_node = create_assign_instruction(LHS_index, op1_index, op2_index, op);

		return assign_node;

	}else{
        syntax_error();
    }
}

//expr → primary op primary
void parse_expr(){
    op1 = parse_primary();
    op = parse_op();
    op2 = parse_primary();
}

//primary → ID | NUM
std::string parse_primary(){
    Token t = lexer.peek(1);
    if (t.token_type == ID)
	{
        Token temp = expect(ID);
        allocate_memory_location(temp.lexeme, 0);
        //cout << temp.lexeme << " ";
        return temp.lexeme;
    }
    else if(t.token_type == NUM){
        Token temp = expect(NUM);
        allocate_memory_location(temp.lexeme, stoi(temp.lexeme));
        //cout << temp.lexeme << " ";
        //cout << "NUM ";
        return temp.lexeme;
    }else{
        syntax_error();
    }
}

//op → PLUS | MINUS | MULT | DIV
ArithmeticOperatorType parse_op(){
    Token t = lexer.peek(1);
    if (t.token_type == PLUS)
	{
        expect(PLUS);
        //cout << "PLUS ";
        return OPERATOR_PLUS;
    }
    else if(t.token_type == MINUS){
        expect(MINUS);
        //cout << "MINUS ";
        return OPERATOR_MINUS;
    }
    else if(t.token_type == MULT){
        expect(MULT);
        //cout << "MULT ";
        return OPERATOR_MULT;
    }
    else if(t.token_type == DIV){
        expect(DIV);
        //cout << "DIV ";
        return OPERATOR_DIV;
    }
    else{
        syntax_error();
    }
}

//output stmt → output ID SEMICOLON
InstructionNode* parse_output_stmt(){
    expect(OUTPUT);
    //cout << "OUTPUT ";
    Token temp = expect(ID);
    allocate_memory_location(temp.lexeme, 0);
    //cout << temp.lexeme << " ";

    int var_index = get_memory_location(temp.lexeme); // Get the memory location of the variable

    expect(SEMICOLON);
    //cout << "SEMICOLON ";

    InstructionNode* output_node = create_output_instruction(var_index);

    return output_node;

}

//input stmt → input ID SEMICOLON
InstructionNode* parse_input_stmt(){
    expect(INPUT);
    //cout << "INPUT ";
    Token temp = expect(ID);
    allocate_memory_location(temp.lexeme, 0);
    //cout << temp.lexeme << " ";

    int var_index = get_memory_location(temp.lexeme); // Get the memory location of the variable

    expect(SEMICOLON);
    //cout << "SEMICOLON ";

    // Create the input instruction node
    InstructionNode* input_node = create_input_instruction(var_index);
    // InstructionNode* current = input_node;
    // cout << "Input Node:";
    // cout << current->input_inst.var_index << "\n";
    
    return input_node;

    
    // Link the instruction node to the previous node in the linked list
    //link_instruction_node(input_node);

    // Update the pointer to the last node in the linked list if necessary
    //update_last_instruction_node(input_node);

}

//while stmt → WHILE condition body
void parse_while_stmt(){
    expect(WHILE);
    //cout << "WHILE ";
    parse_condition();
    parse_body();
}

//if stmt → IF condition body
void parse_if_stmt(){
    expect(IF);
    //cout << "IF ";
    parse_condition();
    parse_body();
}

//condition → primary relop primary
void parse_condition(){
    parse_primary();
    parse_relop();
    parse_primary();
}

//relop → GREATER | LESS | NOTEQUAL
void parse_relop(){
    Token t = lexer.peek(1);
    if (t.token_type == GREATER)
	{
        expect(GREATER);
        //cout << "GREATER ";
    }
    else if(t.token_type == LESS){
        expect(LESS);
        //cout << "LESS ";
    }
    else if(t.token_type == NOTEQUAL){
        expect(NOTEQUAL);
        //cout << "NOTEQUAL ";
    }else{
        syntax_error();
    }
}

//switch stmt → SWITCH ID LBRACE case_list RBRACE
//switch stmt → SWITCH ID LBRACE case_list default_case RBRACE
void parse_switch_stmt(){
    expect(SWITCH);
    //cout << "SWITCH ";
    Token temp = expect(ID);
    allocate_memory_location(temp.lexeme, 0);
    //cout << temp.lexeme << " ";
    expect(LBRACE);
    //cout << "LBRACE ";
    parse_case_list();
    Token t = lexer.peek(1);
    //first def
    if (t.token_type == RBRACE)
	{
        expect(RBRACE);
        //cout << "RBRACE ";
        return;
    }
    //second def
    else if(t.token_type == DEFAULT){
        parse_default_case();
        expect(RBRACE);
        //cout << "RBRACE ";
        return;
    }else{
        syntax_error();
    }

}

//for stmt → FOR LPAREN assign_stmt condition SEMICOLON assign_stmt RPAREN body
void parse_for_stmt(){
    expect(FOR);
    //cout << "FOR ";
    expect(LPAREN);
    //cout << "LPAREN ";
    parse_assign_stmt();
    parse_condition();
    expect(SEMICOLON);
    //cout << "SEMICOLON ";
    parse_assign_stmt();
    expect(RPAREN);
    //cout << "RPAREN ";
    parse_body();

}

//case_list → case case_list | case
void parse_case_list(){
    parse_case();
    Token t = lexer.peek(1);
    //first def
    if (t.token_type == CASE)
	{
        parse_case_list();
        return;
    }
    //second def
    else if (t.token_type == RBRACE || t.token_type == DEFAULT)
	{
        return;
    }else{
        syntax_error();
    }
}

//case → CASE NUM COLON body
void parse_case(){
    expect(CASE);
    //cout << "CASE ";
    Token temp = expect(NUM);
    allocate_memory_location(temp.lexeme, stoi(temp.lexeme));
    //cout << temp.lexeme << " ";
    //cout << "NUM ";
    expect(COLON);
    //cout << "COLON ";
    parse_body();
}

//default_case → DEFAULT COLON body
void parse_default_case(){
    expect(DEFAULT);
    //cout << "DEFAULT ";
    expect(COLON);
    //cout << "COLON ";
    parse_body();
}

//inputs → num_list
void parse_inputs(){
    parse_num_list();
}

//num_list → NUM
//num_list → NUM num_list
void parse_num_list(){
    Token temp = expect(NUM);
    inputs.push_back(stoi(temp.lexeme));
    //cout << temp.lexeme << " ";
    Token t = lexer.peek(1);
    //first def
    if (t.token_type == END_OF_FILE)
	{
        //cout << "\n";
        return;
    }
    //second def
    else if (t.token_type == NUM)
	{
        parse_num_list();
    }else{
        syntax_error();
    }
}

// Function to print the location table
void print_location_table() {
    cout << "Location Table:" << endl;
    for (const auto& entry : locationTable) {
        cout << entry.first << " -> " << entry.second << endl;
    }
}

void printinputs() {
    cout << "Inputs:" << endl;
    for (int i = 0; i < inputs.size(); ++i) {
        cout << inputs[i] << " ";
    }
    cout << endl;
}

void printMemory() {
    cout << "Memory:" << endl;
    for (int i = 0; i < nextLocation; ++i) {
        cout << mem[i] << " ";
    }
    cout << endl;
}

// std::vector<int> getInputs(){
//     return inputsList;
// }

struct InstructionNode * parse_generate_intermediate_representation(){


    parse_program();
    // print_location_table();
    // printinputs();
    // printMemory();

    //cout << "Linked List:" << "\n";
    InstructionNode* current = first_instruction;
    // while(current != nullptr){
    //     if(current->type == IN) {
    //         cout << "input " << current->input_inst.var_index << ";\n";
    //     }
    //     if(current->type == OUT) {
    //         cout << "output " << current->output_inst.var_index << ";\n";
    //     }
    //     if(current->type == ASSIGN){
    //         if(current->assign_inst.op == OPERATOR_NONE){
    //             cout << current->assign_inst.left_hand_side_index << " = " << current->assign_inst.operand1_index << ";\n";
    //         }else if(current->assign_inst.op == OPERATOR_PLUS){
    //             cout << current->assign_inst.left_hand_side_index << " = " << current->assign_inst.operand1_index << " + " << current->assign_inst.operand2_index << ";\n";
    //         }else if(current->assign_inst.op == OPERATOR_DIV){
    //             cout << current->assign_inst.left_hand_side_index << " = " << current->assign_inst.operand1_index << " / " << current->assign_inst.operand2_index << ";\n";
    //         }else if(current->assign_inst.op == OPERATOR_MINUS){
    //             cout << current->assign_inst.left_hand_side_index << " = " << current->assign_inst.operand1_index << " - " << current->assign_inst.operand2_index << ";\n";
    //         }else if(current->assign_inst.op == OPERATOR_MULT){
    //             cout << current->assign_inst.left_hand_side_index << " = " << current->assign_inst.operand1_index << " * " << current->assign_inst.operand2_index << ";\n";
    //         }
    //     }
    //     current = current->next;
    // }
    return current;

}
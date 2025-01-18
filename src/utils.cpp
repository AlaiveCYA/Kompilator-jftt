#include <vector>
#include <string>
#include <iostream>
#include <fstream>

#include "utils.hpp"
#include "symbol_table.hpp"

#define AUX_REGISTER1 MAX_REGISTERS - 1
#define AUX_REGISTER2 MAX_REGISTERS - 2
#define AUX_REGISTER3 MAX_REGISTERS - 3

#define CONST_ONE_REGISTER MAX_REGISTERS - 4
#define CONST_MINUS_ONE_REGISTER MAX_REGISTERS - 5
#define CONST_ZERO_REGISTER MAX_REGISTERS - 6

#define USER_REGISTERS CONST_ZERO_REGISTER - 1

/*************FILE MANAGEMENT**************/

static std::ofstream output_file;

static std::string output_filename;

void set_output_filename(std::string filename) {
    output_filename = filename;
}

/*************************************************************************/

static std::vector<command*> global_commands = std::vector<command*>();
static std::vector<SymbolTable*> symbol_tables = std::vector<SymbolTable*>();
static SymbolTable* current_table = nullptr;
static SymbolTable* global_table = nullptr;

bool is_used[3] = {false, false, false};

void print_error(std::string error, int line_number) {
    std::cout << "Error in line: " << line_number << "\n" << error << std::endl;
    exit(1);
}

/********************WRITE**********************/

void write_statement::generate_code() {
    if (value->type == VALUE) {
        // std::cout << "generating code for write statement" << std::endl;
        if(value->val == 1){
            output_file << "LOAD " << CONST_ONE_REGISTER << "\n";
        }
        else if(value->val == -1){
            output_file << "LOAD " << CONST_MINUS_ONE_REGISTER << "\n";
        }
        else if(value->val == 0){
            output_file << "LOAD " << CONST_ZERO_REGISTER << "\n";
        }
        else{
            output_file << "SET" << " " << value->val << "\n";
        }
        output_file << "PUT 0" << "\n";
    }
    if(value->type == VARIABLE){
        output_file << "PUT " << value->register_number << "\n";
    }
}

write_statement::write_statement(struct variable* value) {
    this->value = value;
}

command* create_write_statement(struct variable* value, int line_number) {
    write_statement* new_statement = new write_statement(value);
    if(value->type == VALUE){
        if(value->val == 1){
            is_used[0] = true;
        }
        else if(value->val == -1){
            is_used[1] = true;
        }
        else if(value->val == 0){
            is_used[2] = true;
        }
        new_statement->lines_taken = 2;
    }
    else if(value->type == VARIABLE){
        new_statement->lines_taken = 1;
    }
    if(value->is_initialized == false){
        print_error("Variable not initialized", line_number);
    }
    return new_statement;
}

/*********************READ**********************/

void read_statement::generate_code() {
    output_file << "GET " << var->register_number << "\n";
}

read_statement::read_statement(struct variable* value) {
    this->var = value;
}

command* create_read_statement(struct variable* value, int line_number) {
    read_statement* new_statement = new read_statement(value);
    if(value->type == VALUE){
        print_error("Cannot read into a number", line_number);
    }
    value->is_initialized = true;
    Symbol* symbol = current_table->get_symbol(value->name);
    if(symbol == nullptr){
        global_table->get_symbol(value->name)->is_initialized = true;
    }
    else{
        symbol->is_initialized = true;
    }
    new_statement->lines_taken = 1;
    return new_statement;
}

/********************ADDITION*************************/

addition::addition(struct variable* left, struct variable* right) {
    this->left = left;
    this->right = right;
}

void addition::generate_code() {
    if(left->type == VALUE && right->type == VALUE){
        if(left->val + right->val == 1){
            output_file << "LOAD " << CONST_ONE_REGISTER << "\n";
        }
        else if(left->val + right->val == -1){
            output_file << "LOAD " << CONST_MINUS_ONE_REGISTER << "\n";
        }
        else if(left->val + right->val == 0){
            output_file << "LOAD " << CONST_ZERO_REGISTER << "\n";
        }
        else{
            output_file << "SET " << left->val + right->val << "\n";
        }
    }
    else if (left->type == VALUE && right->type == VARIABLE) {
        if(left->val == 1){
            output_file << "LOAD " << CONST_ONE_REGISTER << "\n";
        }
        else if(left->val == -1){
            output_file << "LOAD " << CONST_MINUS_ONE_REGISTER << "\n";
        }
        else if(left->val == 0){
            output_file << "LOAD " << CONST_ZERO_REGISTER << "\n";
        }
        else{
            output_file << "SET " << left->val << "\n";
        }
        output_file << "ADD " << right->register_number << "\n";
    }
    else if (left->type == VARIABLE && right->type == VALUE) {

        if(right->val == 1){
            output_file << "LOAD " << CONST_ONE_REGISTER << "\n";
        }
        else if(right->val == -1){
            output_file << "LOAD " << CONST_MINUS_ONE_REGISTER << "\n";
        }
        else if(right->val == 0){
            output_file << "LOAD " << CONST_ZERO_REGISTER << "\n";
        }
        else{
            output_file << "SET " << right->val << "\n";
        }
        output_file << "ADD " << left->register_number << "\n";
    }
    else if (left->type == VARIABLE && right->type == VARIABLE) {
        output_file << "LOAD " << left->register_number << "\n";
        output_file << "ADD " << right->register_number << "\n";
    }
}

expression* create_addition(struct variable* left, struct variable* right, int line_number) {
    addition* new_addition = new addition(left, right);
    if (left->type == VALUE && right->type == VALUE) {
        if(left->val >= 0 && MAX_NUMBER - left->val < right->val){
            print_error("Number overflow", line_number);
        }
        else if(left->val < 0 && MIN_NUMBER - left->val > right->val){
            print_error("Number overflow", line_number);
        }
        new_addition->lines_taken = 1;

        if(left->val + right->val == 1){
            is_used[0] = true;
        }
        else if(left->val + right->val == -1){
            is_used[1] = true;
        }
        else if(left->val + right->val == 0){
            is_used[2] = true;
        }
    }
    else{

        if(left->type == VALUE){
            if(left->val == 1){
                is_used[0] = true;
            }
            else if(left->val == -1){
                is_used[1] = true;
            }
            else if(left->val == 0){
                is_used[2] = true;
            }
        }
        else if(right->type == VALUE){
            if(right->val == 1){
                is_used[0] = true;
            }
            else if(right->val == -1){
                is_used[1] = true;
            }
            else if(right->val == 0){
                is_used[2] = true;
            }
        }
        if(left->is_initialized == false || right->is_initialized == false){
            print_error("Variable not initialized", line_number);
        }

        new_addition->lines_taken = 2;
    }
    
    return new_addition;
}

/*********************SUBTRACTION*************************/

subtraction::subtraction(struct variable* left, struct variable* right) {
    this->left = left;
    this->right = right;
}

void subtraction::generate_code() {
    if (left->type == VALUE && right->type == VALUE) {
        if(left->val - right->type == 1){
            output_file << "LOAD " << CONST_ONE_REGISTER << "\n";
        }
        else if(left->val - right->type == -1){
            output_file << "LOAD " << CONST_MINUS_ONE_REGISTER << "\n";
        }
        else if(left->val - right->type == 0){
            output_file << "LOAD " << CONST_ZERO_REGISTER << "\n";
        }
        else{
            output_file << "SET " << left->val - right->val << "\n";
        }
    }
    else if (left->type == VALUE && right->type == VARIABLE) {
        
        if(left->val == 1){
            output_file << "LOAD " << CONST_ONE_REGISTER << "\n";
        }
        else if(left->val == -1){
            output_file << "LOAD " << CONST_MINUS_ONE_REGISTER << "\n";
        }
        else if(left->val == 0){
            output_file << "LOAD " << CONST_ZERO_REGISTER << "\n";
        }
        else{
            output_file << "SET " << left->val << "\n";
        }
        output_file << "SUB " << right->register_number << "\n";
    }
    else if (left->type == VARIABLE && right->type == VALUE) {

        if(right->val == 1){
            output_file << "LOAD " << CONST_MINUS_ONE_REGISTER << "\n";
        }
        else if(right->val == -1){
            output_file << "LOAD " << CONST_ONE_REGISTER << "\n";
        }
        else if(right->val == 0){
            output_file << "LOAD " << CONST_ZERO_REGISTER << "\n";
        }
        else{
            output_file << "SET -" << right->val << "\n";
        }
        
        output_file << "ADD " << left->register_number << "\n";
    }
    else if (left->type == VARIABLE && right->type == VARIABLE) {
        output_file << "LOAD " << left->register_number << "\n";
        output_file << "SUB " << right->register_number << "\n";
    }
}

expression* create_subtraction(struct variable* left, struct variable* right, int line_number) {

    subtraction* new_subtraction = new subtraction(left, right);
    if (left->type == VALUE && right->type == VALUE) {
        if(left->val >= 0 && MAX_NUMBER + left->val < right->val){
            print_error("Number overflow", line_number);
        }
        else if(left->val < 0 && MIN_NUMBER + left->val > right->val){
            print_error("Number overflow", line_number);
        }

        if(left->val - right->val == 1){
            is_used[0] = true;
        }
        else if(left->val - right->val == -1){
            is_used[1] = true;

        }
        else if(left->val - right->val == 0){
            is_used[2] = true;
        }

        new_subtraction->lines_taken = 1;
    }
    else{

        if(left->type == VALUE){
            if(left->val == 1){
                is_used[0] = true;
            }
            else if(left->val == -1){
                is_used[1] = true;
            }
            else if(left->val == 0){
                is_used[2] = true;
            }
        }
        else if(right->type == VALUE){
            if(right->val == 1){
                is_used[1] = true;
            }
            else if(right->val == -1){
                is_used[0] = true;
            }
            else if(right->val == 0){
                is_used[2] = true;
            }
        }

        if(left->is_initialized == false || right->is_initialized == false){
            print_error("Variable not initialized", line_number);
        }

        new_subtraction->lines_taken = 2;
    }
    return new_subtraction;
}

/****************VALUE EXP********************/

value_expression::value_expression(struct variable* value) {
    this->left = value;
}

void value_expression::generate_code() {
    if(left->type == VALUE){
        if(left->val == 1){
            output_file << "LOAD " << CONST_ONE_REGISTER << "\n";
        }
        else if(left->val == -1){
            output_file << "LOAD " << CONST_MINUS_ONE_REGISTER << "\n";
        }
        else if(left->val == 0){
            output_file << "LOAD " << CONST_ZERO_REGISTER << "\n";
        }
        else{
            output_file << "SET " << left->val << "\n";
        }
    }
    else if(left->type == VARIABLE){
        output_file << "LOAD " << left->register_number << "\n";
    }
}

expression* pass_variable_as_expression(struct variable* value, int line_number) {
    value_expression* new_value = new value_expression(value);
    new_value->lines_taken = 1;

    if(value->type == VALUE){
        if(value->val == 1){
            is_used[0] = true;
        }
        else if(value->val == -1){
            is_used[1] = true;
        }
        else if(value->val == 0){
            is_used[2] = true;
        }
    }

    if(value->is_initialized == false){
        print_error("Variable not initialized", line_number);
    }

    return new_value;
}

/****************ASSIGNMENT********************/

assignment::assignment(struct variable* left, expression* right) {
    this->left = left;
    this->right = right;
}

void assignment::generate_code() {

    right->generate_code();
    
    output_file << "STORE " << left->register_number << "\n";
}

command* create_assignment(struct variable* left, expression* right, int line_number) {
    if(left->type == VALUE){
        print_error("Cannot assign variable to a number", line_number);
    }
    assignment* new_assignment = new assignment(left, right);
    new_assignment->lines_taken = right->lines_taken + 1;

    left->is_initialized = true;

    Symbol* symbol = current_table->get_symbol(left->name);
    if(symbol == nullptr){
        global_table->get_symbol(left->name)->is_initialized = true;
    }
    else{
        symbol->is_initialized = true;
    }

    return new_assignment;
}

/*******************EQ CONDITION***************************/

condition::condition(struct variable* left, struct variable* right) {
    this->left = left;
    this->right = right;
}

condition* create_eq_condition(struct variable* left, struct variable* right, int line_number){

    eq_condition* new_condition = new eq_condition(left, right);

    new_condition->lines_taken = 3;
    new_condition->type = COND_EQ;

    if(left->type == VALUE){
        if(left->val == 1){
            is_used[0] = true;
        }
        else if(left->val == -1){
            is_used[1] = true;
        }
        else if(left->val == 0){
            is_used[2] = true;
        }
    }
    else if(right->type == VALUE){
        if(right->val == 1){
            is_used[0] = true;
        }
        else if(right->val == -1){
            is_used[1] = true;
        }
        else if(right->val == 0){
            is_used[2] = true;
        }
    }

    if(left->is_initialized == false || right->is_initialized == false){
        print_error("Variable not initialized", line_number);
    }

    return new_condition;
}

void eq_condition::generate_code() {
    if (left->type == VALUE && right->type == VARIABLE) {
        if(left->val == 1){
            output_file << "LOAD " << CONST_ONE_REGISTER << "\n";
        }
        else if(left->val == -1){
            output_file << "LOAD " << CONST_MINUS_ONE_REGISTER << "\n";
        }
        else if(left->val == 0){
            output_file << "LOAD " << CONST_ZERO_REGISTER << "\n";
        }
        else{
            output_file << "SET " << left->val << "\n";
        }
        output_file << "SUB " << right->register_number << "\n";
        output_file << "JZERO 2" << "\n";
    }
    else if (left->type == VARIABLE && right->type == VALUE) {
        if(right->val == 1){
            output_file << "LOAD " << CONST_ONE_REGISTER << "\n";
        }
        else if(right->val == -1){
            output_file << "LOAD " << CONST_MINUS_ONE_REGISTER << "\n";
        }
        else if(right->val == 0){
            output_file << "LOAD " << CONST_ZERO_REGISTER << "\n";
        }
        else{
            output_file << "SET " << right->val << "\n";
        }
        output_file << "SUB " << left->register_number << "\n";
        output_file << "JZERO 2" << "\n";
    }
    else if (left->type == VARIABLE && right->type == VARIABLE) {
        output_file << "LOAD " << left->register_number << "\n";
        output_file << "SUB " << right->register_number << "\n";
        output_file << "JZERO 2" << "\n";
    }
}

/*******************NEQ CONDITION************************/

condition* create_neq_condition(struct variable* left, struct variable* right, int line_number){

    neq_condition* new_condition = new neq_condition(left, right);
    new_condition->lines_taken = 2;
    new_condition->type = COND_NEQ;

    if(left->type == VALUE){
        if(left->val == 1){
            is_used[0] = true;
        }
        else if(left->val == -1){
            is_used[1] = true;
        }
        else if(left->val == 0){
            is_used[2] = true;
        }
    }
    else if(right->type == VALUE){
        if(right->val == 1){
            is_used[0] = true;
        }
        else if(right->val == -1){
            is_used[1] = true;
        }
        else if(right->val == 0){
            is_used[2] = true;
        }
    }

    if(left->is_initialized == false || right->is_initialized == false){
        print_error("Variable not initialized", line_number);
    }

    return new_condition;
}


void neq_condition::generate_code() {
    if (left->type == VALUE && right->type == VARIABLE) {
        if(left->val == 1){
            output_file << "LOAD " << CONST_ONE_REGISTER << "\n";
        }
        else if(left->val == -1){
            output_file << "LOAD " << CONST_MINUS_ONE_REGISTER << "\n";
        }
        else if(left->val == 0){
            output_file << "LOAD " << CONST_ZERO_REGISTER << "\n";
        }
        else{
            output_file << "SET " << left->val << "\n";
        }
        output_file << "SUB " << right->register_number << "\n";
    }
    else if (left->type == VARIABLE && right->type == VALUE) {
        if(right->val == 1){
            output_file << "LOAD " << CONST_ONE_REGISTER << "\n";
        }
        else if(right->val == -1){
            output_file << "LOAD " << CONST_MINUS_ONE_REGISTER << "\n";
        }
        else if(right->val == 0){
            output_file << "LOAD " << CONST_ZERO_REGISTER << "\n";
        }
        else{
            output_file << "SET " << right->val << "\n";
        }
        output_file << "SUB " << left->register_number << "\n";
    }
    else if (left->type == VARIABLE && right->type == VARIABLE) {
        output_file << "LOAD " << left->register_number << "\n";
        output_file << "SUB " << right->register_number << "\n";
    }
}

/*******************LT CONDITION************************/

condition* create_lt_condition(struct variable* left, struct variable* right, int line_number){

    lt_condition* new_condition = new lt_condition(left, right);
    new_condition->lines_taken = 3;
    new_condition->type = COND_LT;

    if(left->type == VALUE){
        if(left->val == 1){
            is_used[0] = true;
        }
        else if(left->val == -1){
            is_used[1] = true;
        }
        else if(left->val == 0){
            is_used[2] = true;
        }
    }
    else if(right->type == VALUE){
        if(right->val == 1){
            is_used[0] = true;
        }
        else if(right->val == -1){
            is_used[1] = true;
        }
        else if(right->val == 0){
            is_used[2] = true;
        }
    }

    if(left->is_initialized == false || right->is_initialized == false){
        print_error("Variable not initialized", line_number);
    }

    return new_condition;
}

void lt_condition::generate_code(void){
    if(left->type == VALUE && right->type == VARIABLE){
        if(left->val == 1){
            output_file << "LOAD " << CONST_ONE_REGISTER << "\n";
        }
        else if(left->val == -1){
            output_file << "LOAD " << CONST_MINUS_ONE_REGISTER << "\n";
        }
        else if(left->val == 0){
            output_file << "LOAD " << CONST_ZERO_REGISTER << "\n";
        }
        else{
            output_file << "SET " << left->val << "\n";
        }
        output_file << "SUB " << right->register_number << "\n";
        output_file << "JNEG 2" << "\n";
    }
    else if(left->type == VARIABLE && right->type == VALUE){
        if(right->val == 1){
            output_file << "LOAD " << CONST_ONE_REGISTER << "\n";
        }
        else if(right->val == -1){
            output_file << "LOAD " << CONST_MINUS_ONE_REGISTER << "\n";
        }
        else if(right->val == 0){
            output_file << "LOAD " << CONST_ZERO_REGISTER << "\n";
        }
        else{
            output_file << "SET " << right->val << "\n";
        }
        output_file << "SUB " << left->register_number << "\n";
        output_file << "JPOS 2" << "\n";
    }
    else if(left->type == VARIABLE && right->type == VARIABLE){
        output_file << "LOAD " << left->register_number << "\n";
        output_file << "SUB " << right->register_number << "\n";
        output_file << "JNEG 2" << "\n";
    }
}

/*******************LEQ CONDITION************************/

condition* create_leq_condition(struct variable* left, struct variable* right, int line_number){

    leq_condition* new_condition = new leq_condition(left, right);
    new_condition->lines_taken = 2;
    new_condition->type = COND_LEQ;

    if(left->type == VALUE){
        if(left->val == 1){
            is_used[0] = true;
        }
        else if(left->val == -1){
            is_used[1] = true;
        }
        else if(left->val == 0){
            is_used[2] = true;
        }
    }
    else if(right->type == VALUE){
        if(right->val == 1){
            is_used[0] = true;
        }
        else if(right->val == -1){
            is_used[1] = true;
        }
        else if(right->val == 0){
            is_used[2] = true;
        }
    }

    if(left->is_initialized == false || right->is_initialized == false){
        print_error("Variable not initialized", line_number);
    }

    return new_condition;
}

void leq_condition::generate_code(void){
    if(left->type == VALUE && right->type == VARIABLE){
        if(left->val == 1){
            output_file << "LOAD " << CONST_ONE_REGISTER << "\n";
        }
        else if(left->val == -1){
            output_file << "LOAD " << CONST_MINUS_ONE_REGISTER << "\n";
        }
        else if(left->val == 0){
            output_file << "LOAD " << CONST_ZERO_REGISTER << "\n";
        }
        else{
            output_file << "SET " << left->val << "\n";
        }
        output_file << "SUB " << right->register_number << "\n";
    }
    else if(left->type == VARIABLE && right->type == VALUE){
        if(right->val == 1){
            output_file << "LOAD " << CONST_ONE_REGISTER << "\n";
        }
        else if(right->val == -1){
            output_file << "LOAD " << CONST_MINUS_ONE_REGISTER << "\n";
        }
        else if(right->val == 0){
            output_file << "LOAD " << CONST_ZERO_REGISTER << "\n";
        }
        else{
            output_file << "SET " << right->val << "\n";
        }
        output_file << "SUB " << left->register_number << "\n";
    }
    else if(left->type == VARIABLE && right->type == VARIABLE){
        output_file << "LOAD " << left->register_number << "\n";
        output_file << "SUB " << right->register_number << "\n";
    }
}

/********************IF*******************************/

if_statement::if_statement(struct condition* condition, std::vector<command*> *commands) {
    this->condition = condition;
    this->commands = commands;
}

command* create_if_statement(struct condition* condition, std::vector<command*> *commands, int line_number) {
    
    if_statement* new_if = new if_statement(condition, commands);
    int lines_taken = 0;
    for(command* command : *commands){
        lines_taken += command->lines_taken;
    }

    new_if->lines_taken = condition->lines_taken + lines_taken + 1;
    return new_if;
}

void if_statement::generate_code() {

    if(condition->left->type == VALUE && condition->right->type == VALUE){
        if(condition->type == COND_EQ){
            if(condition->left->val == condition->right->val){
                for(command* command : *commands){
                    command->generate_code();
                }
            }
        }
        else if(condition->type == COND_NEQ){
            if(condition->left->val != condition->right->val){
                for(command* command : *commands){
                    command->generate_code();
                }
            }
        }
        else if(condition->type == COND_LT){
            if(condition->left->val < condition->right->val){
                for(command* command : *commands){
                    command->generate_code();
                }
            }
        }
        else if(condition->type == COND_LEQ){
            if(condition->left->val <= condition->right->val){
                for(command* command : *commands){
                    command->generate_code();
                }
            }
        }

        return;
    }

    condition->generate_code();
    if(condition->type == COND_EQ){
        output_file << "JUMP " << lines_taken - condition->lines_taken << "\n";
    }
    else if(condition->type == COND_NEQ){
        output_file << "JZERO " << lines_taken - condition->lines_taken << "\n";
    }
    else if(condition->type == COND_LT){
        output_file << "JUMP " << lines_taken - condition->lines_taken << "\n";
    }
    else if(condition->type == COND_LEQ){
        if(condition->left->type == VARIABLE && condition->right->type == VALUE){
            output_file << "JNEG " << lines_taken - condition->lines_taken << "\n";
        }
        else {
            output_file << "JPOS " << lines_taken - condition->lines_taken << "\n";
        }
    }
    for(command* command : *commands){
        command->generate_code();
    }
}

/******************IF-ELSE**********************************/

if_else_statement::if_else_statement(struct condition* condition, std::vector<command*> *if_commands, std::vector<command*> *else_commands) {
    this->condition = condition;
    this->if_commands = if_commands;
    this->else_commands = else_commands;
}

command* create_if_else_statement(struct condition* condition, std::vector<command*> *if_commands, std::vector<command*> *else_commands, int line_number) {
    if_else_statement* new_if_else = new if_else_statement(condition, if_commands, else_commands);
    int if_lines_taken = 0;
    for(command* command : *if_commands){
        if_lines_taken += command->lines_taken;
    }
    int else_lines_taken = 0;
    for(command* command : *else_commands){
        else_lines_taken += command->lines_taken;
    }
    new_if_else->if_lines_taken = if_lines_taken;
    new_if_else->else_lines_taken = else_lines_taken;
    new_if_else->lines_taken = condition->lines_taken + if_lines_taken + else_lines_taken + 2;
    return new_if_else;
}

void if_else_statement::generate_code() {

    if(condition->left->type == VALUE && condition->right->type == VALUE){
        if(condition->type == COND_EQ){
            if(condition->left->val == condition->right->val){
                for(command* command : *if_commands){
                    command->generate_code();
                }
            }
            else{
                for(command* command : *else_commands){
                    command->generate_code();
                }
            }
        }
        else if(condition->type == COND_NEQ){
            if(condition->left->val != condition->right->val){
                for(command* command : *if_commands){
                    command->generate_code();
                }
            }
            else{
                for(command* command : *else_commands){
                    command->generate_code();
                }
            }
        }
        else if(condition->type == COND_LT){
            if(condition->left->val < condition->right->val){
                for(command* command : *if_commands){
                    command->generate_code();
                }
            }
            else{
                for(command* command : *else_commands){
                    command->generate_code();
                }
            }
        }
        else if(condition->type == COND_LEQ){
            if(condition->left->val <= condition->right->val){
                for(command* command : *if_commands){
                    command->generate_code();
                }
            }
            else{
                for(command* command : *else_commands){
                    command->generate_code();
                }
            }
        }

        output_file << "JUMP -" << lines_taken - condition->lines_taken - 2 << "\n";

        return;
    }

    condition->generate_code();
    if(condition->type == COND_EQ){
        output_file << "JUMP " << if_lines_taken + 2 << "\n";
    }
    else if(condition->type == COND_NEQ){
        output_file << "JZERO " << if_lines_taken + 2 << "\n";
    }
    else if(condition->type == COND_LT){
        output_file << "JUMP " << if_lines_taken + 2 << "\n";
    }
    else if(condition->type == COND_LEQ){
        if(condition->left->type == VARIABLE && condition->right->type == VALUE){
            output_file << "JNEG " << if_lines_taken + 2 << "\n";
        }
        else {
            output_file << "JPOS " << if_lines_taken + 2 << "\n";
        }
    }
    for(command* command : *if_commands){
        command->generate_code();
    }
    output_file << "JUMP " << else_lines_taken + 1 << "\n";
    for(command* command : *else_commands){
        command->generate_code();
    }
}

/******************WHILE*************************************/

while_statement::while_statement(struct condition* condition, std::vector<command*> *commands) {
    this->condition = condition;
    this->commands = commands;
}

command* create_while_statement(struct condition* condition, std::vector<command*> *commands, int line_number) {
    while_statement* new_while = new while_statement(condition, commands);
    int lines_taken = 0;
    for(command* command : *commands){
        lines_taken += command->lines_taken;
    }
    new_while->lines_taken = condition->lines_taken + lines_taken + 2;
    return new_while;
}

void while_statement::generate_code() {
    
    if(condition->left->type == VALUE && condition->right->type == VALUE){
        if(condition->type == COND_EQ){
            if(condition->left->val == condition->right->val){
                for(command* command : *commands){
                    command->generate_code();
                }
            }
        }
        else if(condition->type == COND_NEQ){
            if(condition->left->val != condition->right->val){
                for(command* command : *commands){
                    command->generate_code();
                }
            }
        }
        else if(condition->type == COND_LT){
            if(condition->left->val < condition->right->val){
                for(command* command : *commands){
                    command->generate_code();
                }
            }
        }
        else if(condition->type == COND_LEQ){
            if(condition->left->val <= condition->right->val){
                for(command* command : *commands){
                    command->generate_code();
                }
            }
        }

        output_file << "JUMP -" << lines_taken - condition->lines_taken - 2 << "\n";

        return;
    }

    condition->generate_code();
    if(condition->type == COND_EQ){
        output_file << "JUMP " << lines_taken - condition->lines_taken << "\n";
    }
    else if(condition->type == COND_NEQ){
        output_file << "JZERO " << lines_taken - condition->lines_taken << "\n";
    }
    else if(condition->type == COND_LT){
        output_file << "JUMP " << lines_taken - condition->lines_taken << "\n";
    }
    else if(condition->type == COND_LEQ){
        if(condition->left->type == VARIABLE && condition->right->type == VALUE){
            output_file << "JNEG " << lines_taken - condition->lines_taken << "\n";
        }
        else {
            output_file << "JPOS " << lines_taken - condition->lines_taken << "\n";
        }
    }
    for(command* command : *commands){
        command->generate_code();
    }

    output_file << "JUMP -" << lines_taken - 1 << "\n";
    

}

/******************REPEAT-UNTIL***********************************/

repeat_until_statement::repeat_until_statement(std::vector<command*> *commands, struct condition* condition) {
    this->commands = commands;
    this->condition = condition;
}

command* create_repeat_until_statement(std::vector<command*> *commands, struct condition* condition, int line_number) {
    repeat_until_statement* new_repeat_until = new repeat_until_statement(commands, condition);
    int lines_taken = 0;
    for(command* command : *commands){
        lines_taken += command->lines_taken;
    }
    
    new_repeat_until->lines_taken = condition->lines_taken + lines_taken + 1;
    
    return new_repeat_until;
}

void repeat_until_statement::generate_code() {

    for(command* command : *commands){
        command->generate_code();
    }
    condition->generate_code();
    if(condition->type == COND_EQ){
        output_file << "JUMP -" << lines_taken - 1 << "\n";
    }
    else if(condition->type == COND_NEQ){
        output_file << "JZERO -" << lines_taken - 1 << "\n";
    }
    else if(condition->type == COND_LT){
        output_file << "JUMP -" << lines_taken - 1 << "\n";
    }
    else if(condition->type == COND_LEQ){
        if(condition->left->type == VARIABLE && condition->right->type == VALUE){
            output_file << "JNEG -" << lines_taken - 1 << "\n";
        }
        else {
            output_file << "JPOS -" << lines_taken - 1 << "\n";
        }
    }
}

/******************PASSING COMMANDS*******************************/

std::vector<command*> *pass_commands(std::vector<command*> *commands, command* new_commands) {
    
    commands->push_back(new_commands);
    return commands;   
}

std::vector<command*> *pass_commands(command* new_command) {
    std::vector<command*> *commands = new std::vector<command*>();
    commands->push_back(new_command);
    return commands;
}

/******************VARIABLES****************************/

struct variable* create_number_variable(long long int value, int line_number) {
    variable* new_variable = new variable();
    new_variable->type = VALUE;
    new_variable->val = value;
    new_variable->is_initialized = true;
    if(value == 1){
        is_used[0] = true;
    }
    else if(value == -1){
        is_used[1] = true;
    }
    else if(value == 0){
        is_used[2] = true;
    }
    return new_variable;
}

struct variable* create_variable(std::string name, int line_number) {
    if(current_table->get_symbol(name) == nullptr && global_table->get_symbol(name) == nullptr){
        print_error("Variable " + name + " not declared", line_number);
        return nullptr;
    }
    variable* new_variable = new variable();

    Symbol* symbol = current_table->get_symbol(name);

    if(symbol == nullptr){
        symbol = global_table->get_symbol(name);
    }

    if(symbol->type == ARRAY){
        print_error("Variable " + name + " is an array", line_number);
        return nullptr;
    }

    new_variable->name = name;
    new_variable->type = VARIABLE;
    new_variable->register_number = symbol->offset;
    new_variable->is_initialized = symbol->is_initialized;
    return new_variable;
}

/********************************COMPILER*****************************************/

void generate_code(void) {

    output_file.open(output_filename);

    if(is_used[0]){
        output_file << "SET 1 " << "\n";
        output_file << "STORE " << CONST_ONE_REGISTER << "\n";
    }
    if(is_used[1]){
        output_file << "SET -1 " << "\n";
        output_file << "STORE " << CONST_MINUS_ONE_REGISTER << "\n";
    }
    if(is_used[2]){
        output_file << "SET 0 " << "\n";
        output_file << "STORE " << CONST_ZERO_REGISTER << "\n";
    }

    for (command* command : global_commands) {
        command->generate_code();
    }
}

void end_program(void) {
    output_file << "HALT" << std::endl;
    output_file.close();
}

void open_file() {
    output_file.open(output_filename);
}

void close_file() {
    output_file.close();
}

void create_new_symbol_table(void) {
    SymbolTable* new_table = new SymbolTable();
    symbol_tables.push_back(new_table);
    current_table = new_table;
}

void set_globals(std::vector<command*> *commands) {
    global_table = current_table;
    global_commands = *commands;
}

void initialize_variable(std::string name, int line_number) {
    if(current_table == nullptr){
        current_table = new SymbolTable();
        symbol_tables.push_back(current_table);
    }
    if(current_table->get_symbol(name) == nullptr){
        Symbol* new_symbol = new Symbol(name, VARIABLE, 1, 0, 0);
        if(new_symbol->offset + new_symbol->length - 1 > USER_REGISTERS){
            print_error("Out of registers", line_number);
        }
        current_table->add_symbol(new_symbol);
    }
    else{
        print_error("Variable " + name + " already declared", line_number);
    }
}






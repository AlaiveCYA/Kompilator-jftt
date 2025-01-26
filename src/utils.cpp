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

static long user_registers = USER_REGISTERS;

void set_output_filename(std::string filename) {
    output_filename = filename;
}

/*************************************************************************/

static std::vector<command*> global_commands = std::vector<command*>();
static std::vector<procedure*> procedures = std::vector<procedure*>();
static SymbolTable* current_table = nullptr;

long int current_output_line = 0;

bool is_used[3] = {false, false, false};

void print_error(std::string error, int line_number) {
    std::cout << "Error in line: " << line_number << "\n" << error << std::endl;
    exit(1);
}

/********************WRITE**********************/

void write_statement::generate_code() {
    if(!value->is_pointer){
        if (value->type == VALUE) {

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
            current_output_line += 2;
        }
        if(value->type == VARIABLE){
            output_file << "PUT " << value->register_number << "\n";
            current_output_line++;
        }
    }
    else {
        if(value->type == VARIABLE){
            output_file << "LOADI " << value->register_number << "\n";
            output_file << "PUT 0" << "\n";
            current_output_line += 2;
        }
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
    }


    if(value->is_initialized == false){
        print_error("Variable not initialized", line_number);
    }

    if(value->is_pointer){
        new_statement->lines_taken = 2;
    }
    else{
        if(value->type == VALUE){
            new_statement->lines_taken = 2;
        }
        else{
            new_statement->lines_taken = 1;
        }
    }
    return new_statement;
}

/*********************READ**********************/

void read_statement::generate_code() {

    if(!var->is_pointer){
        if(var->type == VARIABLE){
            output_file << "GET " << var->register_number << "\n";
            current_output_line++;
        }
    }
    else{
        if(var->type == VARIABLE){
            output_file << "GET 0" << "\n";
            output_file << "STOREI " << var->register_number << "\n";
            current_output_line += 2;
        }
    }
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
        print_error("Variable not declared", line_number);
    }
    if(symbol->is_iterator){
        print_error("Cannot read into an iterator", line_number);
    }

    symbol->is_initialized = true;
    if(value->is_pointer){
        new_statement->lines_taken = 2;
    }
    else{
        new_statement->lines_taken = 1;
    }
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
        current_output_line++;
        return;
    }

    if(!left->is_pointer){

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
            current_output_line++;
            if(right->is_pointer){
                if(right->type == VARIABLE){
                    output_file << "ADDI " << right->register_number << "\n";
                    current_output_line++;
                }
            }
            else{
                if(right->type == VARIABLE){
                    output_file << "ADD " << right->register_number << "\n";
                    current_output_line++;
                }
            }
        }
        else if(left->type == VARIABLE){

            if(right->type == VALUE){
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
                current_output_line += 2;
            }
            if(right->is_pointer){
                if(right->type == VARIABLE){
                    output_file << "LOAD " << left->register_number << "\n";
                    output_file << "ADDI " << right->register_number << "\n";
                    current_output_line += 2;
                }
            }
            else{
                if(right->type == VARIABLE){
                    output_file << "LOAD " << left->register_number << "\n";
                    output_file << "ADD " << right->register_number << "\n";
                    current_output_line += 2;
                }
            }
        }
    }
    else{
        if(right->type == VALUE){
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
            current_output_line++;
            if(left->type == VARIABLE){
                output_file << "ADDI " << left->register_number << "\n";
                current_output_line++;
            }
        }
        if(right->is_pointer){
            if(right->type == VARIABLE){
                output_file << "LOADI " << left->register_number << "\n";
                output_file << "ADDI " << right->register_number << "\n";
                current_output_line += 2;
            }
        }
        else{
            if(right->type == VARIABLE){
                output_file << "LOADI " << left->register_number << "\n";
                output_file << "ADD " << right->register_number << "\n";
                current_output_line += 2;
            }
        }   
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

        if(left->type == VALUE && right->type == VALUE){
            new_addition->lines_taken = 1;
        }
        else{
            new_addition->lines_taken = 2;
        }
    }
    
    return new_addition;
}

/*********************SUBTRACTION*************************/

subtraction::subtraction(struct variable* left, struct variable* right) {
    this->left = left;
    this->right = right;
}

void subtraction::generate_code() {
   if(left->type == VALUE && right->type == VALUE){
        if(left->val - right->val == 1){
            output_file << "LOAD " << CONST_ONE_REGISTER << "\n";
        }
        else if(left->val - right->val == -1){
            output_file << "LOAD " << CONST_MINUS_ONE_REGISTER << "\n";
        }
        else if(left->val - right->val == 0){
            output_file << "LOAD " << CONST_ZERO_REGISTER << "\n";
        }
        else{
            output_file << "SET " << left->val - right->val << "\n";
        }
        current_output_line++;
        return;
    }

    if(!left->is_pointer){

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
            current_output_line++;
            if(right->is_pointer){
                if(right->type == VARIABLE){
                    output_file << "SUBI " << right->register_number << "\n";
                    current_output_line++;
                }
            }
            else{
                if(right->type == VARIABLE){
                    output_file << "SUB " << right->register_number << "\n";
                    current_output_line++;
                }
            }
        }
        else if(left->type == VARIABLE){

            if(right->type == VALUE){
                if(right->val == 1){
                    output_file << "LOAD " << CONST_MINUS_ONE_REGISTER << "\n";
                    current_output_line++;
                }
                else if(right->val == -1){
                    output_file << "LOAD " << CONST_ONE_REGISTER << "\n";
                    current_output_line++;
                }
                else if(right->val == 0){
                    output_file << "LOAD " << CONST_ZERO_REGISTER << "\n";
                    current_output_line++;
                }
                else{
                    output_file << "SET -" << right->val << "\n";
                    current_output_line++;
                }
                output_file << "ADD " << left->register_number << "\n";
                current_output_line++;
            }
            if(right->is_pointer){
                if(right->type == VARIABLE){
                    output_file << "LOAD " << left->register_number << "\n";
                    output_file << "SUBI " << right->register_number << "\n";
                    current_output_line += 2;
                }
            }
            else{
                if(right->type == VARIABLE){
                    output_file << "LOAD " << left->register_number << "\n";
                    output_file << "SUB " << right->register_number << "\n";
                    current_output_line += 2;
                }
            }
        }
    }
    else{
        if(right->type == VALUE){
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
            current_output_line++;
            if(left->type == VARIABLE){
                output_file << "ADDI " << left->register_number << "\n";
                current_output_line++;
            }
        }
        if(right->is_pointer){
            if(right->type == VARIABLE){
                output_file << "LOADI " << left->register_number << "\n";
                output_file << "SUBI " << right->register_number << "\n";
                current_output_line += 2;
            }
        }
        else{
            if(right->type == VARIABLE){
                output_file << "LOADI " << left->register_number << "\n";
                output_file << "SUB " << right->register_number << "\n";
                current_output_line += 2;
            }
        }   
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
        current_output_line++;
    }
    else if(left->is_pointer){
        if(left->type == VARIABLE){
            output_file << "LOADI " << left->register_number << "\n";
            current_output_line++;
        }
    }
    else{
        if(left->type == VARIABLE){
            output_file << "LOAD " << left->register_number << "\n";
            current_output_line++;
        }
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

    if(left->is_pointer){
        if(left->type == VARIABLE){
            output_file << "STOREI " << left->register_number << "\n";
            current_output_line++;
        }
    }
    else{
        if(left->type == VARIABLE){
            output_file << "STORE " << left->register_number << "\n";
            current_output_line++;
        }
    }
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
        print_error("Variable not declared", line_number);
    }
    if(symbol->is_iterator){
        print_error("Cannot assign to an iterator", line_number);
    }
    symbol->is_initialized = true;
    left->is_pointer = symbol->is_pointer;

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
    if (left->type == VALUE && right->type != VALUE) {
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
        current_output_line++;
        if(right->is_pointer){
            if(right->type == VARIABLE){
                output_file << "SUBI " << right->register_number << "\n";
                current_output_line++;
            }
        }
        else{
            if(right->type == VARIABLE){
                output_file << "SUB " << right->register_number << "\n";
                current_output_line++;
            }
        }
        output_file << "JZERO 2" << "\n";
        current_output_line++;
    }
    else if (left->type != VALUE && right->type == VALUE) {
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
        current_output_line++;
        if(left->is_pointer){
            if(left->type == VARIABLE){
                output_file << "SUBI " << left->register_number << "\n";
                current_output_line++;
            }
        }
        else{
            if(left->type == VARIABLE){
                output_file << "SUB " << left->register_number << "\n";
                current_output_line++;
            }
        }
        output_file << "JZERO 2" << "\n";
        current_output_line++;
    }
    else if(left->is_pointer){
        if(left->type == VARIABLE){
            output_file << "LOADI " << left->register_number << "\n";
            current_output_line++;
        }
        if(right->is_pointer){
            if(right->type == VARIABLE){
                output_file << "SUBI " << right->register_number << "\n";
                current_output_line++;
            }
        }
        else{
            if(right->type == VARIABLE){
                output_file << "SUB " << right->register_number << "\n";
                current_output_line++;
            }
        }
        output_file << "JZERO 2" << "\n";
        current_output_line++;
    }
    else{
        if(left->type == VARIABLE){
            output_file << "LOAD " << left->register_number << "\n";
            current_output_line++;
        }
        if(right->is_pointer){
            if(right->type == VARIABLE){
                output_file << "SUBI " << right->register_number << "\n";
                current_output_line++;
            }
        }
        else{
            if(right->type == VARIABLE){
                output_file << "SUB " << right->register_number << "\n";
                current_output_line++;
            }
        }   
        output_file << "JZERO 2" << "\n";
        current_output_line++;
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
    if (left->type == VALUE && right->type != VALUE) {
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
        current_output_line++;
        if(right->is_pointer){
            if(right->type == VARIABLE){
                output_file << "SUBI " << right->register_number << "\n";
                current_output_line++;
            }
        }
        else{
            if(right->type == VARIABLE){
                output_file << "SUB " << right->register_number << "\n";
                current_output_line++;
            }
        }
    }
    else if (left->type != VALUE && right->type == VALUE) {
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
        current_output_line++;
        if(left->is_pointer){
            if(left->type == VARIABLE){
                output_file << "SUBI " << left->register_number << "\n";
                current_output_line++;
            }
        }
        else{
            if(left->type == VARIABLE){
                output_file << "SUB " << left->register_number << "\n";
                current_output_line++;
            }
        }
    }
    else if (left->is_pointer){
        if(left->type == VARIABLE){
            output_file << "LOADI " << left->register_number << "\n";
            current_output_line++;
        }
        if(right->is_pointer){
            if(right->type == VARIABLE){
                output_file << "SUBI " << right->register_number << "\n";
                current_output_line++;
            }
        }
        else{
            if(right->type == VARIABLE){
                output_file << "SUB " << right->register_number << "\n";
                current_output_line++;
            }
        }
    }
    else{
        if(left->type == VARIABLE){
            output_file << "LOAD " << left->register_number << "\n";
            current_output_line++;
        }
        if(right->is_pointer){
            if(right->type == VARIABLE){
                output_file << "SUBI " << right->register_number << "\n";
                current_output_line++;
            }
        }
        else{
            if(right->type == VARIABLE){
                output_file << "SUB " << right->register_number << "\n";
                current_output_line++;
            }
        }   
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
    if(left->type == VALUE && right->type != VALUE){
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
        current_output_line++;
        if(right->is_pointer){
            if(right->type == VARIABLE){
                output_file << "SUBI " << right->register_number << "\n";
                current_output_line++;
            }
        }
        else{
            if(right->type == VARIABLE){
                output_file << "SUB " << right->register_number << "\n";
                current_output_line++;
            }
        }
        output_file << "JNEG 2" << "\n";
        current_output_line++;
    }
    else if(left->type != VALUE && right->type == VALUE){
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
        current_output_line++;
        if(left->is_pointer){
            if(left->type == VARIABLE){
                output_file << "SUBI " << left->register_number << "\n";
                current_output_line++;
            }
        }
        else{
            if(left->type == VARIABLE){
                output_file << "SUB " << left->register_number << "\n";
                current_output_line++;
            }
        }
        output_file << "JPOS 2" << "\n";
        current_output_line++;
    }
    else if(left->is_pointer){
        if(left->type == VARIABLE){
            output_file << "LOADI " << left->register_number << "\n";
            current_output_line++;
        }
        if(right->is_pointer){
            if(right->type == VARIABLE){
                output_file << "SUBI " << right->register_number << "\n";
                current_output_line++;
            }
        }
        else{
            if(right->type == VARIABLE){
                output_file << "SUB " << right->register_number << "\n";
                current_output_line++;
            }
        }
        output_file << "JNEG 2" << "\n";
        current_output_line++;
    }
    else{
        if(left->type == VARIABLE){
            output_file << "LOAD " << left->register_number << "\n";
            current_output_line++;
        }
        if(right->is_pointer){
            if(right->type == VARIABLE){
                output_file << "SUBI " << right->register_number << "\n";
                current_output_line++;
            }
        }
        else{
            if(right->type == VARIABLE){
                output_file << "SUB " << right->register_number << "\n";
                current_output_line++;
            }
        }   
        output_file << "JNEG 2" << "\n";
        current_output_line++;
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
    if(left->type == VALUE && right->type != VALUE){
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
        current_output_line++;
        if(right->is_pointer){
            if(right->type == VARIABLE){
                output_file << "SUBI " << right->register_number << "\n";
                current_output_line++;
            }
        }
        else{
            if(right->type == VARIABLE){
                output_file << "SUB " << right->register_number << "\n";
                current_output_line++;
            }
        }
    }
    else if(left->type != VALUE && right->type == VALUE){
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
        current_output_line++;
        if(left->is_pointer){
            if(left->type == VARIABLE){
                output_file << "SUBI " << left->register_number << "\n";
                current_output_line++;
            }
        }
        else{
            if(left->type == VARIABLE){
                output_file << "SUB " << left->register_number << "\n";
                current_output_line++;
            }
        }
    }
    else if(left->is_pointer){
        if(left->type == VARIABLE){
            output_file << "LOADI " << left->register_number << "\n";
            current_output_line++;
        }
        if(right->is_pointer){
            if(right->type == VARIABLE){
                output_file << "SUBI " << right->register_number << "\n";
                current_output_line++;

            }
        }
        else{
            if(right->type == VARIABLE){
                output_file << "SUB " << right->register_number << "\n";
                current_output_line++;
            }
        }
    }
    else{
        if(left->type == VARIABLE){
            output_file << "LOAD " << left->register_number << "\n";
            current_output_line++;
        }
        if(right->is_pointer){
            if(right->type == VARIABLE){
                output_file << "SUBI " << right->register_number << "\n";
                current_output_line++;
            }
        }
        else{
            if(right->type == VARIABLE){
                output_file << "SUB " << right->register_number << "\n";
                current_output_line++;
            }
        }   
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

    if(condition->left->type == VALUE && condition->right->type == VALUE){
        if(condition->type == COND_EQ){
            if(condition->left->val == condition->right->val){
                new_if->lines_taken = lines_taken;
            }
            else{
                new_if->lines_taken = 0;
            }
        }
        else if(condition->type == COND_NEQ){
            if(condition->left->val != condition->right->val){
                new_if->lines_taken = lines_taken;
            }
            else{
                new_if->lines_taken = 0;
            }
        }
        else if(condition->type == COND_LT){
            if(condition->left->val < condition->right->val){
                new_if->lines_taken = lines_taken;
            }
            else{
                new_if->lines_taken = 0;
            }
        }
        else if(condition->type == COND_LEQ){
            if(condition->left->val <= condition->right->val){
                new_if->lines_taken = lines_taken;
            }
            else{
                new_if->lines_taken = 0;
            }
        }
        return new_if;
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
        current_output_line++;
    }
    else if(condition->type == COND_NEQ){
        output_file << "JZERO " << lines_taken - condition->lines_taken << "\n";
        current_output_line++;
    }
    else if(condition->type == COND_LT){
        output_file << "JUMP " << lines_taken - condition->lines_taken << "\n";
        current_output_line++;
    }
    else if(condition->type == COND_LEQ){
        if(condition->left->type == VARIABLE && condition->right->type == VALUE){
            output_file << "JNEG " << lines_taken - condition->lines_taken << "\n";
            current_output_line++;
        }
        else {
            output_file << "JPOS " << lines_taken - condition->lines_taken << "\n";
            current_output_line++;
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
    if(condition->left->type == VALUE && condition->right->type == VALUE){
        if(condition->type == COND_EQ){
            if(condition->left->val == condition->right->val){
                new_if_else->lines_taken = if_lines_taken;
            }
            else{
                new_if_else->lines_taken = else_lines_taken;
            }
        }
        else if(condition->type == COND_NEQ){
            if(condition->left->val != condition->right->val){
                new_if_else->lines_taken = if_lines_taken;
            }
            else{
                new_if_else->lines_taken = else_lines_taken;
            }
        }
        else if(condition->type == COND_LT){
            if(condition->left->val < condition->right->val){
                new_if_else->lines_taken = if_lines_taken;
            }
            else{
                new_if_else->lines_taken = else_lines_taken;
            }
        }
        else if(condition->type == COND_LEQ){
            if(condition->left->val <= condition->right->val){
                new_if_else->lines_taken = if_lines_taken;
            }
            else{
                new_if_else->lines_taken = else_lines_taken;
            }
        }
        return new_if_else;
    }
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

        return;
    }

    condition->generate_code();
    if(condition->type == COND_EQ){
        output_file << "JUMP " << if_lines_taken + 2 << "\n";
        current_output_line++;
    }
    else if(condition->type == COND_NEQ){
        output_file << "JZERO " << if_lines_taken + 2 << "\n";
        current_output_line++;
    }
    else if(condition->type == COND_LT){
        output_file << "JUMP " << if_lines_taken + 2 << "\n";
        current_output_line++;
    }
    else if(condition->type == COND_LEQ){
        if(condition->left->type == VARIABLE && condition->right->type == VALUE){
            output_file << "JNEG " << if_lines_taken + 2 << "\n";
            current_output_line++;
        }
        else {
            output_file << "JPOS " << if_lines_taken + 2 << "\n";
            current_output_line++;
        }
    }
    for(command* command : *if_commands){
        command->generate_code();
    }
    output_file << "JUMP " << else_lines_taken + 1 << "\n";
    current_output_line++;
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

    if(condition->left->type == VALUE && condition->right->type == VALUE){
        if(condition->type == COND_EQ){
            if(condition->left->val == condition->right->val){
                new_while->lines_taken = lines_taken + 1;
            }
            else{
                new_while->lines_taken = 0;
            }
        }
        else if(condition->type == COND_NEQ){
            if(condition->left->val != condition->right->val){
                new_while->lines_taken = lines_taken + 1; 
            }
            else{
                new_while->lines_taken = 0;
            }
        }
        else if(condition->type == COND_LT){
            if(condition->left->val < condition->right->val){
                new_while->lines_taken = lines_taken + 1;
            }
            else{
                new_while->lines_taken = 0;
            }
        }
        else if(condition->type == COND_LEQ){
            if(condition->left->val <= condition->right->val){
                new_while->lines_taken = lines_taken + 1;
            }
            else{
                new_while->lines_taken = 0;
            }
        }
        return new_while;
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
                output_file << "JUMP -" << lines_taken - 2 << "\n";
                current_output_line++;
            }
        }
        else if(condition->type == COND_NEQ){
            if(condition->left->val != condition->right->val){
                for(command* command : *commands){
                    command->generate_code();
                }
                output_file << "JUMP -" << lines_taken - 2 << "\n";
                current_output_line++;
            }
        }
        else if(condition->type == COND_LT){
            if(condition->left->val < condition->right->val){
                for(command* command : *commands){
                    command->generate_code();
                }
                output_file << "JUMP -" << lines_taken - 2 << "\n";
                current_output_line++;
            }
        }
        else if(condition->type == COND_LEQ){
            if(condition->left->val <= condition->right->val){
                for(command* command : *commands){
                    command->generate_code();
                }
                output_file << "JUMP -" << lines_taken - 2 << "\n";
                current_output_line++;
            }
        }

        return;
    }

    condition->generate_code();
    if(condition->type == COND_EQ){
        output_file << "JUMP " << lines_taken - condition->lines_taken << "\n";
        current_output_line++;
    }
    else if(condition->type == COND_NEQ){
        output_file << "JZERO " << lines_taken - condition->lines_taken << "\n";
        current_output_line++;
    }
    else if(condition->type == COND_LT){
        output_file << "JUMP " << lines_taken - condition->lines_taken << "\n";
        current_output_line++;
    }
    else if(condition->type == COND_LEQ){
        if(condition->left->type != VALUE && condition->right->type == VALUE){
            output_file << "JNEG " << lines_taken - condition->lines_taken << "\n";
            current_output_line++;
        }
        else {
            output_file << "JPOS " << lines_taken - condition->lines_taken << "\n";
            current_output_line++;
        }
    }
    for(command* command : *commands){
        command->generate_code();
    }

    output_file << "JUMP -" << lines_taken - 1 << "\n";
    current_output_line++;
    

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
        current_output_line++;
    }
    else if(condition->type == COND_NEQ){
        output_file << "JZERO -" << lines_taken - 1 << "\n";
        current_output_line++;
    }
    else if(condition->type == COND_LT){
        output_file << "JUMP -" << lines_taken - 1 << "\n";
        current_output_line++;
    }
    else if(condition->type == COND_LEQ){
        if(condition->left->type != VALUE && condition->right->type == VALUE){
            output_file << "JNEG -" << lines_taken - 1 << "\n";
            current_output_line++;
        }
        else {
            output_file << "JPOS -" << lines_taken - 1 << "\n";
            current_output_line++;
        }
    }

}

/******************FOR***********************************/

for_statement::for_statement(struct variable* variable, struct variable* start, struct variable* end, std::vector<command*> *commands, for_direction_t direction) {
    this->iterator = variable;
    this->start = start;
    this->end = end;
    this->commands = commands;
    this->direction = direction;
}

command* create_for_statement(struct variable* iterator, struct variable* start, struct variable* end, std::vector<command*> *commands, int line_number, for_direction_t direction) {

    for_statement* new_for = new for_statement(iterator, start, end, commands, direction);
    
    if(iterator->type == VALUE){
        print_error("Cannot assign variable to a number", line_number);
    }

    new_for->end_register = get_register_counter();
    increment_register_counter();

    if(new_for->end_register + 1 > user_registers){
        print_error("Out of registers", line_number);
    }

    is_used[0] = true;

    if(start->type == VALUE){
        if(start->val == -1){
            is_used[1] = true;
        }
        else if(start->val == 0){
            is_used[2] = true;
        }
    }
    if(end->type == VALUE){
        if(end->val == -1){
            is_used[1] = true;
        }
        else if(end->val == 0){
            is_used[2] = true;
        }
    }
    int lines_taken = 0;
    for(command* command : *commands){
        lines_taken += command->lines_taken;
    }
    new_for->lines_taken = lines_taken + 11;

    current_table->remove_symbol(iterator->name);

    return new_for;
}

void for_statement::generate_code(void){

    if(start->type == VALUE){
        if(start->val == 1){
            output_file << "LOAD " << CONST_ONE_REGISTER << "\n";
        }
        else if(start->val == -1){
            output_file << "LOAD " << CONST_MINUS_ONE_REGISTER << "\n";
        }
        else if(start->val == 0){
            output_file << "LOAD " << CONST_ZERO_REGISTER << "\n";
        }
        else{
            output_file << "SET " << start->val << "\n";
        }
        current_output_line++;
    }
    else if(start->is_pointer){
        if(start->type == VARIABLE){
            output_file << "LOADI " << start->register_number << "\n";
            current_output_line++;
        }
    }
    else{
        if(start->type == VARIABLE){
            output_file << "LOAD " << start->register_number << "\n";
            current_output_line++;
        }
    }
    output_file << "STORE " << iterator->register_number << "\n";
    current_output_line++;

    if(end->type == VALUE){
        if(end->val == 1){
            output_file << "LOAD " << CONST_ONE_REGISTER << "\n";
        }
        else if(end->val == -1){
            output_file << "LOAD " << CONST_MINUS_ONE_REGISTER << "\n";
        }
        else if(end->val == 0){
            output_file << "LOAD " << CONST_ZERO_REGISTER << "\n";
        }
        else{
            output_file << "SET " << start->val << "\n";
        }
        current_output_line++;
    }
    else if(end->is_pointer){
        if(end->type == VARIABLE){
            output_file << "LOADI " << end->register_number << "\n";
            current_output_line++;
        }
    }
    else{
        if(end->type == VARIABLE){
            output_file << "LOAD " << end->register_number << "\n";
            current_output_line++;
        }
    }

    output_file << "STORE " << end_register << "\n";
    current_output_line++;

    if(direction == UP){
        
        output_file << "LOAD " << end_register << "\n";
        output_file << "SUB " << iterator->register_number << "\n";
        output_file << "JNEG " << lines_taken - 6 << "\n";
        current_output_line+=3;

    }
    else if(direction == DOWN){

        output_file << "LOAD " << end_register << "\n";
        output_file << "SUB " << iterator->register_number << "\n";
        output_file << "JPOS " << lines_taken - 6 << "\n";
        current_output_line+=3;
    }

    for(command* command : *commands){
        command->generate_code();
    }

    if(direction == UP){
        output_file << "LOAD " << iterator->register_number << "\n";
        output_file << "ADD " << CONST_ONE_REGISTER << "\n";
        output_file << "STORE " << iterator->register_number << "\n";
        output_file << "JUMP -" << lines_taken - 5 << "\n";
        current_output_line+=4;
    }
    else if(direction == DOWN){
        output_file << "LOAD " << iterator->register_number << "\n";
        output_file << "SUB " << CONST_ONE_REGISTER << "\n";
        output_file << "STORE " << iterator->register_number << "\n";
        output_file << "JUMP -" << lines_taken - 5 << "\n";
        current_output_line+=4;
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
    if(current_table->get_symbol(name) == nullptr){
        print_error("Variable " + name + " not declared", line_number);
        return nullptr;
    }
    variable* new_variable = new variable();

    Symbol* symbol = current_table->get_symbol(name);

    if(symbol->type == ARRAY){
        print_error("Variable " + name + " is an array", line_number);
        return nullptr;
    }

    new_variable->name = name;
    new_variable->type = VARIABLE;
    new_variable->register_number = symbol->offset;
    new_variable->is_initialized = symbol->is_initialized;
    new_variable->is_iterator = false;
    new_variable->is_pointer = symbol->is_pointer;
    return new_variable;
}

/************************FORMAL PARAMETERS*************************************/

std::vector<formal_parameter*> *create_parameters(std::string name, var_type_t type, int line_number) {
    
    if(current_table == nullptr){
        current_table = new SymbolTable();
    }
    if(current_table->get_symbol(name) == nullptr){
        if(get_register_counter() > user_registers){
            print_error("Out of registers", line_number);
        }
        Symbol* new_symbol = new Symbol(name, type, 1, 0, 0, user_registers);
        new_symbol->is_pointer = true;
        new_symbol->is_initialized = true;
        user_registers--;
        current_table->add_symbol(new_symbol);
        formal_parameter* new_parameter = new formal_parameter();
        std::vector<formal_parameter*> *parameters = new std::vector<formal_parameter*>();
        new_parameter->name = name;
        new_parameter->type = type;
        new_parameter->register_number = new_symbol->offset;
        parameters->push_back(new_parameter);
        return parameters;
        
    }
    else{
        print_error("Variable " + name + " already declared", line_number);
        return nullptr;
    }
}

std::vector<formal_parameter*> *create_parameters(std::vector<formal_parameter*> *params, std::string name, var_type_t type, int line_number) {
    
    if(current_table == nullptr){
        current_table = new SymbolTable();
    }
    if(current_table->get_symbol(name) == nullptr){
        if(get_register_counter() > user_registers){
            print_error("Out of registers", line_number);
        }
        Symbol* new_symbol = new Symbol(name, type, 1, 0, 0, user_registers);
        new_symbol->is_pointer = true;
        new_symbol->is_initialized = true;
        user_registers -= 1;
        current_table->add_symbol(new_symbol);
        formal_parameter* new_parameter = new formal_parameter();
        new_parameter->name = name;
        new_parameter->type = type;
        new_parameter->register_number = new_symbol->offset;   
        params->push_back(new_parameter);
        return params;
        
    }
    else{
        print_error("Variable " + name + " already declared", line_number);
        return nullptr;
    }
}

/******************************PROCEDURE*****************************/

procedure::procedure(std::string name, std::vector<formal_parameter*> *parameters) {
    this->name = name;
    this->parameters = parameters;
}

procedure* initialize_procedure(std::string name, std::vector<formal_parameter*> *parameters, int line_number) {
    
    procedure* new_procedure = new procedure(name, parameters);
    new_procedure->name = name;
    new_procedure->parameters = parameters;

    if(get_register_counter() > user_registers){
        print_error("Out of registers", line_number);
    }

    new_procedure->return_position = user_registers;
    user_registers -= 1;

    procedures.push_back(new_procedure);
    return new_procedure;
}

procedure* get_procedure(std::string name, int line_number) {
    for(procedure* procedure : procedures){
        if(procedure->name == name){
            return procedure;
        }
    }
    print_error("Procedure " + name + " not declared", line_number);
    return nullptr;
}

void create_procedure(procedure* new_procedure, std::vector<command*> *commands, int line_number) {
    new_procedure->commands = commands;
    int lines_taken = 0;
    for(command* command : *commands){
        lines_taken += command->lines_taken;
    }
    new_procedure->lines_taken = lines_taken + 1;

    int offset = 0;

    for (procedure* procedure : procedures) {
        offset += procedure->lines_taken;
    }

    new_procedure->begining_line = offset + 1 - new_procedure->lines_taken;

    current_table = nullptr;
}

void procedure::generate_code(void) {

    for(command* command : *commands){
        command->generate_code();
    }
    output_file << "RTRN " << return_position << "\n";
    current_output_line++;
}

/**********************ACTUAL PARAMETERS**********************/

std::vector<std::string> *create_actual_parameters(std::vector<std::string> *parameters, std::string new_parameter, int line_number) {
    parameters->push_back(new_parameter);
    return parameters;
}

std::vector<std::string> *create_actual_parameters(std::string new_parameter, int line_number) {
    std::vector<std::string> *parameters = new std::vector<std::string>();
    parameters->push_back(new_parameter);
    return parameters;
}

/***************************CALLING PROCEDURE***************************/

procedure_call::procedure_call(std::string name, std::vector<variable*> *parameters) {
    this->name = name;
    this->params = parameters;
}

command* create_procedure_call(std::string name, std::vector<std::string> *parameters, int line_number) {
    procedure* called_procedure = get_procedure(name, line_number);
    if(called_procedure == nullptr){
        print_error("Procedure " + name + " not declared", line_number);
    }

    if(called_procedure->parameters->size() != parameters->size()){
        print_error("Wrong number of arguments", line_number);
    }

    std::vector<variable*> *params = new std::vector<variable*>();
    for(size_t i = 0; i < called_procedure->parameters->size(); i++){
        Symbol* parameter = current_table->get_symbol(parameters->at(i));

        if(parameter == nullptr){
            print_error("Variable " + parameters->at(i) + " not declared", line_number);
        }

        if(parameter->type != called_procedure->parameters->at(i)->type){
            print_error("Wrong type of argument: " + parameter->name, line_number);
        }

        variable* new_parameter = new variable();
        new_parameter->name = parameter->name;
        new_parameter->type = parameter->type;
        new_parameter->register_number = parameter->offset;
        if(new_parameter->register_number == 1){
            is_used[0] = true;
        }
        new_parameter->is_initialized = true;
        new_parameter->is_iterator = false;
        new_parameter->is_pointer = parameter->is_pointer;

        parameter->is_initialized = true;

        params->push_back(new_parameter);

    }

    procedure_call* new_call = new procedure_call(name, params);
    new_call->proc = called_procedure;

    int lines_taken = 0;

    for(size_t i = 0; i < params->size(); i++){
        lines_taken += 2;
    }
    new_call->lines_taken = lines_taken + 3;
    return new_call;
}


void procedure_call::generate_code(void) {
    for(size_t i = 0; i < params->size(); i++){
        if(params->at(i)->is_pointer){
            if(params->at(i)->type == VARIABLE){
                output_file << "LOAD " << params->at(i)->register_number << "\n";
                current_output_line++;
            }
        }
        else{
            if(params->at(i)->type == VARIABLE){
                output_file << "SET " << params->at(i)->register_number << "\n";
                current_output_line++;
            }
        }
        output_file << "STORE " << proc->parameters->at(i)->register_number << "\n";
        current_output_line++;
    }
    output_file << "SET " << current_output_line + 3 << "\n";
    output_file << "STORE " << proc->return_position << "\n";
    current_output_line += 2;
    output_file << "JUMP " << proc->begining_line - current_output_line - 1 << "\n";
    current_output_line++;
}

/********************************COMPILER*****************************************/

void generate_code(void) {

    output_file.open(output_filename);

    int procedure_lines = 0;

    for(procedure* procedure : procedures){
        procedure->begining_line++;
        procedure_lines += procedure->lines_taken;
    }


    if(is_used[0]){
        output_file << "SET 1 " << "\n";
        output_file << "STORE " << CONST_ONE_REGISTER << "\n";
        current_output_line += 2;

        for(procedure* procedure : procedures){
            procedure->begining_line += 2;
        }
    }
    if(is_used[1]){
        output_file << "SET -1 " << "\n";
        output_file << "STORE " << CONST_MINUS_ONE_REGISTER << "\n";
        current_output_line += 2;

        for(procedure* procedure : procedures){
            procedure->begining_line += 2;
        }
    }
    if(is_used[2]){
        output_file << "SET 0 " << "\n";
        output_file << "STORE " << CONST_ZERO_REGISTER << "\n";
        current_output_line += 2;

        for(procedure* procedure : procedures){
            procedure->begining_line += 2;
        }
    }

    output_file << "JUMP " << procedure_lines + 1 << "\n";
    current_output_line++;

    for(procedure* procedure : procedures){
        procedure->generate_code();
    }

    for (command* command : global_commands) {
        command->generate_code();
    }

}

void end_program(void) {
    output_file << "HALT" << std::endl;
    current_output_line++;
    output_file.close();
}

void open_file() {
    output_file.open(output_filename);
}

void close_file() {
    output_file.close();
}


void set_globals(std::vector<command*> *commands) {
    global_commands = *commands;
}

void initialize_variable(std::string name, int line_number) {
    if(current_table == nullptr){
        current_table = new SymbolTable();
    }
    if(current_table->get_symbol(name) == nullptr){
        Symbol* new_symbol = new Symbol(name, VARIABLE, 1, 0, 0);
        new_symbol->is_initialized = false;
        new_symbol->is_pointer = false;
        new_symbol->is_iterator = false;
        if(new_symbol->offset + new_symbol->length - 1 > user_registers){
            print_error("Out of registers", line_number);
        }
        current_table->add_symbol(new_symbol);
    }
    else{
        print_error("Variable " + name + " already declared", line_number);
    }
}

void initialize_array(std::string name, int begining, int end, int line_number) {
    if(current_table == nullptr){
        current_table = new SymbolTable();
    }
    if(current_table->get_symbol(name) == nullptr){
        long long int length = end - begining + 1;
        Symbol* new_symbol = new Symbol(name, ARRAY, length, begining, end, get_register_counter() - begining);
        new_symbol->is_initialized = false;
        new_symbol->is_pointer = false;
        new_symbol->is_iterator = false;
        if(new_symbol->offset + new_symbol->length - 1 > user_registers){
            print_error("Out of registers", line_number);
        }
        current_table->add_symbol(new_symbol);
    }
    else{
        print_error("Variable " + name + " already declared", line_number);
    }
}

struct variable* create_iterator(std::string name, int line_number) {
    if(current_table->get_symbol(name) == nullptr){
        Symbol* new_symbol = new Symbol(name, VARIABLE, 1, 0, 0);
        if(get_register_counter() - 1 > user_registers){
            print_error("Out of registers", line_number);
        }
        new_symbol->is_iterator = true;
        new_symbol->is_initialized = true;
        new_symbol->is_pointer = false;
        current_table->add_symbol(new_symbol);
    }
    else{
        print_error("Variable " + name + " already declared", line_number);
    }
    variable* new_variable = new variable();
    new_variable->name = name;
    new_variable->type = VARIABLE;
    new_variable->register_number = current_table->get_symbol(name)->offset;
    new_variable->is_initialized = true;
    new_variable->is_iterator = true;
    new_variable->is_pointer = false;
    return new_variable;
}






#include <vector>
#include <string>
#include <iostream>
#include <fstream>

#include "utils.hpp"
#include "symbol_table.hpp"

#define AUX_REGISTER1 MAX_REGISTERS - 1 //MULARG1
#define AUX_REGISTER2 MAX_REGISTERS - 2 //MULARG2
#define AUX_REGISTER3 MAX_REGISTERS - 3 //RESULT, QUOTIENT
#define AUX_REGISTER4 MAX_REGISTERS - 4 //RTRN
#define AUX_REGISTER5 MAX_REGISTERS - 5 //FLAG1 SIGN1
#define AUX_REGISTER6 MAX_REGISTERS - 6 //REMINDER
#define AUX_REGISTER7 MAX_REGISTERS - 7 //FLAG2 SIGN2
#define AUX_REGISTER8 MAX_REGISTERS - 8 //TEMPB DIVISION
#define AUX_REGISTER9 MAX_REGISTERS - 9 //POWER DIVISION

#define CONST_ONE_REGISTER MAX_REGISTERS - 10
#define CONST_MINUS_ONE_REGISTER MAX_REGISTERS - 11
#define CONST_ZERO_REGISTER MAX_REGISTERS - 12

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
static std::vector<Symbol*> arrays = std::vector<Symbol*>();
static Symbol* last_left = nullptr;
static Symbol* last_right = nullptr;
static operation_t last_operation = NONE;

long int current_output_line = 1;

bool is_used[3] = {false, false, false};
bool is_multiplication = false;
bool is_division = false;
long long int multiplication_begining_line = 0;
long long int division_begining_line = 0;

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
        if(value->type == ARRAY){

            output_file << "LOAD " << value->array_pointer << "\n";
            if(value->is_index_variable_pointer){
                output_file << "ADDI " << value->index_variable_register << "\n";
            }
            else{
                output_file << "ADD " << value->index_variable_register << "\n";
            }
            output_file << "LOADI 0" << "\n";
            output_file << "PUT 0" << "\n";
            current_output_line += 4;
        }
    }
    else {
        if(value->type == VARIABLE){
            output_file << "LOADI " << value->register_number << "\n";
            output_file << "PUT 0" << "\n";
            current_output_line += 2;
        }
        if(value->type == ARRAY){

            if(value->is_static_reference){
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
                output_file << "ADD " << value->register_number << "\n";
            }
            else{
                output_file << "LOAD " << value->register_number << "\n";
                if(value->is_index_variable_pointer){
                    output_file << "ADDI " << value->index_variable_register << "\n";
                }
                else{
                    output_file << "ADD " << value->index_variable_register << "\n";
                }
            }

            output_file << "LOADI 0" << "\n";
            output_file << "PUT 0" << "\n";
            current_output_line += 4;
        }
    }

}

write_statement::write_statement(struct variable* value) {
    this->value = value;
}

command* create_write_statement(struct variable* value, int line_number) {
    write_statement* new_statement = new write_statement(value);

    if(value->is_initialized == false){
        print_error("Variable " + value->name + " not initialized", line_number);
    }

    if(value->is_pointer){
        if(value->type == VARIABLE){
            new_statement->lines_taken = 2;
        }
        else if(value->type == ARRAY){
            new_statement->lines_taken = 4;
        }
    }
    else{
        if(value->type == VALUE){
            new_statement->lines_taken = 2;
        }
        else if (value->type == VARIABLE){
            new_statement->lines_taken = 1;
        }
        else if(value->type == ARRAY){
            new_statement->lines_taken = 4;
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
        if(var->type == ARRAY){
            output_file << "LOAD " << var->array_pointer << "\n";
            if(var->is_index_variable_pointer){
                output_file << "ADDI " << var->index_variable_register << "\n";
            }
            else{
                output_file << "ADD " << var->index_variable_register << "\n";
            }
            output_file << "STORE " << AUX_REGISTER1 << "\n";
            output_file << "GET 0" << "\n";
            output_file << "STOREI " << AUX_REGISTER1 << "\n";
            current_output_line += 5;
        }
    }
    else{
        if(var->type == VARIABLE){
            output_file << "GET 0" << "\n";
            output_file << "STOREI " << var->register_number << "\n";
            current_output_line += 2;
        }
        if(var->type == ARRAY){
            if(var->is_static_reference){
                if(var->val == 1){
                    output_file << "LOAD " << CONST_ONE_REGISTER << "\n";
                }
                else if(var->val == -1){
                    output_file << "LOAD " << CONST_MINUS_ONE_REGISTER << "\n";
                }
                else if(var->val == 0){
                    output_file << "LOAD " << CONST_ZERO_REGISTER << "\n";
                }
                else{
                    output_file << "SET" << " " << var->val << "\n";
                }
                output_file << "ADD " << var->register_number << "\n";
            }
            else{
                output_file << "LOAD " << var->register_number << "\n";
                if(var->is_index_variable_pointer){
                    output_file << "ADDI " << var->index_variable_register << "\n";
                }
                else{
                    output_file << "ADD " << var->index_variable_register << "\n";
                }
            }
            output_file << "STORE " << AUX_REGISTER1 << "\n";
            output_file << "GET 0" << "\n";
            output_file << "STOREI " << AUX_REGISTER1 << "\n";
            current_output_line += 5;
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
        if(value->type == VARIABLE){
            new_statement->lines_taken = 2;
        }
        else if(value->type == ARRAY){
            new_statement->lines_taken = 5;
        }
    }
    else{
        if(value->type == VARIABLE){
            new_statement->lines_taken = 1;
        }
        else if(value->type == ARRAY){
            new_statement->lines_taken = 5;
        }
    }

    if(symbol == last_left || symbol == last_right){
        last_operation = NONE;
    }

    return new_statement;
}

/********************ADDITION*************************/

addition::addition(struct variable* left, struct variable* right) {
    this->left = left;
    this->right = right;
}

void addition::generate_code() {

    if(left->type == VALUE && right->type == VALUE && is_precalc){
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

    if(left->type == VARIABLE){
        if(left->is_pointer){
            output_file << "LOADI " << left->register_number << "\n";
            current_output_line++;
        }
        else{
            output_file << "LOAD " << left->register_number << "\n";
            current_output_line++;
        }
        output_file << "STORE " << AUX_REGISTER1 << "\n";
        current_output_line++;
    }
    else if (left->type == VALUE){
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
        output_file << "STORE " << AUX_REGISTER1 << "\n";
        current_output_line += 2;
    }
    else if(left->type == ARRAY){

        if(left->is_static_reference){
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
                output_file << "SET" << " " << left->val << "\n";
            }
            output_file << "ADD " << left->register_number << "\n";
        }
        else{
            if(left->is_pointer){
                output_file << "LOAD " << left->register_number << "\n";
            }
            else{
                output_file << "LOAD " << left->array_pointer << "\n";
            }
            if(left->is_index_variable_pointer){
                output_file << "ADDI " << left->index_variable_register << "\n";
            }
            else{
                output_file << "ADD " << left->index_variable_register << "\n";
            }
        }
        output_file << "LOADI 0" << "\n";
        output_file << "STORE " << AUX_REGISTER1 << "\n";
        current_output_line += 4;
    }

    if(right->type == VARIABLE){
        if(right->is_pointer){
            output_file << "LOADI " << right->register_number << "\n";
            current_output_line++;
        }
        else{
            output_file << "LOAD " << right->register_number << "\n";
            current_output_line++;
        }
        output_file << "ADD " << AUX_REGISTER1 << "\n";
        current_output_line++;
    }
    else if (right->type == VALUE){
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
        output_file << "ADD " << AUX_REGISTER1 << "\n";
        current_output_line++;
    }
    else if(right->type == ARRAY){
        if(right->is_static_reference){
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
                output_file << "SET" << " " << right->val << "\n";
            }
            output_file << "ADD " << right->register_number << "\n";
        }
        else{
            if(right->is_pointer){
                output_file << "LOAD " << right->register_number << "\n";
            }
            else{
                output_file << "LOAD " << right->array_pointer << "\n";
            }
            if(right->is_index_variable_pointer){
                output_file << "ADDI " << right->index_variable_register << "\n";
            }
            else{
                output_file << "ADD " << right->index_variable_register << "\n";
            }
        }
        output_file << "LOADI 0" << "\n";
        output_file << "ADD " << AUX_REGISTER1 << "\n";
        current_output_line += 4;
    }
}

expression* create_addition(struct variable* left, struct variable* right, int line_number) {
    addition* new_addition = new addition(left, right);
    if (left->type == VALUE && right->type == VALUE) {
        if(left->val >= 0 && MAX_NUMBER - left->val < right->val){
            new_addition->is_precalc = false;
        }
        else if(left->val < 0 && MIN_NUMBER - left->val > right->val){
            new_addition->is_precalc = false;
        }
        else{
            new_addition->is_precalc = true;
        }
    }
    else{
        if(left->is_initialized == false){
            print_error("Variable " + left->name + " not initialized", line_number);
        }
        if(right->is_initialized == false){
            print_error("Variable " + right->name + " not initialized", line_number);
        }
    }

    int lines_taken = 0;

    if(left->type == VALUE && right->type == VALUE && new_addition->is_precalc){
        lines_taken = 1;
    }
    else if (left->type == VARIABLE){
        lines_taken += 2;
    }
    else if (left->type == ARRAY){
        lines_taken += 4;
    }
    else if (left->type == VALUE && !new_addition->is_precalc){
        lines_taken += 2;
    }

    if(right->type == VARIABLE){
        lines_taken += 2;
    }
    else if (right->type == ARRAY){
        lines_taken += 4;
    }
    else if (right->type == VALUE && !new_addition->is_precalc){
        lines_taken += 2;
    }
    
    new_addition->lines_taken = lines_taken;
    return new_addition;
}

/*********************SUBTRACTION*************************/

subtraction::subtraction(struct variable* left, struct variable* right) {
    this->left = left;
    this->right = right;
}

void subtraction::generate_code() {

   if(left->type == VALUE && right->type == VALUE && is_precalc){
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

    if(right->type == VARIABLE){
        if(right->is_pointer){
            output_file << "LOADI " << right->register_number << "\n";
            current_output_line++;
        }
        else{
            output_file << "LOAD " << right->register_number << "\n";
            current_output_line++;
        }
        output_file << "STORE " << AUX_REGISTER1 << "\n";
        current_output_line++;
    }
    else if (right->type == VALUE){
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
        output_file << "STORE " << AUX_REGISTER1 << "\n";
        current_output_line++;
    }
    else if(right->type == ARRAY){
        if(right->is_static_reference){
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
                output_file << "SET" << " " << right->val << "\n";
            }
            output_file << "ADD " << right->register_number << "\n";
        }
        else{
            if(right->is_pointer){
                output_file << "LOAD " << right->register_number << "\n";
            }
            else{
                output_file << "LOAD " << right->array_pointer << "\n";
            }
            if(right->is_index_variable_pointer){
                output_file << "ADDI " << right->index_variable_register << "\n";
            }
            else{
                output_file << "ADD " << right->index_variable_register << "\n";
            }
        }
        output_file << "LOADI 0" << "\n";
        output_file << "STORE " << AUX_REGISTER1 << "\n";
        current_output_line += 4;
    }

    if(left->type == VARIABLE){
        if(left->is_pointer){
            output_file << "LOADI " << left->register_number << "\n";
            current_output_line++;
        }
        else{
            output_file << "LOAD " << left->register_number << "\n";
            current_output_line++;
        }
        output_file << "SUB " << AUX_REGISTER1 << "\n";
        current_output_line++;
    }
    else if (left->type == VALUE){
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
        output_file << "SUB " << AUX_REGISTER1 << "\n";
        current_output_line += 2;
    }
    else if(left->type == ARRAY){
        if(left->is_static_reference){
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
                output_file << "SET" << " " << left->val << "\n";
            }
            output_file << "ADD " << left->register_number << "\n";
        }
        else{
            if(left->is_pointer){
                output_file << "LOAD " << left->register_number << "\n";
            }
            else{
                output_file << "LOAD " << left->array_pointer << "\n";
            }
            if(left->is_index_variable_pointer){
                output_file << "ADDI " << left->index_variable_register << "\n";
            }
            else{
                output_file << "ADD " << left->index_variable_register << "\n";
            }
        }
        output_file << "LOADI 0" << "\n";
        output_file << "SUB " << AUX_REGISTER1 << "\n";
        current_output_line += 4;
    }
}

expression* create_subtraction(struct variable* left, struct variable* right, int line_number) {

    subtraction* new_subtraction = new subtraction(left, right);
    if (left->type == VALUE && right->type == VALUE) {
        if(left->val >= 0 && MAX_NUMBER + left->val < right->val){
            new_subtraction->is_precalc = false;
        }
        else if(left->val < 0 && MIN_NUMBER + left->val > right->val){
            new_subtraction->is_precalc = false;
        }
        else{
            new_subtraction->is_precalc = true;
        }
    }
    else{
        if(left->is_initialized == false){
            print_error("Variable " + left->name + " not initialized", line_number);
        }
        if(right->is_initialized == false){
            print_error("Variable " + right->name + " not initialized", line_number);
        }
    }

    int lines_taken = 0;

    if(left->type == VALUE && right->type == VALUE && new_subtraction->is_precalc){
        lines_taken = 1;
    }
    else if (left->type == VARIABLE){
        lines_taken += 2;
    }
    else if (left->type == ARRAY){
        lines_taken += 4;
    }
    else if (left->type == VALUE && !new_subtraction->is_precalc){
        lines_taken += 2;
    }

    if(right->type == VARIABLE){
        lines_taken += 2;
    }
    else if (right->type == ARRAY){
        lines_taken += 4;
    }
    else if (right->type == VALUE && !new_subtraction->is_precalc){
        lines_taken += 2;
    }

    new_subtraction->lines_taken = lines_taken;

    return new_subtraction;
}

/*********************MULTIPLICATION*************************/

multiplication::multiplication(struct variable* left, struct variable* right) {
    this->left = left;
    this->right = right;
}

expression* create_multiplication(struct variable* left, struct variable* right, int line_number) {
    multiplication* new_multiplication = new multiplication(left, right);

    if(left->is_initialized == false){
        print_error("Variable " + left->name + " not initialized", line_number);
    }
    if(right->is_initialized == false){
        print_error("Variable " + right->name + " not initialized", line_number);
    }

    Symbol* left_symbol = current_table->get_symbol(left->name);
    Symbol* right_symbol = current_table->get_symbol(right->name);


    int lines_taken = 0;

    if(left->type == VALUE && right->type == VALUE){
        if(left->val == 0 || right->val == 0){
            new_multiplication->is_precalc = true;
            new_multiplication->value = 0;
        }
        else if(left->val == 1){
            new_multiplication->is_precalc = true;
            new_multiplication->value = right->val;
        }
        else if(right->val == 1){
            new_multiplication->is_precalc = true;
            new_multiplication->value = left->val;
        }
        else if(left->val == -1){
            new_multiplication->is_precalc = true;
            new_multiplication->value = -right->val;
        }
        else if(right->val == -1){
            new_multiplication->is_precalc = true;
            new_multiplication->value = -left->val;
        }
        else{
            new_multiplication->is_precalc = false;
            is_multiplication = true;
        }
    }
    else{
        new_multiplication->is_precalc = false;
        is_multiplication = true;
    }

    if(left->type == VALUE && right->type == VALUE && new_multiplication->is_precalc){
        lines_taken = 1;
    }
    else if (left->type == VARIABLE){
        lines_taken += 2;
    }
    else if (left->type == ARRAY){
        lines_taken += 4;
    }
    else if (left->type == VALUE && !new_multiplication->is_precalc){
        lines_taken += 2;
    }

    if(right->type == VARIABLE){
        lines_taken += 2;
    }
    else if (right->type == ARRAY){
        lines_taken += 4;
    }
    else if (right->type == VALUE && !new_multiplication->is_precalc){
        lines_taken += 2;
    }

    is_used[0] = true;
    is_used[2] = true;

    new_multiplication->lines_taken = lines_taken + 3;

    if(last_left == left_symbol && last_right == right_symbol && last_operation == MULT){
        new_multiplication->lines_taken = 1;
        new_multiplication->is_repeated = true;
    }

    last_left = left_symbol;
    last_right = right_symbol;
    last_operation = MULT;

    return new_multiplication;
}

void multiplication::generate_code() {

    if(is_repeated){
        output_file << "LOAD " << AUX_REGISTER3 << "\n";
        current_output_line++;
        return;
    }

    if(left->type == VALUE && right->type == VALUE && is_precalc){
        if(value == 1){
            output_file << "LOAD " << CONST_ONE_REGISTER << "\n";
        }
        else if(value == -1){
            output_file << "LOAD " << CONST_MINUS_ONE_REGISTER << "\n";
        }
        else if(value == 0){
            output_file << "LOAD " << CONST_ZERO_REGISTER << "\n";
        }
        else{
            output_file << "SET " << value << "\n";
        }
        current_output_line++;
        return;
    }

    if(left->type == VARIABLE){
        if(left->is_pointer){
            output_file << "LOADI " << left->register_number << "\n";
            current_output_line++;
        }
        else{
            output_file << "LOAD " << left->register_number << "\n";
            current_output_line++;
        }
        output_file << "STORE " << AUX_REGISTER1 << "\n";
        current_output_line++;
    }
    else if (left->type == VALUE){
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
        output_file << "STORE " << AUX_REGISTER1 << "\n";
        current_output_line += 2;
    }
    else if(left->type == ARRAY){
        if(left->is_static_reference){
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
                output_file << "SET" << " " << left->val << "\n";
            }
            output_file << "ADD " << left->register_number << "\n";
        }
        else{
            if(left->is_pointer){
                output_file << "LOAD " << left->register_number << "\n";
            }
            else{
                output_file << "LOAD " << left->array_pointer << "\n";
            }
            if(left->is_index_variable_pointer){
                output_file << "ADDI " << left->index_variable_register << "\n";
            }
            else{
                output_file << "ADD " << left->index_variable_register << "\n";
            }
        }
        output_file << "LOADI 0" << "\n";
        output_file << "STORE " << AUX_REGISTER1 << "\n";
        current_output_line += 4;
    }

    if(right->type == VARIABLE){
        if(right->is_pointer){
            output_file << "LOADI " << right->register_number << "\n";
            current_output_line++;
        }
        else{
            output_file << "LOAD " << right->register_number << "\n";
            current_output_line++;
        }
        output_file << "STORE " << AUX_REGISTER2 << "\n";
        current_output_line++;
    }
    else if (right->type == VALUE){
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
        output_file << "STORE " << AUX_REGISTER2 << "\n";
        current_output_line++;
    }
    else if(right->type == ARRAY){
        if(right->is_static_reference){
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
                output_file << "SET" << " " << right->val << "\n";
            }
            output_file << "ADD " << right->register_number << "\n";
        }
        else{
            if(right->is_pointer){
                output_file << "LOAD " << right->register_number << "\n";
            }
            else{
                output_file << "LOAD " << right->array_pointer << "\n";
            }
            if(right->is_index_variable_pointer){
                output_file << "ADDI " << right->index_variable_register << "\n";
            }
            else{
                output_file << "ADD " << right->index_variable_register << "\n";
            }
        }
        output_file << "LOADI 0" << "\n";
        output_file << "STORE " << AUX_REGISTER2 << "\n";
        current_output_line += 4;
    }

    output_file << "SET " << current_output_line + 2 << "\n";
    current_output_line++;
    output_file << "STORE " << AUX_REGISTER4 << "\n";
    current_output_line++;
    output_file << "JUMP " << multiplication_begining_line - current_output_line + 2 << "\n";
    current_output_line++;
}

/*********************DIVISION*************************/

division::division(struct variable* left, struct variable* right) {
    this->left = left;
    this->right = right;
}

expression* create_division(struct variable* left, struct variable* right, int line_number) {
    division* new_division = new division(left, right);

    if(left->is_initialized == false){
        print_error("Variable " + left->name + " not initialized", line_number);
    }
    if(right->is_initialized == false){
        print_error("Variable " + right->name + " not initialized", line_number);
    }

    Symbol* left_symbol = current_table->get_symbol(left->name);
    Symbol* right_symbol = current_table->get_symbol(right->name);


    int lines_taken = 0;

    if(left->type == VALUE && right->type == VALUE){

        if(right->val == left->val){
            new_division->is_precalc = true;
            new_division->value = 1;
            if(right->val == 0){
                new_division->is_precalc = true;
                new_division->value = 0;
            }
        }
        else if(right->val == -left->val){
            new_division->is_precalc = true;
            new_division->value = -1;
        }
        else if(right->val == 0){
            new_division->is_precalc = true;
            new_division->value = 0;
        }
        else if(left->val == 0){
            new_division->is_precalc = true;
            new_division->value = 0;
        }
        else if(left->val < right->val){
            new_division->is_precalc = true;
            new_division->value = 0;
        }
        else{
            new_division->is_precalc = false;
            is_division = true;
        }
    }
    else{
        new_division->is_precalc = false;
        is_division = true;
    }

    if(left->type == VALUE && right->type == VALUE && new_division->is_precalc){
        lines_taken = 1;
    }
    else if (left->type == VARIABLE){
        lines_taken += 2;
    }
    else if (left->type == ARRAY){
        lines_taken += 4;
    }
    else if (left->type == VALUE && !new_division->is_precalc){
        lines_taken += 2;
    }

    if(right->type == VARIABLE){
        lines_taken += 2;
    }
    else if (right->type == ARRAY){
        lines_taken += 4;
    }
    else if (right->type == VALUE && !new_division->is_precalc){
        lines_taken += 2;
    }

    is_used[0] = true;
    is_used[1] = true;
    is_used[2] = true;

    new_division->lines_taken = lines_taken + 4;

    if(last_left == left_symbol && last_right == right_symbol && last_operation == DIVISION){
        new_division->lines_taken = 1;
        new_division->is_repeated = true;
    }

    last_left = left_symbol;
    last_right = right_symbol;
    last_operation = DIVISION;

    return new_division;
}

void division::generate_code() {

    if(is_repeated){
        output_file << "LOAD " << AUX_REGISTER3 << "\n";
        current_output_line++;
        return;
    }

    if(left->type == VALUE && right->type == VALUE && is_precalc){
        if(value == 1){
            output_file << "LOAD " << CONST_ONE_REGISTER << "\n";
        }
        else if(value == -1){
            output_file << "LOAD " << CONST_MINUS_ONE_REGISTER << "\n";
        }
        else if(value == 0){
            output_file << "LOAD " << CONST_ZERO_REGISTER << "\n";
        }
        else{
            output_file << "SET " << value << "\n";
        }
        current_output_line++;
        return;
    }

    if(left->type == VARIABLE){
        if(left->is_pointer){
            output_file << "LOADI " << left->register_number << "\n";
            current_output_line++;
        }
        else{
            output_file << "LOAD " << left->register_number << "\n";
            current_output_line++;
        }
        output_file << "STORE " << AUX_REGISTER1 << "\n";
        current_output_line++;
    }
    else if (left->type == VALUE){
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
        output_file << "STORE " << AUX_REGISTER1 << "\n";
        current_output_line += 2;
    }
    else if(left->type == ARRAY){
        if(left->is_static_reference){
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
                output_file << "SET" << " " << left->val << "\n";
            }
            output_file << "ADD " << left->register_number << "\n";
        }
        else{
            if(left->is_pointer){
                output_file << "LOAD " << left->register_number << "\n";
            }
            else{
                output_file << "LOAD " << left->array_pointer << "\n";
            }
            if(left->is_index_variable_pointer){
                output_file << "ADDI " << left->index_variable_register << "\n";
            }
            else{
                output_file << "ADD " << left->index_variable_register << "\n";
            }
        }
        output_file << "LOADI 0" << "\n";
        output_file << "STORE " << AUX_REGISTER1 << "\n";
        current_output_line += 4;
    }

    if(right->type == VARIABLE){
        if(right->is_pointer){
            output_file << "LOADI " << right->register_number << "\n";
            current_output_line++;
        }
        else{
            output_file << "LOAD " << right->register_number << "\n";
            current_output_line++;
        }
        output_file << "STORE " << AUX_REGISTER2 << "\n";
        current_output_line++;
    }
    else if (right->type == VALUE){
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
        output_file << "STORE " << AUX_REGISTER2 << "\n";
        current_output_line++;
    }
    else if(right->type == ARRAY){
        if(right->is_static_reference){
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
                output_file << "SET" << " " << right->val << "\n";
            }
            output_file << "ADD " << right->register_number << "\n";
        }
        else{
            if(right->is_pointer){
                output_file << "LOAD " << right->register_number << "\n";
            }
            else{
                output_file << "LOAD " << right->array_pointer << "\n";
            }
            if(right->is_index_variable_pointer){
                output_file << "ADDI " << right->index_variable_register << "\n";
            }
            else{
                output_file << "ADD " << right->index_variable_register << "\n";
            }
        }
        output_file << "LOADI 0" << "\n";
        output_file << "STORE " << AUX_REGISTER2 << "\n";
        current_output_line += 4;
    }

    output_file << "SET " << current_output_line + 2 << "\n";
    current_output_line++;
    output_file << "STORE " << AUX_REGISTER4 << "\n";
    current_output_line++;
    output_file << "JUMP " << division_begining_line - current_output_line + 2 << "\n";
    current_output_line++;

    output_file << "LOAD " << AUX_REGISTER3 << "\n";
    current_output_line++;
}

/*********************MODULO**********************/

modulo::modulo(struct variable* left, struct variable* right) {
    this->left = left;
    this->right = right;
}

expression* create_modulo(struct variable* left, struct variable* right, int line_number) {
    modulo* new_modulo = new modulo(left, right);

    if(left->is_initialized == false){
        print_error("Variable " + left->name + " not initialized", line_number);
    }
    if(right->is_initialized == false){
        print_error("Variable " + right->name + " not initialized", line_number);
    }

    Symbol* left_symbol = current_table->get_symbol(left->name);
    Symbol* right_symbol = current_table->get_symbol(right->name);

    int lines_taken = 0;

    if(left->type == VALUE && right->type == VALUE){

        if(right->val == left->val){
            new_modulo->is_precalc = true;
            new_modulo->value = 0;
        }
        else if(right->val == -left->val){
            new_modulo->is_precalc = true;
            new_modulo->value = 0;
        }
        else if(right->val == 0){
            new_modulo->is_precalc = true;
            new_modulo->value = 0;
        }
        else if(left->val == 0){
            new_modulo->is_precalc = true;
            new_modulo->value = 0;
        }
        else if(left->val < right->val){
            new_modulo->is_precalc = true;
            new_modulo->value = left->val;
        }
        else{
            new_modulo->is_precalc = false;
            is_division = true;
        }
    }
    else{
        new_modulo->is_precalc = false;
        is_division = true;
    }

    if(left->type == VALUE && right->type == VALUE && new_modulo->is_precalc){
        lines_taken = 1;
    }
    else if (left->type == VARIABLE){
        lines_taken += 2;
    }
    else if (left->type == ARRAY){
        lines_taken += 4;
    }
    else if (left->type == VALUE && !new_modulo->is_precalc){
        lines_taken += 2;
    }

    if(right->type == VARIABLE){
        lines_taken += 2;
    }
    else if (right->type == ARRAY){
        lines_taken += 4;
    }
    else if (right->type == VALUE && !new_modulo->is_precalc){
        lines_taken += 2;
    }

    is_used[0] = true;
    is_used[1] = true;
    is_used[2] = true;

    new_modulo->lines_taken = lines_taken + 4;

    if(last_left == left_symbol && last_right == right_symbol && last_operation == DIVISION){
        new_modulo->lines_taken = 1;
        new_modulo->is_repeated = true;
    }

    last_left = left_symbol;
    last_right = right_symbol;
    last_operation = DIVISION;

    return new_modulo;
}

void modulo::generate_code() {

    if(is_repeated){
        output_file << "LOAD " << AUX_REGISTER6 << "\n";
        current_output_line++;
        return;
    }

    if(left->type == VALUE && right->type == VALUE && is_precalc){
        if(value == 1){
            output_file << "LOAD " << CONST_ONE_REGISTER << "\n";
        }
        else if(value == -1){
            output_file << "LOAD " << CONST_MINUS_ONE_REGISTER << "\n";
        }
        else if(value == 0){
            output_file << "LOAD " << CONST_ZERO_REGISTER << "\n";
        }
        else{
            output_file << "SET " << value << "\n";
        }
        current_output_line++;
        return;
    }

    if(left->type == VARIABLE){
        if(left->is_pointer){
            output_file << "LOADI " << left->register_number << "\n";
            current_output_line++;
        }
        else{
            output_file << "LOAD " << left->register_number << "\n";
            current_output_line++;
        }
        output_file << "STORE " << AUX_REGISTER1 << "\n";
        current_output_line++;
    }
    else if (left->type == VALUE){
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
        output_file << "STORE " << AUX_REGISTER1 << "\n";
        current_output_line += 2;
    }
    else if(left->type == ARRAY){
        if(left->is_static_reference){
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
                output_file << "SET" << " " << left->val << "\n";
            }
            output_file << "ADD " << left->register_number << "\n";
        }
        else{
            if(left->is_pointer){
                output_file << "LOAD " << left->register_number << "\n";
            }
            else{
                output_file << "LOAD " << left->array_pointer << "\n";
            }
            if(left->is_index_variable_pointer){
                output_file << "ADDI " << left->index_variable_register << "\n";
            }
            else{
                output_file << "ADD " << left->index_variable_register << "\n";
            }
        }
        output_file << "LOADI 0" << "\n";
        output_file << "STORE " << AUX_REGISTER1 << "\n";
        current_output_line += 4;
    }

    if(right->type == VARIABLE){
        if(right->is_pointer){
            output_file << "LOADI " << right->register_number << "\n";
            current_output_line++;
        }
        else{
            output_file << "LOAD " << right->register_number << "\n";
            current_output_line++;
        }
        output_file << "STORE " << AUX_REGISTER2 << "\n";
        current_output_line++;
    }
    else if (right->type == VALUE){
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
        output_file << "STORE " << AUX_REGISTER2 << "\n";
        current_output_line++;
    }
    else if(right->type == ARRAY){
        if(right->is_static_reference){
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
                output_file << "SET" << " " << right->val << "\n";
            }
            output_file << "ADD " << right->register_number << "\n";
        }
        else{
            if(right->is_pointer){
                output_file << "LOAD " << right->register_number << "\n";
            }
            else{
                output_file << "LOAD " << right->array_pointer << "\n";
            }
            if(right->is_index_variable_pointer){
                output_file << "ADDI " << right->index_variable_register << "\n";
            }
            else{
                output_file << "ADD " << right->index_variable_register << "\n";
            }
        }
        output_file << "LOADI 0" << "\n";
        output_file << "STORE " << AUX_REGISTER2 << "\n";
        current_output_line += 4;
    }

    output_file << "SET " << current_output_line + 2 << "\n";
    current_output_line++;
    output_file << "STORE " << AUX_REGISTER4 << "\n";
    current_output_line++;
    output_file << "JUMP " << division_begining_line - current_output_line + 2 << "\n";
    current_output_line++;

    output_file << "LOAD " << AUX_REGISTER6 << "\n";
    current_output_line++;
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
    else if(left->type == VARIABLE){
        if(left->is_pointer){
            output_file << "LOADI " << left->register_number << "\n";
        }
        else{
            output_file << "LOAD " << left->register_number << "\n";
        }
        current_output_line++;
    }
    else if(left->type == ARRAY){
        if(left->is_static_reference){
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
                output_file << "SET" << " " << left->val << "\n";
            }
            output_file << "ADD " << left->register_number << "\n";
        }
        else{
            if(left->is_pointer){
                output_file << "LOAD " << left->register_number << "\n";
            }
            else{
                output_file << "LOAD " << left->array_pointer << "\n";
            }
            if(left->is_index_variable_pointer){
                output_file << "ADDI " << left->index_variable_register << "\n";
            }
            else{
                output_file << "ADD " << left->index_variable_register << "\n";
            }
        }
        output_file << "LOADI 0" << "\n";
        current_output_line += 3;
    }
}

expression* pass_variable_as_expression(struct variable* value, int line_number) {
    value_expression* new_value = new value_expression(value);
    
    if(value->type == ARRAY){
        new_value->lines_taken = 3;
    }
    else {
        new_value->lines_taken = 1;
    }

    if(value->is_initialized == false){
        print_error("Variable " + value->name + " not initialized", line_number);
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

    if(left->type == VARIABLE){
        if(left->is_pointer){
            output_file << "STOREI " << left->register_number << "\n";
        }
        else{
            output_file << "STORE " << left->register_number << "\n";
        }
        current_output_line++;
    }
    else if(left->type == ARRAY){
        output_file << "STORE " << AUX_REGISTER1 << "\n";
        current_output_line++;
        if(left->is_static_reference){
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
                output_file << "SET" << " " << left->val << "\n";
            }
            output_file << "ADD " << left->register_number << "\n";
        }
        else{
            if(left->is_pointer){
                output_file << "LOAD " << left->register_number << "\n";
            }
            else{
                output_file << "LOAD " << left->array_pointer << "\n";
            }
            if(left->is_index_variable_pointer){
                output_file << "ADDI " << left->index_variable_register << "\n";
            }
            else{
                output_file << "ADD " << left->index_variable_register << "\n";
            }
        }
        output_file << "STORE " << AUX_REGISTER2 << "\n";
        output_file << "LOAD " << AUX_REGISTER1 << "\n";
        output_file << "STOREI " << AUX_REGISTER2 << "\n";
        current_output_line += 5;
    }
}

command* create_assignment(struct variable* left, expression* right, int line_number) {
    if(left->type == VALUE){
        print_error("Cannot assign variable to a number", line_number);
    }
    assignment* new_assignment = new assignment(left, right);
    
    if(left->type == ARRAY){
        new_assignment->lines_taken = 6;
    }
    else{
        new_assignment->lines_taken = 1;
    }

    new_assignment->lines_taken += right->lines_taken;

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

    if(symbol == last_left || symbol == last_right){
        last_operation = NONE;
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

    int lines_taken = 1;
    
    if(left->type == VALUE){
        lines_taken+=2;
    }
    else if(left->type == VARIABLE){
        lines_taken+=2;
    }
    else if(left->type == ARRAY){
        lines_taken += 4;
    }

    if(right->type == VALUE){
        lines_taken+=2;
    }
    else if(right->type == VARIABLE){
        lines_taken+=2;
    }
    else if(right->type == ARRAY){
        lines_taken += 4;
    }

    new_condition->lines_taken = lines_taken;

    new_condition->type = COND_EQ;

    if(left->is_initialized == false){
        print_error("Variable " + left->name + " not initialized", line_number);
    }
    if(right->is_initialized == false){
        print_error("Variable " + right->name + " not initialized", line_number);
    }

    return new_condition;
}

void eq_condition::generate_code() {
    
    if (left->type == VALUE) {
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
        output_file << "STORE " << AUX_REGISTER1 << "\n";
        current_output_line++;
    }
    else if (left->type == VARIABLE) {
        if(left->is_pointer){
            output_file << "LOADI " << left->register_number << "\n";
            current_output_line++;
        }
        else{
            output_file << "LOAD " << left->register_number << "\n";
            current_output_line++;
        }
        output_file << "STORE " << AUX_REGISTER1 << "\n";
        current_output_line++;
    }
    else if (left->type == ARRAY) {
        if(left->is_static_reference){
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
                output_file << "SET" << " " << left->val << "\n";
            }
            output_file << "ADD " << left->register_number << "\n";
        }
        else{
            if(left->is_pointer){
                output_file << "LOAD " << left->register_number << "\n";
            }
            else{
                output_file << "LOAD " << left->array_pointer << "\n";
            }
            if(left->is_index_variable_pointer){
                output_file << "ADDI " << left->index_variable_register << "\n";
            }
            else{
                output_file << "ADD " << left->index_variable_register << "\n";
            }
        }
        output_file << "LOADI 0" << "\n";
        output_file << "STORE " << AUX_REGISTER1 << "\n";
        current_output_line += 4;
    }

    if (right->type == VALUE) {
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
        output_file << "SUB " << AUX_REGISTER1 << "\n";
        current_output_line++;
    }
    else if (right->type == VARIABLE) {
        if(right->is_pointer){
            output_file << "LOADI " << right->register_number << "\n";
            current_output_line++;
        }
        else{
            output_file << "LOAD " << right->register_number << "\n";
            current_output_line++;
        }
        output_file << "SUB " << AUX_REGISTER1 << "\n";
        current_output_line++;
    }
    else if (right->type == ARRAY) {
        if(right->is_static_reference){
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
                output_file << "SET" << " " << right->val << "\n";
            }
            output_file << "ADD " << right->register_number << "\n";
        }
        else{
            if(right->is_pointer){
                output_file << "LOAD " << right->register_number << "\n";
            }
            else{
                output_file << "LOAD " << right->array_pointer << "\n";
            }
            if(right->is_index_variable_pointer){
                output_file << "ADDI " << right->index_variable_register << "\n";
            }
            else{
                output_file << "ADD " << right->index_variable_register << "\n";
            }
        }
        output_file << "LOADI 0" << "\n";
        output_file << "SUB " << AUX_REGISTER1 << "\n";
        current_output_line += 4;
    }

    output_file << "JZERO 2" << "\n";
    current_output_line++;
}

/*******************NEQ CONDITION************************/

condition* create_neq_condition(struct variable* left, struct variable* right, int line_number){

    neq_condition* new_condition = new neq_condition(left, right);

    int lines_taken = 0;
    
    if(left->type == VALUE){
        lines_taken+=2;
    }
    else if(left->type == VARIABLE){
        lines_taken+=2;
    }
    else if(left->type == ARRAY){
        lines_taken += 4;
    }

    if(right->type == VALUE){
        lines_taken+=2;
    }
    else if(right->type == VARIABLE){
        lines_taken+=2;
    }
    else if(right->type == ARRAY){
        lines_taken += 4;
    }

    new_condition->lines_taken = lines_taken;

    new_condition->type = COND_NEQ;

    if(left->is_initialized == false){
        print_error("Variable " + left->name + " not initialized", line_number);
    }
    if(right->is_initialized == false){
        print_error("Variable " + right->name + " not initialized", line_number);
    }

    return new_condition;
}


void neq_condition::generate_code() {

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
        output_file << "STORE " << AUX_REGISTER1 << "\n";
        current_output_line++;
    }
    else if(left->type == VARIABLE){
        if(left->is_pointer){
            output_file << "LOADI " << left->register_number << "\n";
            current_output_line++;
        }
        else{
            output_file << "LOAD " << left->register_number << "\n";
            current_output_line++;
        }
        output_file << "STORE " << AUX_REGISTER1 << "\n";
        current_output_line++;
    }
    else if(left->type == ARRAY){
        if(left->is_static_reference){
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
                output_file << "SET" << " " << left->val << "\n";
            }
            output_file << "ADD " << left->register_number << "\n";
        }
        else{
            if(left->is_pointer){
                output_file << "LOAD " << left->register_number << "\n";
            }
            else{
                output_file << "LOAD " << left->array_pointer << "\n";
            }
            if(left->is_index_variable_pointer){
                output_file << "ADDI " << left->index_variable_register << "\n";
            }
            else{
                output_file << "ADD " << left->index_variable_register << "\n";
            }
        }
        output_file << "LOADI 0" << "\n";
        output_file << "STORE " << AUX_REGISTER1 << "\n";
        current_output_line += 4;
    }

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
        output_file << "SUB " << AUX_REGISTER1 << "\n";
        current_output_line++;
    }
    else if(right->type == VARIABLE){
        if(right->is_pointer){
            output_file << "LOADI " << right->register_number << "\n";
            current_output_line++;
        }
        else{
            output_file << "LOAD " << right->register_number << "\n";
            current_output_line++;
        }
        output_file << "SUB " << AUX_REGISTER1 << "\n";
        current_output_line++;
    }
    else if(right->type == ARRAY){
        if(right->is_static_reference){
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
                output_file << "SET" << " " << right->val << "\n";
            }
            output_file << "ADD " << right->register_number << "\n";
        }
        else{
            if(right->is_pointer){
                output_file << "LOAD " << right->register_number << "\n";
            }
            else{
                output_file << "LOAD " << right->array_pointer << "\n";
            }
            if(right->is_index_variable_pointer){
                output_file << "ADDI " << right->index_variable_register << "\n";
            }
            else{
                output_file << "ADD " << right->index_variable_register << "\n";
            }
        }
        output_file << "LOADI 0" << "\n";
        output_file << "SUB " << AUX_REGISTER1 << "\n";
        current_output_line += 4;
    }
}

/*******************LT CONDITION************************/

condition* create_lt_condition(struct variable* left, struct variable* right, int line_number){

    lt_condition* new_condition = new lt_condition(left, right);
    
    int lines_taken = 1;

    if(left->type == VALUE){
        lines_taken+=2;
    }
    else if(left->type == VARIABLE){
        lines_taken+=2;
    }
    else if(left->type == ARRAY){
        lines_taken += 4;
    }

    if(right->type == VALUE){
        lines_taken+=2;
    }
    else if(right->type == VARIABLE){
        lines_taken+=2;
    }
    else if(right->type == ARRAY){
        lines_taken += 4;
    }

    new_condition->lines_taken = lines_taken;

    new_condition->type = COND_LT;

    if(left->is_initialized == false){
        print_error("Variable " + left->name + " not initialized", line_number);
    }
    if(right->is_initialized == false){
        print_error("Variable " + right->name + " not initialized", line_number);
    }

    return new_condition;
}

void lt_condition::generate_code(void){
    
    if(right->type == VARIABLE){
        if(right->is_pointer){
            output_file << "LOADI " << right->register_number << "\n";
            current_output_line++;
        }
        else{
            output_file << "LOAD " << right->register_number << "\n";
            current_output_line++;
        }
        output_file << "STORE " << AUX_REGISTER1 << "\n";
        current_output_line++;
    }
    else if (right->type == VALUE){
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
        output_file << "STORE " << AUX_REGISTER1 << "\n";
        current_output_line++;
    }
    else if(right->type == ARRAY){
        if(right->is_static_reference){
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
                output_file << "SET" << " " << right->val << "\n";
            }
            output_file << "ADD " << right->register_number << "\n";
        }
        else{
            if(right->is_pointer){
                output_file << "LOAD " << right->register_number << "\n";
            }
            else{
                output_file << "LOAD " << right->array_pointer << "\n";
            }
            if(right->is_index_variable_pointer){
                output_file << "ADDI " << right->index_variable_register << "\n";
            }
            else{
                output_file << "ADD " << right->index_variable_register << "\n";
            }
        }
        output_file << "LOADI 0" << "\n";
        output_file << "STORE " << AUX_REGISTER1 << "\n";
        current_output_line += 4;
    }

    if(left->type == VARIABLE){
        if(left->is_pointer){
            output_file << "LOADI " << left->register_number << "\n";
            current_output_line++;
        }
        else{
            output_file << "LOAD " << left->register_number << "\n";
            current_output_line++;
        }
        output_file << "SUB " << AUX_REGISTER1 << "\n";
        current_output_line++;
    }
    else if (left->type == VALUE){
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
        output_file << "SUB " << AUX_REGISTER1 << "\n";
        current_output_line += 2;
    }
    else if(left->type == ARRAY){
        if(left->is_static_reference){
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
                output_file << "SET" << " " << left->val << "\n";
            }
            output_file << "ADD " << left->register_number << "\n";
        }
        else{
            if(left->is_pointer){
                output_file << "LOAD " << left->register_number << "\n";
            }
            else{
                output_file << "LOAD " << left->array_pointer << "\n";
            }
            if(left->is_index_variable_pointer){
                output_file << "ADDI " << left->index_variable_register << "\n";
            }
            else{
                output_file << "ADD " << left->index_variable_register << "\n";
            }
        }
        output_file << "LOADI 0" << "\n";
        output_file << "SUB " << AUX_REGISTER1 << "\n";
        current_output_line += 4;
    }

    output_file << "JNEG 2" << "\n";
    current_output_line++;
}

/*******************LEQ CONDITION************************/

condition* create_leq_condition(struct variable* left, struct variable* right, int line_number){

    leq_condition* new_condition = new leq_condition(left, right);
    
    int lines_taken = 0;

    if(left->type == VALUE){
        lines_taken+=2;
    }
    else if(left->type == VARIABLE){
        lines_taken+=2;
    }
    else if(left->type == ARRAY){
        lines_taken += 4;
    }

    if(right->type == VALUE){
        lines_taken+=2;
    }
    else if(right->type == VARIABLE){
        lines_taken+=2;
    }
    else if(right->type == ARRAY){
        lines_taken += 4;
    }

    new_condition->lines_taken = lines_taken;

    new_condition->type = COND_LEQ;

    if(left->is_initialized == false){
        print_error("Variable " + left->name + " not initialized", line_number);
    }
    if(right->is_initialized == false){
        print_error("Variable " + right->name + " not initialized", line_number);
    }

    return new_condition;
}

void leq_condition::generate_code(void){
    
    if(right->type == VARIABLE){
        if(right->is_pointer){
            output_file << "LOADI " << right->register_number << "\n";
            current_output_line++;
        }
        else{
            output_file << "LOAD " << right->register_number << "\n";
            current_output_line++;
        }
        output_file << "STORE " << AUX_REGISTER1 << "\n";
        current_output_line++;
    }
    else if (right->type == VALUE){
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
        output_file << "STORE " << AUX_REGISTER1 << "\n";
        current_output_line++;
    }
    else if(right->type == ARRAY){
        if(right->is_static_reference){
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
                output_file << "SET" << " " << right->val << "\n";
            }
            output_file << "ADD " << right->register_number << "\n";
        }
        else{
            if(right->is_pointer){
                output_file << "LOAD " << right->register_number << "\n";
            }
            else{
                output_file << "LOAD " << right->array_pointer << "\n";
            }
            if(right->is_index_variable_pointer){
                output_file << "ADDI " << right->index_variable_register << "\n";
            }
            else{
                output_file << "ADD " << right->index_variable_register << "\n";
            }
        }
        output_file << "LOADI 0" << "\n";
        output_file << "STORE " << AUX_REGISTER1 << "\n";
        current_output_line += 4;
    }

    if(left->type == VARIABLE){
        if(left->is_pointer){
            output_file << "LOADI " << left->register_number << "\n";
            current_output_line++;
        }
        else{
            output_file << "LOAD " << left->register_number << "\n";
            current_output_line++;
        }
        output_file << "SUB " << AUX_REGISTER1 << "\n";
        current_output_line++;
    }
    else if (left->type == VALUE){
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
        output_file << "SUB " << AUX_REGISTER1 << "\n";
        current_output_line += 2;
    }
    else if(left->type == ARRAY){
        if(left->is_static_reference){
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
                output_file << "SET" << " " << left->val << "\n";
            }
            output_file << "ADD " << left->register_number << "\n";
        }
        else{
            if(left->is_pointer){
                output_file << "LOAD " << left->register_number << "\n";
            }
            else{
                output_file << "LOAD " << left->array_pointer << "\n";
            }
            if(left->is_index_variable_pointer){
                output_file << "ADDI " << left->index_variable_register << "\n";
            }
            else{
                output_file << "ADD " << left->index_variable_register << "\n";
            }
        }
        output_file << "LOADI 0" << "\n";
        output_file << "SUB " << AUX_REGISTER1 << "\n";
        current_output_line += 4;
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
        output_file << "JPOS " << lines_taken - condition->lines_taken << "\n";
        current_output_line++;
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
        output_file << "JPOS " << if_lines_taken + 2 << "\n";
        current_output_line++;
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

    new_while->lines_taken = condition->lines_taken + lines_taken + 2;
    return new_while;
}

void while_statement::generate_code() {

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
        output_file << "JPOS " << lines_taken - condition->lines_taken << "\n";
        current_output_line++;
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
        output_file << "JPOS -" << lines_taken - 1 << "\n";
        current_output_line++;
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

    
    int lines_taken = 0;
    for(command* command : *commands){
        lines_taken += command->lines_taken;
    }
    new_for->lines_taken = lines_taken + 11;

    if(end->type == ARRAY){
        new_for->lines_taken += 2;
    }

    if(start->type == ARRAY){
        new_for->lines_taken += 2;
    }

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
    else if (start->type == VARIABLE){
        if(start->is_pointer){
            output_file << "LOADI " << start->register_number << "\n";
            current_output_line++;
        }
        else{
            output_file << "LOAD " << start->register_number << "\n";
            current_output_line++;
        }
    }
    else if(start->type == ARRAY){
        if(start->is_static_reference){
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
                output_file << "SET" << " " << start->val << "\n";
            }
            output_file << "ADD " << start->register_number << "\n";
        }
        else{
            if(start->is_pointer){
                output_file << "LOAD " << start->register_number << "\n";
            }
            else{
                output_file << "LOAD " << start->array_pointer << "\n";
            }
            if(start->is_index_variable_pointer){
                output_file << "ADDI " << start->index_variable_register << "\n";
            }
            else{
                output_file << "ADD " << start->index_variable_register << "\n";
            }
        }
        output_file << "LOADI 0" << "\n";
        current_output_line += 3;
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
            output_file << "SET " << end->val << "\n";
        }
        current_output_line++;
    }
    else if (end->type == VARIABLE){
        if(end->is_pointer){
            output_file << "LOADI " << end->register_number << "\n";
            current_output_line++;
        }
        else{
            output_file << "LOAD " << end->register_number << "\n";
            current_output_line++;
        }
    }
    else if(end->type == ARRAY){
        if(end->is_static_reference){
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
                output_file << "SET" << " " << end->val << "\n";
            }
            output_file << "ADD " << end->register_number << "\n";
        }
        else{
            if(end->is_pointer){
                output_file << "LOAD " << end->register_number << "\n";
            }
            else{
                output_file << "LOAD " << end->array_pointer << "\n";
            }
            if(end->is_index_variable_pointer){
                output_file << "ADDI " << end->index_variable_register << "\n";
            }
            else{
                output_file << "ADD " << end->index_variable_register << "\n";
            }
        }
        output_file << "LOADI 0" << "\n";
        current_output_line += 3;
    }

    output_file << "STORE " << end_register << "\n";
    current_output_line++;

    int jump_offset = 6;

    if(end->type == ARRAY){
        jump_offset += 2;
    }

    if(start->type == ARRAY){
        jump_offset += 2;
    }

    if(direction == UP){
        
        output_file << "LOAD " << end_register << "\n";
        output_file << "SUB " << iterator->register_number << "\n";
        output_file << "JNEG " << lines_taken - jump_offset << "\n";
        current_output_line+=3;

    }
    else if(direction == DOWN){

        output_file << "LOAD " << end_register << "\n";
        output_file << "SUB " << iterator->register_number << "\n";
        output_file << "JPOS " << lines_taken - jump_offset << "\n";
        current_output_line+=3;
    }

    for(command* command : *commands){
        command->generate_code();
    }

    if(direction == UP){
        output_file << "LOAD " << iterator->register_number << "\n";
        output_file << "ADD " << CONST_ONE_REGISTER << "\n";
        output_file << "STORE " << iterator->register_number << "\n";
        output_file << "JUMP -" << lines_taken - jump_offset + 1 << "\n";
        current_output_line+=4;
    }
    else if(direction == DOWN){
        output_file << "LOAD " << iterator->register_number << "\n";
        output_file << "SUB " << CONST_ONE_REGISTER << "\n";
        output_file << "STORE " << iterator->register_number << "\n";
        output_file << "JUMP -" << lines_taken - jump_offset + 1 << "\n";
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

    if(symbol->offset < 0){
        print_error("Negative memory adress", line_number);
    }

    new_variable->name = name;
    new_variable->type = VARIABLE;
    new_variable->register_number = symbol->offset;
    new_variable->is_initialized = symbol->is_initialized;
    new_variable->is_iterator = false;
    new_variable->is_pointer = symbol->is_pointer;
    return new_variable;
}

struct variable* create_array_variable(std::string name, long long int index, int line_number) {
    if(current_table->get_symbol(name) == nullptr){
        print_error("Variable " + name + " not declared", line_number);
        return nullptr;
    }
    variable* new_variable = new variable();

    Symbol* symbol = current_table->get_symbol(name);

    if(symbol->type == VARIABLE){
        print_error("Variable " + name + " is not an array", line_number);
        return nullptr;
    }

    if(!symbol->is_pointer && (index < symbol->start_index || index > symbol->end_index)){
        print_error("Index out of bounds", line_number);
    }

    if(symbol->offset + index < 0){
        print_error("Negative memory adress", line_number);
    }

    new_variable->name = name;
    if(symbol->is_pointer){
        new_variable->type = ARRAY;
        new_variable->register_number = symbol->offset;
        new_variable->is_static_reference = true;
        new_variable->val = index;
    }
    else{
        new_variable->type = VARIABLE;
        new_variable->register_number = symbol->offset + index;
    }
    new_variable->is_initialized = symbol->is_initialized;
    new_variable->is_iterator = false;
    new_variable->is_pointer = symbol->is_pointer;
    return new_variable;
}

struct variable* create_array_variable(std::string name, std::string index_variable, int line_number) {
    if(current_table->get_symbol(name) == nullptr){
        print_error("Variable " + name + " not declared", line_number);
        return nullptr;
    }
    if(current_table->get_symbol(index_variable) == nullptr){
        print_error("Variable " + index_variable + " not declared", line_number);
        return nullptr;
    }
    variable* new_variable = new variable();

    Symbol* symbol = current_table->get_symbol(name);
    Symbol* index_symbol = current_table->get_symbol(index_variable);


    if(symbol->type != ARRAY){
        print_error("Variable " + name + " is not an array", line_number);
        return nullptr;
    }

    if(index_symbol->is_initialized == false){
        print_error("Variable " + index_variable + " not initialized", line_number);
    }

    if(symbol->offset < 0){
        print_error("Negative memory adress", line_number);
    }

    if(index_symbol->offset < 0){
        print_error("Negative memory adress", line_number);
    }

    new_variable->name = name;
    new_variable->type = ARRAY;
    new_variable->register_number = symbol->offset;
    new_variable->is_initialized = true;
    new_variable->is_iterator = false;
    new_variable->is_pointer = symbol->is_pointer;
    new_variable->index_variable_register = index_symbol->offset;
    new_variable->is_index_variable_pointer = index_symbol->is_pointer;
    new_variable->array_pointer = symbol->array_pointer;
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

        if(new_symbol->offset < 0){
            print_error("Negative memory adress", line_number);
        }

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

        if(new_symbol->offset < 0){
            print_error("Negative memory adress", line_number);
        }

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

    new_procedure->begining_line = offset + 1;

    procedures.push_back(new_procedure);

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

        if(parameter == last_left || parameter == last_right){
            last_operation = NONE;
        }

        variable* new_parameter = new variable();
        new_parameter->name = parameter->name;
        new_parameter->type = parameter->type;

        if(parameter->offset < 0){
            print_error("Negative memory adress", line_number);
        }

        new_parameter->register_number = parameter->offset;
        if(new_parameter->register_number == 1){
            is_used[0] = true;
        }
        new_parameter->is_initialized = true;
        new_parameter->is_iterator = false;
        new_parameter->is_pointer = parameter->is_pointer;

        parameter->is_initialized = true;

        if(parameter->type == ARRAY){
            new_parameter->array_pointer = parameter->array_pointer;
        }

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
            else if(params->at(i)->type == ARRAY){
                output_file << "LOAD " << params->at(i)->array_pointer << "\n";
                current_output_line++;
            }
        }
        else{
            if(params->at(i)->type == VARIABLE){
                output_file << "SET " << params->at(i)->register_number << "\n";
                current_output_line++;
            }
            else if(params->at(i)->type == ARRAY){
                output_file << "LOAD " << params->at(i)->array_pointer << "\n";
                current_output_line++;
            }
        }
        output_file << "STORE " << proc->parameters->at(i)->register_number << "\n";
        current_output_line++;
    }
    output_file << "SET " << current_output_line + 2 << "\n";
    output_file << "STORE " << proc->return_position << "\n";
    current_output_line += 2;
    output_file << "JUMP " << proc->begining_line - current_output_line << "\n";
    current_output_line++;
}

/********************************COMPILER*****************************************/

void generate_multiplication(void){

    output_file << "LOAD " << CONST_ZERO_REGISTER << "\n";
    output_file << "STORE " << AUX_REGISTER3 << "\n";
    output_file << "STORE " << AUX_REGISTER5 << "\n";
    output_file << "LOAD " << AUX_REGISTER2 << "\n";
    output_file << "JPOS 13" << "\n";
    output_file << "LOAD " << CONST_ZERO_REGISTER << "\n";
    output_file << "SUB " << AUX_REGISTER2 << "\n";
    output_file << "STORE " << AUX_REGISTER2 << "\n";
    output_file << "LOAD " << AUX_REGISTER1 << "\n";
    output_file << "JNEG 4" << "\n";
    output_file << "LOAD " << CONST_ONE_REGISTER << "\n";
    output_file << "STORE " << AUX_REGISTER5 << "\n";
    output_file << "JUMP 4" << "\n";
    output_file << "LOAD " << CONST_ZERO_REGISTER << "\n";
    output_file << "SUB " << AUX_REGISTER1 << "\n";
    output_file << "STORE " << AUX_REGISTER1 << "\n";
    output_file << "JUMP 8" << "\n";
    output_file << "LOAD " << AUX_REGISTER1 << "\n";
    output_file << "JPOS 6" << "\n";
    output_file << "LOAD " << CONST_ZERO_REGISTER << "\n";
    output_file << "SUB " << AUX_REGISTER1 << "\n";
    output_file << "STORE " << AUX_REGISTER1 << "\n";
    output_file << "LOAD " << CONST_ONE_REGISTER << "\n";
    output_file << "STORE " << AUX_REGISTER5 << "\n";
    output_file << "LOAD " << AUX_REGISTER1 << "\n";
    output_file << "JPOS 2" << "\n";
    output_file << "JUMP 15" << "\n";
    output_file << "HALF" << "\n";
    output_file << "ADD 0" << "\n";
    output_file << "SUB " << AUX_REGISTER1 << "\n";
    output_file << "JZERO 4" << "\n";
    output_file << "LOAD " << AUX_REGISTER3 << "\n";
    output_file << "ADD " << AUX_REGISTER2 << "\n";
    output_file << "STORE " << AUX_REGISTER3 << "\n";
    output_file << "LOAD " << AUX_REGISTER2 << "\n";
    output_file << "ADD " << AUX_REGISTER2 << "\n";
    output_file << "STORE " << AUX_REGISTER2 << "\n";
    output_file << "LOAD " << AUX_REGISTER1 << "\n";
    output_file << "HALF" << "\n";
    output_file << "STORE " << AUX_REGISTER1 << "\n";
    output_file << "JUMP -15" << "\n";
    output_file << "LOAD " << AUX_REGISTER5 << "\n";
    output_file << "JZERO 4" << "\n";
    output_file << "LOAD " << CONST_ZERO_REGISTER << "\n";
    output_file << "SUB " << AUX_REGISTER3 << "\n";
    output_file << "STORE " << AUX_REGISTER3 << "\n";
    output_file << "LOAD " << AUX_REGISTER3 << "\n";
    output_file << "RTRN " << AUX_REGISTER4 << "\n";
    current_output_line += 48;
}


void generate_division(void){

    //init
    output_file << "LOAD " << CONST_ZERO_REGISTER << "\n";
    output_file << "STORE " << AUX_REGISTER3 << "\n";
    output_file << "STORE " << AUX_REGISTER5 << "\n";
    output_file << "STORE " << AUX_REGISTER7 << "\n";

    //check if b == 0
    output_file << "SUB " << AUX_REGISTER2 << "\n";
    output_file << "JZERO 2" << "\n";
    output_file << "JUMP 3" << "\n";
    output_file << "STORE " << AUX_REGISTER6 << "\n";
    output_file << "JUMP 74" << "\n";

    //check a < 0
    output_file << "LOAD " << AUX_REGISTER1 << "\n";
    output_file << "JPOS 6" << "\n";
    output_file << "LOAD " << CONST_ONE_REGISTER << "\n";
    output_file << "STORE " << AUX_REGISTER5 << "\n";
    output_file << "LOAD " << CONST_ZERO_REGISTER << "\n";
    output_file << "SUB " << AUX_REGISTER1 << "\n";
    output_file << "STORE " << AUX_REGISTER1 << "\n";

    //check b < 0
    output_file << "LOAD " << AUX_REGISTER2 << "\n";
    output_file << "JPOS 6" << "\n";
    output_file << "LOAD " << CONST_ONE_REGISTER << "\n";
    output_file << "STORE " << AUX_REGISTER7 << "\n";
    output_file << "LOAD " << CONST_ZERO_REGISTER << "\n";
    output_file << "SUB " << AUX_REGISTER2 << "\n";
    output_file << "STORE " << AUX_REGISTER2 << "\n";

    //r = a
    output_file << "LOAD " << AUX_REGISTER1 << "\n";
    output_file << "STORE " << AUX_REGISTER6 << "\n";

    //tempb = b, power = 1
    output_file << "LOAD " << CONST_ONE_REGISTER << "\n";
    output_file << "STORE " << AUX_REGISTER9 << "\n";
    output_file << "LOAD " << AUX_REGISTER2 << "\n";
    output_file << "STORE " << AUX_REGISTER8 << "\n";

    //tempb = tempb * 2
    output_file << "LOAD " << AUX_REGISTER8 << "\n";
    output_file << "ADD " << AUX_REGISTER8 << "\n";
    output_file << "STORE " << AUX_REGISTER8 << "\n";

    //while tempb (acc) <= a
    output_file << "SUB " << AUX_REGISTER1 << "\n";
    output_file << "JPOS 5" << "\n";
    //tempb = tempb * 2
    //power = power * 2
    output_file << "LOAD " << AUX_REGISTER9 << "\n";
    output_file << "ADD " << AUX_REGISTER9 << "\n";
    output_file << "STORE " << AUX_REGISTER9 << "\n";
    output_file << "JUMP -8" << "\n";

    //after while: tempb = tempb / 2
    output_file << "LOAD " << AUX_REGISTER8 << "\n";
    output_file << "HALF" << "\n";
    output_file << "STORE " << AUX_REGISTER8 << "\n";

    //while b <= r
    output_file << "LOAD " << AUX_REGISTER2 << "\n";
    output_file << "SUB " << AUX_REGISTER6 << "\n";
    output_file << "JPOS 17" << "\n";

    //if tempb <= r
    output_file << "LOAD " << AUX_REGISTER8 << "\n";
    output_file << "SUB " << AUX_REGISTER6 << "\n";
    output_file << "JPOS 7" << "\n";

    //r = r - tempb
    output_file << "LOAD " << AUX_REGISTER6 << "\n";
    output_file << "SUB " << AUX_REGISTER8 << "\n";
    output_file << "STORE " << AUX_REGISTER6 << "\n";

    //result = result + power
    output_file << "LOAD " << AUX_REGISTER3 << "\n";
    output_file << "ADD " << AUX_REGISTER9 << "\n";
    output_file << "STORE " << AUX_REGISTER3 << "\n";

    //tempb = tempb / 2
    output_file << "LOAD " << AUX_REGISTER8 << "\n";
    output_file << "HALF" << "\n";
    output_file << "STORE " << AUX_REGISTER8 << "\n";

    //power = power / 2
    output_file << "LOAD " << AUX_REGISTER9 << "\n";
    output_file << "HALF" << "\n";
    output_file << "STORE " << AUX_REGISTER9 << "\n";

    output_file << "JUMP -18" << "\n";

    //if a < 0
    output_file << "LOAD " << AUX_REGISTER5 << "\n";
    output_file << "JZERO 19" << "\n";

    //if b < 0
    output_file << "LOAD " << AUX_REGISTER7 << "\n";
    output_file << "JZERO 5" << "\n";

    //r = -r
    output_file << "LOAD " << CONST_ZERO_REGISTER << "\n";
    output_file << "SUB " << AUX_REGISTER6 << "\n";
    output_file << "STORE " << AUX_REGISTER6 << "\n";

    output_file << "JUMP 12" << "\n";

    //else2, a < 0, b > 0, result = -result, r = b - r
    output_file << "LOAD " << CONST_ZERO_REGISTER << "\n";
    output_file << "SUB " << AUX_REGISTER3 << "\n";
    output_file << "STORE " << AUX_REGISTER3 << "\n";

    //if r !=0, result -=1,
    output_file << "LOAD " << AUX_REGISTER6 << "\n";
    output_file << "JZERO 7" << "\n";
    output_file << "LOAD " << CONST_MINUS_ONE_REGISTER << "\n";
    output_file << "ADD " << AUX_REGISTER3 << "\n";
    output_file << "STORE " << AUX_REGISTER3 << "\n";

    output_file << "LOAD " << AUX_REGISTER2 << "\n";
    output_file << "SUB " << AUX_REGISTER6 << "\n";
    output_file << "STORE " << AUX_REGISTER6 << "\n";

    output_file << "JUMP 14" << "\n";

    //else, if b < 0
    output_file << "LOAD " << AUX_REGISTER7 << "\n";
    output_file << "JZERO 12" << "\n"; 

    //result = -1 - result, r = r - b
    output_file << "LOAD " << CONST_ZERO_REGISTER << "\n";
    output_file << "SUB " << AUX_REGISTER3 << "\n";
    output_file << "STORE " << AUX_REGISTER3 << "\n";

    //if r !=0, result -=1,
    output_file << "LOAD " << AUX_REGISTER6 << "\n";
    output_file << "JZERO 7" << "\n";
    output_file << "LOAD " << CONST_MINUS_ONE_REGISTER << "\n";
    output_file << "ADD " << AUX_REGISTER3 << "\n";
    output_file << "STORE " << AUX_REGISTER3 << "\n";

    output_file << "LOAD " << AUX_REGISTER6 << "\n";
    output_file << "SUB " << AUX_REGISTER2 << "\n";
    output_file << "STORE " << AUX_REGISTER6 << "\n";


    //return
    output_file << "RTRN " << AUX_REGISTER4 << "\n";
    current_output_line += 94;
}



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

        multiplication_begining_line += 2;
        division_begining_line += 2;
    }
    if(is_used[1]){
        output_file << "SET -1 " << "\n";
        output_file << "STORE " << CONST_MINUS_ONE_REGISTER << "\n";
        current_output_line += 2;

        for(procedure* procedure : procedures){
            procedure->begining_line += 2;
        }

        multiplication_begining_line += 2;
        division_begining_line += 2;
    }
    if(is_used[2]){
        output_file << "SET 0 " << "\n";
        output_file << "STORE " << CONST_ZERO_REGISTER << "\n";
        current_output_line += 2;

        for(procedure* procedure : procedures){
            procedure->begining_line += 2;
        }

        multiplication_begining_line += 2;
        division_begining_line += 2;
    }

    for(Symbol* array : arrays){
        output_file << "SET " << array->offset << "\n";
        output_file << "STORE " << array->array_pointer << "\n";
        current_output_line += 2;

        for(procedure* procedure : procedures){
            procedure->begining_line += 2;
        }

        multiplication_begining_line += 2;
        division_begining_line += 2;
    }

    if(is_multiplication){
        procedure_lines += 48;
        for(procedure* procedure : procedures){
            procedure->begining_line += 48;
        }
    }

    if(is_division){
        procedure_lines += 94;
        for(procedure* procedure : procedures){
            procedure->begining_line += 94;
        }
    }

    output_file << "JUMP " << procedure_lines + 1 << "\n";
    current_output_line++;

    if(is_multiplication){
        generate_multiplication();

        division_begining_line += 48;
    }

    if(is_division){
        generate_division();
    }

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

void initialize_array(std::string name, long long int begining, long long int end, int line_number) {
    if(current_table == nullptr){
        current_table = new SymbolTable();
    }
    if(begining > end){
        print_error("Invalid array bounds", line_number);
    }
    if(current_table->get_symbol(name) == nullptr){
        long long int length = end - begining + 1;
        Symbol* new_symbol = new Symbol(name, ARRAY, length + 1, begining, end, get_register_counter() - begining);
        increment_register_counter(length + 1);
        new_symbol->is_initialized = false;
        new_symbol->is_pointer = false;
        new_symbol->is_iterator = false;

        if(new_symbol->offset + new_symbol->end_index + 1 < 0){
            print_error("Negative memory adress", line_number);
        }

        new_symbol->array_pointer = new_symbol->offset + new_symbol->end_index + 1;

        if(new_symbol->offset + new_symbol->length - 1 > user_registers){
            print_error("Out of registers", line_number);
        }
        current_table->add_symbol(new_symbol);
        arrays.push_back(new_symbol);
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

    if(current_table->get_symbol(name)->offset < 0){
        print_error("Negative memory adress", line_number);
    }

    new_variable->register_number = current_table->get_symbol(name)->offset;
    new_variable->is_initialized = true;
    new_variable->is_iterator = true;
    new_variable->is_pointer = false;
    return new_variable;
}






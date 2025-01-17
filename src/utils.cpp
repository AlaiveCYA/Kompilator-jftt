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

static std::vector<Command*> commands = std::vector<Command*>();
static std::vector<SymbolTable*> symbol_tables = std::vector<SymbolTable*>();
static SymbolTable* current_table = nullptr;
static SymbolTable* global_table = nullptr;

static bool assigment_after_exp = false;

void print_error(std::string error, int line_number) {
    std::cout << "Error in line: " << line_number << ":\n" << error << std::endl;
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

void create_write_statement(struct variable* value, int line_number) {
    write_statement* new_statement = new write_statement(value);
    commands.push_back(new_statement);
}

/*********************READ**********************/

void read_statement::generate_code() {
    output_file << "GET " << var->register_number << "\n";
}

read_statement::read_statement(struct variable* value) {
    this->var = value;
}

void create_read_statement(struct variable* value, int line_number) {
    read_statement* new_statement = new read_statement(value);
    commands.push_back(new_statement);
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

struct variable* create_addition(struct variable* left, struct variable* right, int line_number) {
    if (left->type == VALUE && right->type == VALUE) {
        if(left->val >= 0 && MAX_NUMBER - left->val < right->val){
            print_error("Number overflow", line_number);
        }
        else if(left->val < 0 && MIN_NUMBER - left->val > right->val){
            print_error("Number overflow", line_number);
        }
    }
    addition* new_addition = new addition(left, right);
    commands.push_back(new_addition);
    variable* new_variable = new variable();
    new_variable->type = VARIABLE;
    new_variable->register_number = 0;
    assigment_after_exp = true;
    return new_variable;
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

struct variable* create_subtraction(struct variable* left, struct variable* right, int line_number) {
    if (left->type == VALUE && right->type == VALUE) {
        if(left->val >= 0 && MAX_NUMBER + left->val < right->val){
            print_error("Number overflow", line_number);
        }
        else if(left->val < 0 && MIN_NUMBER + left->val > right->val){
            print_error("Number overflow", line_number);
        }
    }
    subtraction* new_subtraction = new subtraction(left, right);
    commands.push_back(new_subtraction);
    variable* new_variable = new variable();
    new_variable->type = VARIABLE;
    new_variable->register_number = 0;
    assigment_after_exp = true;
    return new_variable;
}

/****************ASSIGNMENT********************/

assignment::assignment(struct variable* left, struct variable* right, bool assigment_after_exp) {
    this->left = left;
    this->right = right;
    this->assigment_after_exp = assigment_after_exp;
}

void assignment::generate_code() {
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
        output_file << "STORE " << left->register_number << "\n";
    }
    else if(right->type == VARIABLE && assigment_after_exp){
        output_file << "STORE " << left->register_number << "\n";
    }
    else if(right->type == VARIABLE){
        output_file << "LOAD " << right->register_number << "\n";
        output_file << "STORE " << left->register_number << "\n";
    }
}

void create_assignment(struct variable* left, struct variable* right, int line_number) {
    if(left->type == VALUE){
        print_error("Cannot assign variable to a number", line_number);
    }
    assignment* new_assignment = new assignment(left, right, assigment_after_exp);
    assigment_after_exp = false;
    commands.push_back(new_assignment);
}

/*******************CONDITION***************************/

condition::condition(struct variable* left, struct variable* right, condition_t condition) {
    this->left = left;
    this->right = right;
    this->condition_type = condition;
}

struct condition* create_eq_condition(struct variable* left, struct variable* right, int line_number){

    if(left->type == VALUE && right->type == VALUE){
        if(left->val == right->val){
            condition* new_condition = new condition(left, right, EQ_COND);
            new_condition->is_precalc = true;
            new_condition->value = true;
            return new_condition;
        }
        else{
            condition* new_condition = new condition(left, right, EQ_COND);
            new_condition->is_precalc = true;
            new_condition->value = false;
            return new_condition;
        }
    }

    condition* new_condition = new condition(left, right, EQ_COND);
    return new_condition;
}

struct condition* create_neq_condition(struct variable* left, struct variable* right, int line_number){

    if(left->type == VALUE && right->type == VALUE){
        if(left->val != right->val){
            condition* new_condition = new condition(left, right, NEQ_COND);
            new_condition->is_precalc = true;
            new_condition->value = true;
            return new_condition;
        }
        else{
            condition* new_condition = new condition(left, right, NEQ_COND);
            new_condition->is_precalc = true;
            new_condition->value = false;
            return new_condition;
        }
    }

    condition* new_condition = new condition(left, right, NEQ_COND);
    return new_condition;
}

struct condition* create_lt_condition(struct variable* left, struct variable* right, int line_number){

    if(left->type == VALUE && right->type == VALUE){
        if(left->val < right->val){
            condition* new_condition = new condition(left, right, LT_COND);
            new_condition->is_precalc = true;
            new_condition->value = true;
            return new_condition;
        }
        else{
            condition* new_condition = new condition(left, right, LT_COND);
            new_condition->is_precalc = true;
            new_condition->value = false;
            return new_condition;
        }
    }

    condition* new_condition = new condition(left, right, LT_COND);
    return new_condition;
}

struct condition* create_leq_condition(struct variable* left, struct variable* right, int line_number){

    if(left->type == VALUE && right->type == VALUE){
        if(left->val <= right->val){
            condition* new_condition = new condition(left, right, LEQ_COND);
            new_condition->is_precalc = true;
            new_condition->value = true;
            return new_condition;
        }
        else{
            condition* new_condition = new condition(left, right, LEQ_COND);
            new_condition->is_precalc = true;
            new_condition->value = false;
            return new_condition;
        }
    }

    condition* new_condition = new condition(left, right, LEQ_COND);
    return new_condition;
}

/********************IF*******************************/

if_statement::if_statement(struct condition* condition, std::vector<Command*> commands) {
    this->condition = condition;
    this->commands = commands;
}

void create_if_statement(struct condition* condition, int line_number) {
    
}


/******************VARIABLES****************************/

struct variable* create_number_variable(long long int value, int line_number) {
    variable* new_variable = new variable();
    new_variable->type = VALUE;
    new_variable->val = value;
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
    return new_variable;
}

/********************************COMPILER*****************************************/

void generate_code(void) {

    output_file.open(output_filename);
    for (Command* command : commands) {
        // std::cout << "generating code" << std::endl;
        command->generate_code();
    }
}

void end_program(void) {
    // std::cout << "generating halt" << std::endl;
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

void set_global_symbol_table(void) {
    global_table = current_table;
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






#ifndef COMPILER_UTILS_HPP
#define COMPILER_UTILS_HPP

#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <climits>

#include "symbol_table.hpp"

#define MAX_NUMBER LLONG_MAX
#define MIN_NUMBER LLONG_MIN

struct variable {
    std::string name;
    long long int val;
    var_type_t type;
    long long int register_number;
    bool is_initialized = false;
};

typedef enum {
    COND_EQ,
    COND_NEQ,
    COND_LT,
    COND_LEQ,
} cond_type_t;

class command {
public:
    virtual void generate_code() = 0;
    int lines_taken = 0;
};

class write_statement : public command {
public:
    write_statement(struct variable* value);

    struct variable* value;
    void generate_code();
};

class read_statement : public command {
public:
    read_statement(struct variable* value);

    struct variable* var;
    void generate_code();
};

class condition : public command {
public:
    condition(struct variable* left, struct variable* right);

    struct variable* left;
    struct variable* right;

    cond_type_t type;

    void generate_code() = 0;
};

class eq_condition : public condition {
public:
    eq_condition(struct variable* left, struct variable* right) : condition(left, right) {}

    void generate_code();
};

class neq_condition : public condition {
public:
    neq_condition(struct variable* left, struct variable* right) : condition(left, right) {}

    void generate_code();
};

class lt_condition : public condition {
public:
    lt_condition(struct variable* left, struct variable* right) : condition(left, right) {}

    void generate_code();
};

class leq_condition : public condition {
public:
    leq_condition(struct variable* left, struct variable* right) : condition(left, right) {}

    void generate_code();
};

class if_statement : public command {
public:
    if_statement(struct condition* condition, std::vector<command*> *commands);

    struct condition* condition;
    std::vector<command*> *commands;
    void generate_code();
};

class if_else_statement : public command {
public:
    if_else_statement(struct condition* condition, std::vector<command*> *if_commands, std::vector<command*> *else_commands);

    struct condition* condition;
    std::vector<command*> *if_commands;
    std::vector<command*> *else_commands;
    int if_lines_taken;
    int else_lines_taken;
    void generate_code();
};

class while_statement : public command {
public:
    while_statement(struct condition* condition, std::vector<command*> *commands);

    struct condition* condition;
    std::vector<command*> *commands;
    void generate_code();
};

class repeat_until_statement : public command {
public:
    repeat_until_statement(std::vector<command*> *commands, struct condition* condition);

    struct condition* condition;
    std::vector<command*> *commands;
    void generate_code();
};

class expression : public command {
public:

    struct variable* left;
    struct variable* right;
    bool is_precalc = false;
    long long int value = 0;
    void generate_code() = 0;
};

class addition : public expression {
public:
    addition(struct variable* left, struct variable* right);

    void generate_code();
};

class subtraction : public expression {
public:
    subtraction(struct variable* left, struct variable* right);

    void generate_code();
};

class multiplication : public expression {
public:
    multiplication(struct variable* left, struct variable* right);

    void generate_code();
};

class division : public expression {
public:
    division(struct variable* left, struct variable* right);

    void generate_code();
};

class value_expression : public expression {
public:
    value_expression(struct variable* value);

    void generate_code();
};

class assignment : public command {
public:
    assignment(struct variable* left, expression* exp);

    struct variable* left;
    struct expression* right;
    void generate_code();
};

void set_output_filename(std::string filename);
void open_file(void);
void close_file(void);

void generate_code(void);

command* create_write_statement(struct variable* value, int line_number);
command* create_read_statement(struct variable* value, int line_number);
command* create_assignment(struct variable* left, expression* right, int line_number);
command* create_if_statement(struct condition* condition, std::vector<command*>* commands, int line_number);
command* create_if_else_statement(struct condition* condition, std::vector<command*>* if_commands, std::vector<command*>* else_commands, int line_number);
command* create_while_statement(struct condition* condition, std::vector<command*>* commands, int line_number);
command* create_repeat_until_statement(std::vector<command*>* commands, struct condition* condition, int line_number);



expression* create_addition(struct variable* left, struct variable* right, int line_number);
expression* create_subtraction(struct variable* left, struct variable* right, int line_number);
expression* create_multiplication(struct variable* left, struct variable* right, int line_number);
expression* create_division(struct variable* left, struct variable* right, int line_number);
expression* create_modulo(struct variable* left, struct variable* right, int line_number);
expression* pass_variable_as_expression(struct variable* value, int line_number);



struct condition* create_eq_condition(struct variable* left, struct variable* right, int line_number);
struct condition* create_neq_condition(struct variable* left, struct variable* right, int line_number);
struct condition* create_lt_condition(struct variable* left, struct variable* right, int line_number);
struct condition* create_leq_condition(struct variable* left, struct variable* right, int line_number);



void initialize_variable(std::string name, int line_number);
struct variable* create_number_variable(long long int value, int line_number);
struct variable* create_variable(std::string name, int line_number);


std::vector<command*> *pass_commands(std::vector<command*> *commands, command* new_command);
std::vector<command*> *pass_commands(command* new_command);



void create_new_symbol_table(void);
void set_globals(std::vector<command*> *commands);


void print_error(std::string error, int line_number);
void end_program(void);

#endif //COMPILER_UTILS_HPP
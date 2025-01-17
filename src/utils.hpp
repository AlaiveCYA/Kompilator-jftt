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

typedef enum condition_t {
    EQ_COND,
    NEQ_COND,
    LT_COND,
    LEQ_COND,
}condition_t;

struct variable {
    std::string name;
    long long int val;
    var_type_t type;
    long long int register_number;
};

struct condition {
    condition(struct variable* left, struct variable* right, condition_t condition);
    struct variable* left;
    struct variable* right;
    condition_t condition_type;
    bool is_precalc = false;
    bool value = false;
};

class Command {
public:
    virtual void generate_code() = 0;
};

class write_statement : public Command {
public:
    write_statement(struct variable* value);

    struct variable* value;
    void generate_code();
};

class read_statement : public Command {
public:
    read_statement(struct variable* value);

    struct variable* var;
    void generate_code();
};

class if_statement : public Command {
public:
    if_statement(struct condition* condition, std::vector<Command*> commands);

    struct condition* condition;
    std::vector<Command*> commands;
    void generate_code();
};

class if_else_statement : public Command {
public:
    if_else_statement(struct condition* condition, std::vector<Command*> if_commands, std::vector<Command*> else_commands);

    struct condition* condition;
    std::vector<Command*> if_commands;
    std::vector<Command*> else_commands;
    void generate_code();
};

class expression : public Command {
public:

    struct variable* left;
    struct variable* right;
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

class assignment : public Command {
public:
    assignment(struct variable* left, struct variable* right, bool assigment_after_exp);

    struct variable* left;
    struct variable* right;
    bool assigment_after_exp = false;
    void generate_code();
};

void set_output_filename(std::string filename);
void open_file(void);
void close_file(void);



void generate_code(void);



void create_write_statement(struct variable* value, int line_number);
void create_read_statement(struct variable* value, int line_number);
void create_assignment(struct variable* left, struct variable* right, int line_number);



struct variable* create_addition(struct variable* left, struct variable* right, int line_number);
struct variable* create_subtraction(struct variable* left, struct variable* right, int line_number);
struct variable* create_multiplication(struct variable* left, struct variable* right, int line_number);
struct variable* create_division(struct variable* left, struct variable* right, int line_number);
struct variable* create_modulo(struct variable* left, struct variable* right, int line_number);




struct condition* create_eq_condition(struct variable* left, struct variable* right, int line_number);
struct condition* create_neq_condition(struct variable* left, struct variable* right, int line_number);
struct condition* create_lt_condition(struct variable* left, struct variable* right, int line_number);
struct condition* create_leq_condition(struct variable* left, struct variable* right, int line_number);



void initialize_variable(std::string name, int line_number);
struct variable* create_number_variable(long long int value, int line_number);
struct variable* create_variable(std::string name, int line_number);



void create_new_symbol_table(void);
void set_global_symbol_table(void);


void print_error(std::string error, int line_number);
void end_program(void);

#endif //COMPILER_UTILS_HPP
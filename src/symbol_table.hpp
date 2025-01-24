#ifndef SYMBOL_TABLE_HPP
#define SYMBOL_TABLE_HPP

#include <iostream>
#include <vector>
#include <string>

#include "symbol_table.hpp"

#define MAX_REGISTERS 4611686018427387903

long long int register_counter = 1;

typedef enum {
    VARIABLE,
    VALUE,
    ARRAY,
    PARAM,
} var_type_t;

class Symbol {
public:
    Symbol(std::string name, var_type_t type, long long length, long long start_index, long long end_index);
    Symbol(std::string name, var_type_t type, long long lenght, long long start_index, long long end_index, long long offset);
    std::string name;
    long long int val;
    var_type_t type;
    long long int offset;
    long long int length;
    long long int start_index;
    long long int end_index;
    bool is_initialized = false;
    bool is_iterator;
    bool is_pointer;
};

class SymbolTable {
public:
    SymbolTable();
    void add_symbol(Symbol* symbol);
    Symbol* get_symbol(std::string name);
    void print_table();
    void remove_symbol(std::string name);
    Symbol* get_last();

private:
    std::vector<Symbol*> symbols;
};

#endif //SYMBOL_TABLE_HPP
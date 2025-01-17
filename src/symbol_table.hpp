#ifndef SYMBOL_TABLE_HPP
#define SYMBOL_TABLE_HPP

#include <iostream>
#include <vector>
#include <string>

#include "symbol_table.hpp"

#define MAX_REGISTERS 4611686018427387903

typedef enum {
    VARIABLE,
    VALUE,
    ARRAY,
} var_type_t;

class Symbol {
public:
    Symbol(std::string name, var_type_t type, long long length, long long start_index, long long end_index);
    std::string name;
    long long int val;
    var_type_t type;
    long long int offset;
    long long int length;
    long long int start_index;
    long long int end_index;
    bool is_initialized = false;
};

class SymbolTable {
public:
    SymbolTable();
    void add_symbol(Symbol* symbol);
    Symbol* get_symbol(std::string name);
    void print_table();
private:
    std::vector<Symbol*> symbols;
    std::string name;
};

#endif //SYMBOL_TABLE_HPP
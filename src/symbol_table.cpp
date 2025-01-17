#include "symbol_table.hpp"

static long long int register_counter = 1;

Symbol::Symbol(std::string name, var_type_t type, long long length, long long start_index, long long end_index) {
    this->name = name;
    this->type = type;
    this->offset = register_counter;
    this->length = length;
    this->start_index = start_index;
    this->end_index = end_index;
    this->is_initialized = false;

    register_counter += length;

    if (register_counter > MAX_REGISTERS) {
        std::cout << "Error: Out of registers" << std::endl;
        exit(1);
    }
}

SymbolTable::SymbolTable() {
    this->symbols = std::vector<Symbol*>();
}

void SymbolTable::add_symbol(Symbol* symbol) {
    this->symbols.push_back(symbol);
}

Symbol* SymbolTable::get_symbol(std::string name) {
    for (Symbol* symbol : this->symbols) {
        if (symbol->name == name) {
            return symbol;
        }
    }
    return nullptr;
}

void SymbolTable::print_table() {
    for (Symbol* symbol : this->symbols) {
        std::cout << "Name: " << symbol->name << " Value: " << symbol->val << " Type: " << symbol->type << " Register Number: " << symbol->offset << std::endl;
    }
}
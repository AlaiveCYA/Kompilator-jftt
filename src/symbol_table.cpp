#include "symbol_table.hpp"

long long int register_counter = 1;

Symbol::Symbol(std::string name, var_type_t type, long long length, long long start_index, long long end_index) {
    this->name = name;
    this->type = type;
    this->offset = register_counter;
    this->length = length;
    this->start_index = start_index;
    this->end_index = end_index;
    this->is_initialized = false;

    if(type != VALUE) {
        register_counter += length;
    }
}

Symbol::Symbol(std::string name, var_type_t type, long long length, long long start_index, long long end_index, long long offset) {
    this->name = name;
    this->type = type;
    this->offset = offset;
    this->length = length;
    this->start_index = start_index;
    this->end_index = end_index;
    this->is_initialized = false;
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

void SymbolTable::remove_symbol(std::string name) {
    for (size_t i = 0; i < this->symbols.size(); i++) {
        if (this->symbols[i]->name == name) {
            this->symbols.erase(this->symbols.begin() + i);
            return;
        }
    }
}

Symbol* SymbolTable::get_last() {
    return this->symbols.back();
}

long long int get_register_counter(void) {
    return register_counter;
}

void increment_register_counter(void) {
    register_counter++;
}

void increment_register_counter(long long int increment) {
    register_counter += increment;
}
%{

#include <stdio.h>
#include <iostream>
#include "utils.hpp"
#include <vector>

extern int yylex();
extern int yyerror(const char *s);
extern FILE *yyin;
extern int yylineno;
    
%}

%union {
    std::string* pidentifier;
    long long number;
    struct variable* var;
    struct condition* cond;
    std::vector<command*> *commands;
    command* cmd;
    expression* expr;
};

// Tokens

%token <number> NUMBER
%token PROCEDURE
%token IS
%token TBEGIN
%token END
%token PROGRAM
%token IF
%token THEN
%token ELSE
%token ENDIF
%token WHILE
%token DO
%token ENDWHILE
%token REPEAT
%token UNTIL
%token FOR
%token FROM
%token TO
%token DOWNTO
%token ENDFOR
%token READ
%token WRITE
%token <pidentifier> pidentifier
%token T
%token COMA
%token SEMICOLON
%token LBRACKET
%token RBRACKET
%token LPAR
%token RPAR
%token EQ
%token NEQ
%token GT
%token LT
%token GEQ
%token LEQ
%token ASSIGN
%token COLON
%token MOD
%token PLUS
%token MINUS
%token TIMES
%token DIV

%type <var> value
%type <var> identifier
%type <expr> expression
%type <cond> condition
%type <commands> commands
%type <cmd> command

%left PLUS MINUS
%left TIMES DIV MOD

%%

program: procedures main
        ;

procedures: 
        |   procedures PROCEDURE proc_head IS declarations TBEGIN commands END { create_new_symbol_table(); }
        |   procedures PROCEDURE proc_head IS TBEGIN commands END
        ;

main:       PROGRAM IS declarations TBEGIN commands END { set_globals($5); }
        |   PROGRAM IS TBEGIN commands END 
        ;

commands :  commands command { $$ = pass_commands($1, $2); }
        |   command { $$ = pass_commands($1); }
        ;

command:    identifier ASSIGN expression SEMICOLON { $$ = create_assignment($1, $3, yylineno); }
        |   IF condition THEN commands ELSE commands ENDIF { $$ = create_if_else_statement($2, $4, $6, yylineno); }
        |   IF condition THEN commands ENDIF { $$ = create_if_statement($2, $4, yylineno); }
        |   WHILE condition DO commands ENDWHILE { $$ = create_while_statement($2, $4, yylineno); }
        |   REPEAT commands UNTIL condition SEMICOLON { $$ = create_repeat_until_statement($2, $4, yylineno); }
        |   FOR pidentifier FROM value TO value DO commands ENDFOR
        |   FOR pidentifier FROM value DOWNTO value DO commands ENDFOR
        |   proc_call SEMICOLON
        |   READ identifier SEMICOLON { $$ = create_read_statement($2, yylineno); }
        |   WRITE value SEMICOLON { $$ = create_write_statement($2, yylineno); }
        ;

proc_head: pidentifier LPAR formal_parameters RPAR
        ;

proc_call: pidentifier LPAR actual_parameters RPAR
        ;

declarations:   declarations COMA pidentifier { initialize_variable(*$3, yylineno); }
        |   declarations COMA pidentifier LBRACKET NUMBER COLON NUMBER RBRACKET
        |   pidentifier { initialize_variable(*$1, yylineno); }
        |   pidentifier LBRACKET NUMBER COLON NUMBER RBRACKET

formal_parameters: 
        |   formal_parameters COMA pidentifier
        |   pidentifier
        |   formal_parameters COMA T pidentifier
        |   T pidentifier
        ;

actual_parameters:
        |   actual_parameters COMA value
        |   value
        ;

expression: value { $$ = pass_variable_as_expression($1, yylineno); }
        |   value PLUS value { $$ = create_addition($1, $3, yylineno); }
        |   value MINUS value { $$ = create_subtraction($1, $3, yylineno); }
        |   value TIMES value {}
        |   value DIV value {}
        |   value MOD value {}
        ;

condition: value EQ value { $$ = create_eq_condition($1, $3, yylineno); }
        |   value LT value { $$ = create_lt_condition($1, $3, yylineno); }
        |   value GT value { $$ = create_lt_condition($3, $1, yylineno); }
        |   value LEQ value { $$ = create_leq_condition($1, $3, yylineno); }
        |   value GEQ value { $$ = create_leq_condition($3, $1, yylineno); }
        |   value NEQ value { $$ = create_neq_condition($1, $3, yylineno); }
        ;

value:      NUMBER { $$ = create_number_variable($1, yylineno); }
        |   identifier { $$ = $1; }
        ;

identifier: pidentifier { $$ = create_variable(*$1, yylineno); }
        |   pidentifier LBRACKET pidentifier RBRACKET
        |  pidentifier LBRACKET NUMBER RBRACKET
        ;
%%

int main(int argc, char **argv) {
    if( argc != 3 ) {
        std::cerr << "Prawidlowe wywolanie: ./kompilator plik_wejsciowy plik_wyjsciowy" << std::endl;
        return 1;
    }

    yyin = fopen(argv[1], "r");
    if (yyin == NULL){
        std::cout << "Plik nie istnieje" << std::endl;
        return 1;
    }

    set_output_filename(argv[2]);

	yyparse();

    generate_code();
    end_program();
    return 0;
}
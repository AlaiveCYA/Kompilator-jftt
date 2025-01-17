%{

#include <stdio.h>
#include "utils.hpp"

#include "parser.tab.h"
    
    
void yyerror(const char *s) {
        fprintf(stderr,"%s in line %d\n", s, yylineno);
}

%}

%option noyywrap
%option yylineno
%option nounput

%%

"#"[^\n]*\n { /* ignore comments */ }
[ \t\n] { /* ignore whitespace */ }

"PROGRAM" { return PROGRAM; }
"PROCEDURE" { return PROCEDURE; }
"IS" { return IS; }
"BEGIN" { return TBEGIN; }
"END" { return END; }
"IF" { return IF; }
"THEN" { return THEN; }
"ELSE" { return ELSE; }
"ENDIF" { return ENDIF; }
"WHILE" { return WHILE; }
"DO" { return DO; }
"ENDWHILE" { return ENDWHILE; }
"REPEAT" { return REPEAT; }
"UNTIL" { return UNTIL; }
"FOR" { return FOR; }
"FROM" { return FROM; }
"TO" { return TO; }
"DOWNTO" { return DOWNTO; }
"ENDFOR" { return ENDFOR; }
"READ" { return READ; }
"WRITE" { return WRITE; }
"T" { return T; }
":=" { return ASSIGN; }
"," { return COMA; }
";" { return SEMICOLON; }
"[" { return LBRACKET; }
"]" { return RBRACKET; }
"(" { return LPAR; }
")" { return RPAR; }
"=" { return EQ; }
"!=" { return NEQ; }
">" { return GT; }
"<" { return LT; }
">=" { return GEQ; }
"<=" { return LEQ; }
":" { return COLON; }
"%" { return MOD; }
"+" { return PLUS; }
"-" { return MINUS; }
"*" { return TIMES; }
"/" { return DIV; }

(-?[1-9][0-9]*|0) { yylval.number = atoll(yytext); return NUMBER; }
[_a-z]+ { yylval.pidentifier = new std::string(yytext); return pidentifier; }

. { printf("Unrecognized character: %s\n", yytext); }
%%
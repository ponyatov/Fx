#pragma once

#include <assert.h>

#include <iostream>

extern int main(int argc, char* argv[]);
extern void arg(int argc, char* argv);
extern void init(int argc, char* argv[]);
extern int fini(int err = 0);

struct Object {};

extern int yylex();
extern int yylineno;
extern char* yytext;
extern FILE* yyin;
extern char* yyfile;
extern void yyerror(std::string msg);
extern int yyparse();

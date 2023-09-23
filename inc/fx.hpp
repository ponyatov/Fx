#pragma once

#include <assert.h>

#include <cxxabi.h>

#include <readline/readline.h>
#include <readline/history.h>

#include <iostream>
#include <sstream>
#include <vector>
#include <map>

/// @defgroup init init
/// @{

extern int main(int argc, char* argv[]);   //< program entry
extern void arg(int argc, char* argv);     //< print cmdline arg
extern void init(int argc, char* argv[]);  //< initizalize
extern int fini(int err = 0);              //< system stop (finalize)

/// @}

struct Object {  /// root executable object

    /// @name garbage collector
    size_t ref;           //< ref counter
    static Object* pool;  //< global objects pool
    Object* next;         //< next object in @ref pool

    /// @name fields
    std::string value;  //< object name, number/string value

    /// @name constructor
    Object();
    Object(std::string V);
    virtual ~Object();

    /// @name dump
    virtual std::string tag();
    virtual std::string val();
    std::string head();

    /// @name interpreter/compiler
    virtual void exec();
};

struct Active : Object {
    Active();
    Active(std::string V);
};

struct Cmd : Active {
    void (*fn)();
    Cmd(void (*F)(), std::string V);
    void exec();
};

extern std::map<std::string, Object*> W;  //< vocabulary
extern std::vector<Object*> D;            //< data stack

extern int yylex();                    //< lexer
extern int yylineno;                   //< current line
extern char* yytext;                   //< lexemet: token value
extern FILE* yyin;                     //< current script file
extern char* yyfile;                   //< current file name
extern void yyerror(std::string msg);  //< syntax error callback
extern int yyparse();                  //< syntax parser
#include "fx.parser.hpp"

#define TOKEN(C, X)               \
    {                             \
        yylval.o = new C(yytext); \
        return X;                 \
    }

#define CMD(F, V)                 \
    {                             \
        yylval.o = new Cmd(F, V); \
        return CMD;               \
    }

extern void nop();   //< `( -- )` empty command
extern void halt();  //< `( -- )` stop system
extern void repl();  //< `( -- )` start interactive REPL console
extern void q();     //< `( -- )` debug dump: @ref D & @ref W

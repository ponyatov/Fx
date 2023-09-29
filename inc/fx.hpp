#pragma once

#include <assert.h>

#include <cxxabi.h>

#include <iostream>
#include <sstream>
#include <vector>
#include <stack>
#include <map>

#include <readline/readline.h>
#include <readline/history.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_audio.h>

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
    Object* r();          //< increment ref @returns self

    /// @name fields
    std::string value;          //< object name, number/string value
    std::vector<Object*> nest;  //< nested elements = vector = stack = queue
    std::map<std::string, Object*> slot;  //< map = vocabulary = AST sttributes

    /// @name constructor
    Object();
    Object(std::string V);
    virtual ~Object();

    /// @name dump
    virtual std::string tag();
    virtual std::string val();

    /// `<T:V>` object header
    std::string head(std::string prefix = "");
    /// full text tree dump
    std::string dump(int depth = 0, std::string prefix = "");
    /// @ref dump tree padding
    static std::string pad(int depth);

    /// @name interpreter/compiler
    virtual void exec();

    /// @name stack ops
    void push(Object* o);  //< push to @ref nest
    Object* pop();         //< pop from @ref nest as a stack
};

struct Primitive : Object {
    Primitive();
    Primitive(std::string V);
};

struct Sym : Primitive {
    Sym(std::string V);
    void exec();
};

struct Str : Primitive {
    Str(std::string V);
};

struct Int : Primitive {
    int value;
    std::string val();
    Int(std::string V);
    Int(int N);
};

struct Container : Object {
    Container();
    Container(std::string V);
};

struct Vector : Container {
    Vector();
    Vector(std::string V);
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

extern void nop();     //< `( -- )` empty command
extern void halt();    //< `( -- )` stop system
extern void repl();    //< `( -- )` start interactive REPL console
extern void q();       //< `( -- )` debug dump: @ref D & @ref W
extern void dot();     //< `( ... -- )` clean @ref D
extern void tick();    //< `( -- token )` parse next token into stack
extern void stor();    //< `( o name -- )` store o into @ref W with `name`
extern void get();     //< `( name -- o )` get object from @ref @ by `name`
extern Object* pop();  //< `( n1 n2 -- n1 )` pop element from @ref D
extern void push(Object* o);  //< `( -- o )` push element to @ref D
extern void error(std::string msg, Object* o);  //< raise error

struct IO : Object {
    IO(std::string V);
};

struct GUI : IO {
    GUI(std::string V);
};

struct Win : GUI {
    SDL_Window* window = nullptr;
    Win(std::string V);
};

struct Audio : IO {
    Audio(std::string V);
};

extern void gui();    //< `( -- )` start GUI/video
extern void audio();  //< `( -- )` start audio

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
/// @brief system init
/// @{

extern int main(int argc, char* argv[]);   ///< program entry
extern void arg(int argc, char* argv);     ///< print cmdline args
extern void init(int argc, char* argv[]);  ///< initizalize
extern int fini(int err = 0);              ///< system stop (finalize)

/// @}

/// @defgroup core core
/// @brief Executable Data Structures (c)

/// @ingroup core
/// @brief root executable object
struct Object {
    /// @name garbage collector
    size_t ref;           ///< ref counter
    static Object* pool;  ///< global objects pool
    Object* next;         ///< next object in @ref pool
    Object* r();          ///< increment ref @returns self

    /// @name fields
    std::string value;  ///< object name, number/string value

    /// nested & ordered elements = vector = stack= queue = AST subtree
    std::vector<Object*> nest;

    /// string-associative array = map = vocabulary = AST attributes
    std::map<std::string, Object*> slot;

    /// @name constructor
    Object();
    Object(std::string V);
    virtual ~Object();

    /// @name dump
    /// type/class name (lowercased)
    virtual std::string tag();
    /// get value / stringify
    virtual std::string val();

    /// `<T:V>` object header
    std::string head(std::string prefix = "");
    /// full text tree dump
    std::string dump(int depth = 0, std::string prefix = "");
    /// @ref dump tree padding
    static std::string pad(int depth);

    /// @name interpreter/compiler
    virtual void exec();  ///< execute object in @ref vm global context

    /// @name stack ops
    void clean();          ///< `( ... )` clean @ref nest
    void push(Object* o);  ///< `( -- o )` push to @ref nest
    Object* pop();         ///< `( o -- )` pop from @ref nest

    /// @name slot ops
    Object* get(std::string idx);  ///< get slot by its `.name`
    Object* get(int idx);          ///< get nest by index
};

/// @defgroup prim primitive
/// @ingroup core
/// @brief scalar and atomic values

/// @ingroup prim
struct Primitive : Object {
    Primitive();
    Primitive(std::string V);
};

/// @ingroup prim
/// @brief symbol
struct Sym : Primitive {
    Sym(std::string V);
    void exec();
};

/// @ingroup prim
/// @brief text string
struct Str : Primitive {
    Str(std::string V);
};

/// @ingroup prim
/// @brief integer number
struct Int : Primitive {
    int value;
    std::string val();
    Int(std::string V);
    Int(int N);
};

/// @defgroup cont container
/// @ingroup core
/// @brief data containers

/// @ingroup cont
struct Container : Object {
    Container();
    Container(std::string V);
};

/// @ingroup cont
struct Vector : Container {
    Vector();
    Vector(std::string V);
};

/// @defgroup active active
/// @ingroup core
/// @brief executable active data

/// @ingroup active
struct Active : Object {
    Active();
    Active(std::string V);
};

/// @ingroup active
struct VM : Active {
    VM(std::string V);
};

/// @ingroup active
extern VM vm;  ///< global @ref VM: vocabulary & data stack

/// @ingroup active
struct Cmd : Active {
    void (*fn)();
    Cmd(void (*F)(), std::string V);
    void exec();
};

/// @defgroup skelex skelex
/// @brief lexical skeleton: syntax parser
/// @{
extern int yylex();                    ///< lexer
extern int yylineno;                   ///< current line
extern char* yytext;                   ///< lexemet: token value
extern FILE* yyin;                     ///< current script file
extern char* yyfile;                   ///< current file name
extern void yyerror(std::string msg);  ///< syntax error callback
extern int yyparse();                  ///< syntax parser
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
/// @}

extern void nop();    ///< `( -- )` empty command
extern void halt();   ///< `( -- )` stop system
extern void repl();   ///< `( -- )` start interactive REPL console
extern void q();      ///< `( -- )` debug dump: @ref D & @ref W
extern void clean();  ///< `( ... -- )` clean @ref D
extern void tick();   ///< `( -- token )` parse next token into stack
extern void stor();   ///< `( o name -- )` store o into @ref W with `name`
extern void get();    ///< `( name -- o )` get object from @ref @ by `name`
extern void dot();    ///< `( o -- o.element )` get slot by it's `.name`
extern void error(std::string msg, Object* o);  ///< raise error

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

extern void gui();    ///< `( -- )` start GUI/video
extern void sound();  ///< `( -- )` start audio

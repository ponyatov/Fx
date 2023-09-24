%{
    #include "fx.hpp"
    char* yyfile = nullptr;
%}

%option noyywrap yylineno

s [+\-]?
n [0-9]+
%%
#.*             {}                  // line comment
[ \t\r\n]+      {}                  // drop spaces

{s}{n}          TOKEN(Int,INT)

"nop"           CMD(nop ,"nop" )
"halt"          CMD(halt,"halt")
"repl"          CMD(repl,"repl")
"?"             CMD(   q,"?"   )

.               {yyerror("");}      // any undetected char

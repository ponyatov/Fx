%{
    #include "fx.hpp"
    char* yyfile = nullptr;
%}

%option noyywrap yylineno

s [+\-]?
n [0-9]+
%%
#.*             {}                      // line comment
[ \t\r\n]+      {}                      // drop spaces

{s}{n}          TOKEN(Int,INT)

"nop"           CMD(nop ,"nop" )
"halt"          CMD(halt,"halt")
"repl"          CMD(repl,"repl")
"?"             CMD(   q,"?"   )
"."             CMD( dot,"."   )
"`"             CMD(tick,"`"   )
"="             CMD(stor,"="   )
"@"             CMD( get,"@"   )
"/"             CMD( sub,"/"   )

"gui"           CMD( gui  ,"gui"   )
"audio"         CMD( audio,"audio" )

[_a-zA-Z][_a-zA-Z0-9]*  TOKEN(Sym,SYM)  // symbol

.               {yyerror("");}          // any undetected char

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

"nop"           CMD( nop ,"nop" )
"halt"          CMD( halt,"halt")
"repl"          CMD( repl,"repl")
"?"             CMD(    q,"?"   )
";"             CMD(clean,";"   )
"`"             CMD( tick,"`"   )
"="             CMD( stor,"="   )
"@"             CMD(  get,"@"   )
"."             CMD(  dot,"."   )

"dup"           CMD(  dup,"dup"  )
"drop"          CMD( drop,"drop" )
"swap"          CMD( swap,"swap" )
"over"          CMD( over,"over" )

"gui"           CMD(    gui,"gui"   )
"sound"         CMD(  sound,"sound" )
"play"          CMD(   play,"play"  )
"pause"         CMD( _pause,"pause" )
"stop"          CMD(   stop,"stop"  )

"open"          CMD( open,"open"   )
"close"         CMD( close,"close" )

[_a-zA-Z][_a-zA-Z0-9]*  TOKEN(Sym,SYM)  // symbol

.               {yyerror("");}          // any undetected char

%{
    #include "fx.hpp"
    char* yyfile = nullptr;
%}

%option noyywrap yylineno

%%
#.*             {}                  // line comment
[ \t\r\n]+      {}                  // drop spaces

"nop"           CMD(nop ,"nop" )
"halt"          CMD(halt,"halt")
"repl"          CMD(repl,"repl")

.               {yyerror("");}      // any undetected char

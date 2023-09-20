%{
    #include "fx.hpp"
    char* yyfile = nullptr;
%}

%option noyywrap yylineno

%%
#.*             {}                  // line comment
[ \t\r\n]+      {}                  // drop spaces

"nop"           CMD(nop)
"halt"          CMD(halt)
"repl"          CMD(repl)

.               {yyerror("");}      // any undetected char

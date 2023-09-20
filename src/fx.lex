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

.               {yyerror("");}      // any undetected char

%{
    #include "fx.hpp"
%}

%defines %union { Object *o; }

%type ex
%token <o> CMD INT

%%
syntax : | syntax ex
ex : CMD             { $1->exec(); }
ex : INT             { D.push_back($1); }

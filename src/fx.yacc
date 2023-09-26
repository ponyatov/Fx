%{
    #include "fx.hpp"
%}

%defines %union { Object *o; }

%type ex
%token <o> CMD INT SYM

%%
syntax  : | syntax ex
ex      : CMD               { $1->exec(); }
        | INT               { D.push_back($1); }
        | SYM               { $1->exec(); }

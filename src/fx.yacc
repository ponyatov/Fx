%{
    #include "fx.hpp"
%}

%defines %union { Object *o; }

%type  <o> ex
%token <o> CMD INT SYM

%%
syntax  : | syntax ex       { $2->exec(); }
ex      : CMD
        | INT
        | SYM

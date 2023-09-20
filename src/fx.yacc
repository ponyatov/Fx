%{
    #include "fx.hpp"
%}

%defines %union { Object *o; }

%type ex
%token <o> CMD

%%
syntax : | syntax ex
ex : CMD             { $1->exec(); }

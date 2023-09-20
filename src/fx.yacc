%{
    #include "fx.hpp"
%}

%defines %union { Object *o; }

%type <o> ex
%token <o> CMD

%%
syntax : | syntax ex { std::cout << $2 << std::endl; }
ex : CMD

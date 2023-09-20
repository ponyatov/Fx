#include "fx.hpp"

int main(int argc, char *argv[]) {  //
    arg(0, argv[0]);
    init(argc, argv);
    for (int i = 1; i < argc; i++) {
        arg(i, argv[i]);
        yyfile = argv[i];
        assert(yyin = fopen(yyfile, "r"));
        yyparse();
        fclose(yyin);
        yyfile = nullptr;
    }
    return fini();
}

void arg(int argc, char *argv) {  //
    std::cerr << "argv[" << argc << "] = <" << argv << ">\n";
}

void yyerror(std::string msg) {
    std::cerr << "\n\n"
              << yyfile << ':' << yylineno << ' ' << msg << " [" << yytext
              << "]\n\n";
    exit(-1);
}

Object::Object() {
    ref = 0;
    next = pool;
    pool = this;
}

Object::~Object() { assert(ref == 0); }

Object *Object::pool = nullptr;

void init(int argc, char *argv[]) {}

int fini(int err) { return err; }

void nop() {}

void halt() { exit(fini()); }

Active::Active() : Object() {}

Cmd::Cmd(void (*F)()) : Active() { fn = F; }

std::map<std::string, Object *> W;
std::vector<Object *> D;

std::string Object::tag() {
    std::string ret(abi::__cxa_demangle(typeid(*this).name(), 0, 0, nullptr));
    for (auto &ch : ret) ch = tolower(ch);
    return ret;
}

void Object::exec() {
    std::cerr << "exec:\t" << this << std::endl;
    D.push_back(this);
}

void Cmd::exec() {
    std::cerr << "cmd:\t" << this->tag() << std::endl;
    this->fn();
}

void repl() {}

#include "fx.hpp"
#include "fx.lexer.hpp"

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

void yyerror(std::string msg) { error(msg, new Str(yytext)); }

Object::Object() {
    ref = 0;
    next = pool;
    pool = this;
}

Object::Object(std::string V) : Object() { value = V; }

Object::~Object() { assert(ref == 0); }

Object *Object::pool = nullptr;

void Object::push(Object *o) { nest.push_back(o); }

void init(int argc, char *argv[]) {  //
    W["?"] = new Cmd(q, "?");
    W["argc"] = new Int(argc);
    Vector *v = new Vector("argv");
    W["argv"] = v;
    v->push(new Str(argv[0]));
}

int fini(int err) {
    SDL_Quit();
    return err;
}

void nop() {}

void halt() { exit(fini()); }

Primitive::Primitive() : Object() {}

Primitive::Primitive(std::string V) : Object(V) {}

Sym::Sym(std::string V) : Primitive(V) {}

Str::Str(std::string V) : Primitive(V) {}

void error(std::string msg, Object *o) {
    std::cerr << "\n\n"
              << yyfile << ':' << yylineno << ' ' << msg << ' ' << o->head()
              << "\n\n";
    exit(fini(-1));
}

void Sym::exec() {
    Object *o = W[value];
    if (!o) {
        error("unknown", this);
    } else
        o->exec();
}

Int::Int(std::string V) : Primitive() { value = stoi(V); }
Int::Int(int N) : Primitive() { value = N; }

std::string Int::val() {
    std::ostringstream os;
    os << value;
    return os.str();
}

Container::Container() : Object() {}
Container::Container(std::string V) : Object(V) {}

Vector::Vector() : Container() {}
Vector::Vector(std::string V) : Container(V) {}

Active::Active() : Object() {}
Active::Active(std::string V) : Object(V) {}

Cmd::Cmd(void (*F)(), std::string V) : Active(V) { fn = F; }

std::map<std::string, Object *> W;
std::vector<Object *> D;

std::string Object::tag() {
    std::string ret(abi::__cxa_demangle(typeid(*this).name(), 0, 0, nullptr));
    for (auto &ch : ret) ch = tolower(ch);
    return ret;
}

std::string Object::val() { return value; }

std::string Object::head() {
    std::ostringstream os;
    os << '<' << tag() << ':' << val() << "> @" << this;
    return os.str();
}

void Object::exec() { D.push_back(this); }

void Cmd::exec() { this->fn(); }

void repl() {
    static char *line = nullptr;
    while (true) {
        line = readline("> ");
        if (!line) q();
        if (line && *line) {
            add_history(line);
            yy_scan_string(line);
            yyparse();
            free(line);
            line = nullptr;
        }
    }
}

void q() {
    std::ostringstream os;
    //
    os << "\n\nW:";
    for (auto const &[name, word] : W)  //
        os << "\n\t" << name << " = " << word->head();
    //
    os << "\n\nD:";
    for (auto &d : D)  //
        os << "\n\t" << d->head();
    //
    os << "\n\n";
    std::cout << os.str();
}

void dot() { D.clear(); }

void tick() {
    yylex();
    D.push_back(yylval.o);
}

Object *pop() {
    assert(!D.empty());
    Object *o = D.back();
    D.pop_back();
    return o;
}

void push(Object *o) {
    D.push_back(o);
    o->ref++;
}

void stor() {
    Object *name = pop();
    Object *o = pop();
    W[name->value] = o;
    delete name;
}

void get() {
    Object *name = pop();
    Object *o = W[name->value];
    assert(o);
    push(o);
    delete name;
}

void gui() {
    assert(!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO));
    // error(SDL_GetError(), new Cmd(gui, "gui"));
    W["gui"] = new Win(W["argv"]->nest[0]->value);
    //
    std::cerr << SDL_GetAudioDeviceName();
    halt();
    SDL_OpenAudioDevice();
}

GUI::GUI(std::string V) : Object(V) {}

Win::Win(std::string V) : GUI(V) {
    assert(window = SDL_CreateWindow(V.c_str(), SDL_WINDOWPOS_UNDEFINED,    //
                                     SDL_WINDOWPOS_UNDEFINED,               //
                                     (dynamic_cast<Int *>(W["W"]))->value,  //
                                     (dynamic_cast<Int *>(W["H"]))->value,  //
                                     SDL_WINDOW_SHOWN));
}

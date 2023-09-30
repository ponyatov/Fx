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

Object::~Object() {}  // assert(ref == 0); }

Object *Object::pool = nullptr;

Object *Object::r() {
    ref++;
    return this;
}

void Object::push(Object *o) { nest.push_back(o->r()); }

Object *Object::pop() {
    Object *o = nest.back();
    assert(o);
    nest.pop_back();
    return o;
}

void init(int argc, char *argv[]) {  //
    vm.slot["?"] = (new Cmd(q, "?"))->r();
    vm.slot["argc"] = (new Int(argc))->r();
    Vector *v = static_cast<Vector *>((new Vector("argv"))->r());
    vm.slot["argv"] = v;
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
    Object *o = vm.slot[value];
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

VM::VM(std::string V) : Active(V) {}

VM vm("init");

Cmd::Cmd(void (*F)(), std::string V) : Active(V) { fn = F; }

std::map<std::string, Object *> W;
std::vector<Object *> D;

std::string Object::tag() {
    std::string ret(abi::__cxa_demangle(typeid(*this).name(), 0, 0, nullptr));
    for (auto &ch : ret) ch = tolower(ch);
    return ret;
}

std::string Object::val() { return value; }

std::string Object::head(std::string prefix) {
    std::ostringstream os;
    os << prefix << '<' << tag() << ':' << val() << "> @" << this << " #"
       << ref;
    return os.str();
}

std::string Object::pad(int depth) {
    std::ostringstream os;
    os << std::endl;
    for (uint i = 0; i < depth; i++) os << '\t';  // tab
    return os.str();
}

std::string Object::dump(int depth, std::string prefix) {
    std::ostringstream os;
    // head
    os << pad(depth) << head(prefix);
    // slot{}s
    for (auto const &[k, v] : slot)  //
        os << v->dump(depth + 1, k + " = ");
    //
    size_t idx = 0;
    for (auto const &i : nest)  //
        os << i->dump(depth + 1, std::to_string(idx++) + ": ");
    //
    return os.str();
}

void Object::exec() { vm.push(this); }

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

void q() { std::cout << vm.dump() << std::endl; }

void Object::dot() { nest.clear(); }
void dot() { vm.dot(); }

void tick() {
    assert(yylex() == SYM);
    vm.push(yylval.o);
}

void stor() {
    Object *name = vm.pop();
    assert(name);
    Object *o = vm.pop();
    assert(o);
    o->slot[name->val()] = o;
}

void get() {
    Object *name = vm.pop();
    Object *o = vm.slot[name->val()];
    assert(o);
    vm.push(o);
    delete name;
}

void sub() {
    assert(yylex() == SYM);
    std::string idx = yylval.o->val();
    q();
}

void audio() {
    assert(!SDL_Init(SDL_INIT_AUDIO));
    //
    Vector *a = new Vector("audio");
    vm.nest["sound"] = a;
    Vector *in = new Vector("in");
    a->slot["in"] = in->r();
    Vector *out = new Vector("out");
    a->slot["out"] = out->r();
    //
    // SDL_AudioSpec *spec = nullptr;
    std::string name;
    for (auto i = 0; i < SDL_GetNumAudioDevices(false); i++) {
        {
            name = SDL_GetAudioDeviceName(i, false);
            // spec = SDL_GetAudioDeviceSpec(i, false, spec);
            out->push((new Audio(name))->r());
        }
    }
    for (auto i = 0; i < SDL_GetNumAudioDevices(true); i++) {
        {
            name = SDL_GetAudioDeviceName(i, true);
            in->push((new Audio(name))->r());
        }
    }
}

void gui() {
    assert(!SDL_Init(SDL_INIT_VIDEO));
    // error(SDL_GetError(), new Cmd(gui, "gui"));
    W["gui"] = (new Win(W["argv"]->nest[0]->value))->r();
    //
    // halt();
    // SDL_OpenAudioDevice();
}

IO::IO(std::string V) : Object(V) {}

GUI::GUI(std::string V) : IO(V) {}

Win::Win(std::string V) : GUI(V) {
    assert(window = SDL_CreateWindow(V.c_str(), SDL_WINDOWPOS_UNDEFINED,    //
                                     SDL_WINDOWPOS_UNDEFINED,               //
                                     (dynamic_cast<Int *>(W["W"]))->value,  //
                                     (dynamic_cast<Int *>(W["H"]))->value,  //
                                     SDL_WINDOW_SHOWN));
}

Audio::Audio(std::string V) : IO(V) {}

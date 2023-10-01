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

void Object::clean() { nest.clear(); }
void clean() { vm.clean(); }

void tick() {
    assert(yylex() == SYM);
    vm.push(yylval.o);
}

void stor() {
    Object *name = vm.pop();
    assert(name);
    Object *o = vm.pop();
    assert(o);
    vm.slot[name->val()] = o;
}

Object *Object::get(std::string idx) {
    Object *o = slot[idx];
    assert(o);
    return o;
}

Object *Object::get(int idx) {
    assert(idx < nest.size());
    Object *o = nest[idx];
    assert(o);
    return o;
}

void get() {
    Object *name = vm.pop();
    Object *o = vm.slot[name->val()];
    assert(o);
    vm.push(o);
    delete name;
}

void dot() {
    // operand
    Object *o = vm.pop();
    assert(o);
    // index
    auto type = yylex();
    // type selector
    switch (type) {
        case SYM:
            vm.push(o->get(yylval.o->val()));
            break;
        case INT:
            vm.push(o->get(dynamic_cast<Int *>(yylval.o)->value));
            break;
        default:
            abort();
    }
}

void sound() {
    assert(!SDL_Init(SDL_INIT_AUDIO));
    //
    Vector *a = new Vector("audio");
    vm.slot["audio"] = a;
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
            out->push((new AuDev(name))->r());
        }
    }
    for (auto i = 0; i < SDL_GetNumAudioDevices(true); i++) {
        {
            name = SDL_GetAudioDeviceName(i, true);
            in->push((new AuDev(name))->r());
        }
    }
}

void gui() {
    assert(!SDL_Init(SDL_INIT_VIDEO));
    // error(SDL_GetError(), new Cmd(gui, "gui"));
    // vm.slot["gui"] = (new Win(vm.slot["argv"]->nest[0]->value))->r();
}

IO::IO(std::string V) : Object(V) {}

GUI::GUI(std::string V) : IO(V) {}

Win::Win(std::string V) : GUI(V) {
    assert(window =
               SDL_CreateWindow(V.c_str(), SDL_WINDOWPOS_UNDEFINED,          //
                                SDL_WINDOWPOS_UNDEFINED,                     //
                                (dynamic_cast<Int *>(vm.slot["W"]))->value,  //
                                (dynamic_cast<Int *>(vm.slot["H"]))->value,  //
                                SDL_WINDOW_SHOWN));
}

Audio::Audio(std::string V) : IO(V) {}
AuDev::AuDev(std::string V) : Audio(V) {}

void IO::open() {}
void IO::close() {}

void open() { dynamic_cast<IO *>(vm.pop())->open(); }
void close() { dynamic_cast<IO *>(vm.pop())->close(); }

void AuDev::open() {  //
    std::cerr << head("\n\nopening ") << std::endl;
    SDL_AudioSpec desired, obtained;
    //
    SDL_memset(&desired, 0, sizeof(desired));
    desired.freq = 48000;
    //
    SDL_AudioDeviceID id =
        SDL_OpenAudioDevice(value.c_str(), 0, &desired, &obtained, 0);
    if (!id) {
        std::cerr << SDL_GetError() << std::endl;
        abort();
    }
    slot["id"] = new Int(id);
    slot["freq"] = new Int(obtained.freq);
}

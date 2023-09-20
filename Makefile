# var
MODULE  = $(notdir $(CURDIR))
OS     += $(shell uname -s)

# src
C  += src/$(MODULE).cpp
H  += inc/$(MODULE).hpp
CP += tmp/$(MODULE).parser.cpp tmp/$(MODULE).lexer.cpp
HP += tmp/$(MODULE).parser.hpp tmp/$(MODULE).lexer.hpp

# cfg
CFLAGS += -Iinc -Itmp
CFLAGS += -Og -g2

# all
.PHONY: all
all: bin/$(MODULE)$(EXE) lib/$(MODULE).ini
	$^

# rule
bin/$(MODULE)$(EXE): $(C) $(CP) $(H) $(HP)
	$(CXX) $(CFLAGS) -o $@ $(C) $(CP) $(L)
tmp/$(MODULE).lexer.cpp: src/$(MODULE).lex
	flex -o $@ $<
tmp/$(MODULE).parser.cpp: src/$(MODULE).yacc
	bison -o $@ $<

# install
.PHONY: install update
install: $(OS)_install
	$(MAKE) update
update:  $(OS)_update

.PHONY: Linux_install Linux_update
Linux_install:
Linux_update:
	sudo apt update
	sudo apt install -yu `cat apt.$(OS)`

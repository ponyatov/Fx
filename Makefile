# var
MODULE  = $(notdir $(CURDIR))
OS     += $(shell uname -s)

# version
BR_VER = 2023.08

# dirs
CWD = $(CURDIR)
GZ  = $(HOME)/gz
FW  = $(CWD)/fw

.PHONY: dirs
dirs:
	mkdir -p $(GZ) $(FW)

# tool
CURL = curl -L -o

# src
C  += src/$(MODULE).cpp
H  += inc/$(MODULE).hpp
CP += tmp/$(MODULE).parser.cpp tmp/$(MODULE).lexer.cpp
HP += tmp/$(MODULE).parser.hpp tmp/$(MODULE).lexer.hpp

# cfg
CFLAGS += -Iinc -Itmp
CFLAGS += -Og -g2

# pkg
BR = buildroot-$(BR_VER)
BR_GZ = $(BR).tar.gz

# all
.PHONY: all
all: bin/$(MODULE)$(EXE) lib/$(MODULE).ini
	$^

# rule
bin/$(MODULE)$(EXE): $(C) $(CP) $(H)
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

# linux
.PHONY: br
br: $(BR)/README.md
$(BR)/README.md: $(GZ)/$(BR).tar.gz
	tar zx < $< && touch $@
$(GZ)/$(BR).tar.gz:
	$(CURL) $@ https://github.com/buildroot/buildroot/archive/refs/tags/$(BR_VER).tar.gz

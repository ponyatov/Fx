# var
MODULE  = $(notdir $(CURDIR))
OS      = $(shell uname -o | tr '/' '_' )
CORES  ?= $(shell grep processor /proc/cpuinfo | wc -l)

# fw
APP ?= $(MODULE)
HW  ?= qemu686

include hw/$(HW).mk
include cpu/$(CPU).mk
include arch/$(ARCH).mk
include app/$(APP).mk

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

L += -lreadline

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

fw/%: $(BR)/output/images/%
	cp $< $@

# install
.PHONY: install update
install: $(OS)_install
	$(MAKE) update
update:  $(OS)_update

.PHONY: Linux_install Linux_update
GNU_Linux_install:
GNU_Linux_update:
	sudo apt update
	sudo apt install -yu `cat apt.$(OS)`

.PHONY: Msys_install Msys_update Msys_deploy
Msys_install:
	pacman -Suy
Msys_update:
	pacman -S `cat apt.Msys | tr '\r\n' ' ' `
#	pacman -Su

# linux
.PHONY: br
br: $(BR)/README.md
	cd $(BR) ;\
	rm -f .config && make allnoconfig &&\
	cat  ../all/all.br     >> .config &&\
	cat ../arch/$(ARCH).br >> .config &&\
	cat  ../cpu/$(CPU).br  >> .config &&\
	cat   ../hw/$(HW).br   >> .config &&\
	cat  ../app/$(APP).br  >> .config &&\
	echo 'BR2_DL_DIR="$(GZ)"'                        >> .config &&\
	echo 'BR2_TARGET_GENERIC_HOSTNAME="$(APP)"'      >> .config &&\
	echo 'BR2_TARGET_GENERIC_ISSUE="$(APP) @ $(HW)"' >> .config &&\
	echo 'BR2_ROOTFS_OVERLAY="$(CWD)/root"'          >> .config &&\
	echo 'BR2_LINUX_KERNEL_CUSTOM_CONFIG_FILE="$(CWD)/all/all.kernel"' >> .config &&\
	echo 'BR2_LINUX_KERNEL_CONFIG_FRAGMENT_FILES="$(CWD)/arch/$(ARCH).kernel $(CWD)/cpu/$(CPU).kernel $(CWD)/hw/$(HW).kernel $(CWD)/app/$(APP).kernel"' >> .config &&\
 	make menuconfig && make linux-menuconfig && make -j$(CORES)

.PHONY: fw
fw: fw/bzImage fw/rootfs.cpio fw/rootfs.iso9660

.PHONY: qemu
qemu: fw/bzImage fw/rootfs.cpio
	$(QEMU) $(QEMU_CFG) -kernel fw/bzImage -initrd fw/rootfs.cpio

$(BR)/README.md: $(GZ)/$(BR).tar.gz
	tar zx < $< && touch $@
$(GZ)/$(BR).tar.gz:
	$(CURL) $@ https://github.com/buildroot/buildroot/archive/refs/tags/$(BR_VER).tar.gz


# net
.PHONY: dhcp
dhcp:
	sudo journalctl -u isc-dhcp-server -r

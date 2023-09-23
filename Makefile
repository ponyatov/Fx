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
BR_VER       = 2023.08
SYSLINUX_VER = 6.03

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

SYSLINUX    = syslinux-$(SYSLINUX_VER)
SYSLINUX_GZ = $(SYSLINUX).tar.xz

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
fw: fw/bzImage fw/rootfs.cpio fw/rootfs.iso9660 \
	fw/pxelinux.0 fw/ldlinux.c32 fw/menu.c32 fw/vesamenu.c32

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
	sudo journalctl -u isc-dhcp-server -f

.PHONY: tftp
tftp:
	sudo journalctl -u tftpd-hpa -f

.PHONY: dhclient
dhclient:
	sudo /usr/sbin/dhclient -v -d enp2s0f0

.PHONY: etc
etc:
	rsync -r /etc/default/*dhcp*         etc/default/
	rsync -r /etc/default/*tftp*         etc/default/
	rsync -r /etc/dhcp/dhcpd.conf      etc/dhcp/
	rsync -r /etc/network/interfaces   etc/network/
	rsync -r /etc/network/interfaces.d etc/network/

.PHONY: services
services:
	sudo systemctl enable tftpd-hpa
	sudo systemctl enable isc-dhcp-server

.PHONY: syslinux
syslinux: $(SYSLINUX)/README
$(SYSLINUX)/README: $(GZ)/$(SYSLINUX_GZ)
	xzcat $< | tar x && touch $@
$(GZ)/$(SYSLINUX_GZ):
	$(CURL) $@ https://mirrors.edge.kernel.org/pub/linux/utils/boot/syslinux/$(SYSLINUX_GZ)

fw/%: $(SYSLINUX)/bios/core/%
	cp $< $@
fw/%: $(SYSLINUX)/bios/com32/elflink/ldlinux/%
	cp $< $@
fw/%: $(SYSLINUX)/bios/com32/menu/%
	cp $< $@

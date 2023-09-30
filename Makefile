# var
MODULE  = $(notdir $(CURDIR))
OS      = $(shell uname -o | sed 's/GNU\///')
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
IDS_VER      = 2.2

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
F  += lib/$(MODULE).ini

# cfg
CFLAGS += -Iinc -Itmp -std=c++17
CFLAGS += -Og -g2

L += -lreadline

# pkg
BR    = buildroot-$(BR_VER)
BR_GZ = $(BR).tar.xz

SYSLINUX    = syslinux-$(SYSLINUX_VER)
SYSLINUX_GZ = $(SYSLINUX).tar.xz

# all
.PHONY: all
all: $(BIN)/$(MODULE)$(EXE) $(F)
	$^

$(BIN)/$(MODULE)$(EXE): $(S) $(TMP)/$(MODULE)/Makefile
	$(MAKE) -C $(TMP)/$(MODULE)

$(TMP)/$(MODULE)/Makefile: $(S)
	cmake -B $(TMP)/$(MODULE) -S $(CWD) \
		-DAPP=$(MODULE) -DEXECUTABLE_OUTPUT_PATH=$(BIN)

# rule
# bin/$(MODULE)$(EXE): $(C) $(CP) $(H)
# 	$(CXX) $(CFLAGS) -o $@ $(C) $(CP) $(L)
tmp/$(MODULE).lexer.cpp: src/$(MODULE).lex
	flex -o $@ $<
tmp/$(MODULE).parser.cpp: src/$(MODULE).yacc
	bison -o $@ $<

fw/%: $(BR)/output/images/%
	cp $< $@

# doc
.PHONY: doxygen
doxy: .doxygen
	rm -rf docs ; doxygen $< 1>/dev/null

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
	echo 'BR2_UCLIBC_CONFIG_FRAGMENT_FILES="$(CWD)/all/all.uclibc"'    >> .config &&\
	echo 'BR2_LINUX_KERNEL_CUSTOM_CONFIG_FILE="$(CWD)/all/all.kernel"' >> .config &&\
	echo 'BR2_LINUX_KERNEL_CONFIG_FRAGMENT_FILES="$(CWD)/arch/$(ARCH).kernel $(CWD)/cpu/$(CPU).kernel $(CWD)/hw/$(HW).kernel $(CWD)/app/$(APP).kernel"' >> .config &&\
 	make menuconfig && make linux-menuconfig && make -j$(CORES)

QEMU_CFG    += -netdev user,id=mynet0,net=192.168.76.0/24,dhcpstart=192.168.76.9
QEMU_APPEND += -vga=0x315
.PHONY: qemu
qemu: fw
	$(QEMU) $(QEMU_CFG) \
		-kernel fw/bzImage -initrd fw/rootfs.cpio \
		-append $(QEMU_APPEND)

$(BR)/README.md: $(GZ)/$(BR_GZ)
	xzcat $< | tar x && touch $@
$(GZ)/$(BR_GZ):
	$(CURL) $@ https://buildroot.org/downloads/$(BR_GZ)

# net
.PHONY: dhcp
dhcp:
	sudo journalctl -u isc-dhcp-server -f

.PHONY: tftp
tftp:
	sudo journalctl -u tftpd-hpa -f

.PHONY: nfs
nfs:
	sudo journalctl -u nfs -f

.PHONY: dhclient
dhclient:
	sudo /usr/sbin/dhclient -v -d enp2s0f0

HOSTNAME = $(shell hostname)
.PHONY: etc
etc:
	mkdir -p etc/$(HOSTNAME)
	-rsync -r /etc/default/*dhcp*         etc/$(HOSTNAME)/default/
	-rsync -r /etc/default/*tftp*         etc/$(HOSTNAME)/default/
	-rsync -r /etc/dhcp/dhcpd.conf        etc/$(HOSTNAME)/dhcp/
	-rsync -r /etc/network/interfaces     etc/$(HOSTNAME)/network/
	-rsync -r /etc/network/interfaces.d/* etc/$(HOSTNAME)/network/interfaces.d/

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

fw/%: $(SYSLINUX)/bios/com32/hdt/%
	cp $< $@
fw/pci.ids: $(GZ)/pci-$(IDS_VER).ids.xz
	xzcat $< > $@
$(GZ)/pci-$(IDS_VER).ids.xz:
	$(CURL) $@ https://pci-ids.ucw.cz/v$(IDS_VER)/pci.ids.xz

.PHONY: rootfs
rootfs:
	cd $(BR) ; make rootfs-cpio
	$(MAKE) fw

.PHONY: fw
fw: fw/bios/bzImage fw/bios/rootfs.cpio

fw/bios/%: $(BR)/output/images/%
	cp $< $@

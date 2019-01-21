#############################################################

# Discard this section from all parent makefiles
# Expected variables (with automatic defaults):
#   CSRCS (all "C" files in the dir)
#   SUBDIRS (all subdirs with a Makefile)
#   GEN_LIBS - list of libs to be generated ()
#   GEN_IMAGES - list of object file images to be generated ()
#   GEN_BINS - list of binaries to be generated ()
#   COMPONENTS_xxx - a list of libs/objs in the form
#     subdir/lib to be extracted and rolled up into
#     a generated lib/image xxx.a ()
#
TARGET = eagle
#FLAVOR = release
FLAVOR = debug

#EXTRA_CCFLAGS += -u

ifndef PDIR # {
GEN_IMAGES= eagle.app.v6.out
GEN_BINS= eagle.app.v6.bin
SPECIAL_MKTARGETS=$(APP_MKTARGETS)
SUBDIRS=    \
	firmware 

endif # } PDIR

APPDIR = .
LDDIR = ../ld

CCFLAGS += -Os

TARGET_LDFLAGS =		\
	-nostdlib		\
	-Wl,-EL \
	--longcalls \
	--text-section-literals

ifeq ($(FLAVOR),debug)
    TARGET_LDFLAGS += -g -O2
endif

ifeq ($(FLAVOR),release)
    TARGET_LDFLAGS += -g -O0
endif

COMPONENTS_eagle.app.v6 = \
	firmware/libwifipicprog.a 

LINKFLAGS_eagle.app.v6 = \
	-L../lib        \
	-nostdlib	\
    -T$(LD_FILE)   \
	-Wl,--no-check-sections	\
	-Wl,--gc-sections	\
    -u call_user_start	\
	-Wl,-static						\
	-Wl,--start-group					\
	-lc					\
	-lgcc					\
	-lphy	\
	-lpp	\
	-lnet80211	\
	-llwip	\
	-lwpa	\
	-lcrypto	\
	-lmain	\
	-lupgrade\
	-ldriver \
	-lhal					\
	$(DEP_LIBS_eagle.app.v6)					\
	-Wl,--end-group

#	-ljson	\
#	-lsmartconfig \
#	-lmbedtls	\
#	-lpwm	\
	
DEPENDS_eagle.app.v6 = \
                $(LD_FILE) \
                $(LDDIR)/eagle.rom.addr.v6.ld

#############################################################
# Configuration i.e. compile options etc.
# Target specific stuff (defines etc.) goes in here!  # Generally values applying to a tree are captured in the
#   makefile at its root level - these are then overridden
#   for a subtree within the makefile rooted therein
#

#UNIVERSAL_TARGET_DEFINES =		\

# Other potential configuration flags include:
#	-DTXRX_TXBUF_DEBUG
#	-DTXRX_RXBUF_DEBUG
#	-DWLAN_CONFIG_CCX
CONFIGURATION_DEFINES =	-DICACHE_FLASH \
                        -DGLOBAL_DEBUG_ON

DEFINES +=				\
	$(UNIVERSAL_TARGET_DEFINES)	\
	$(CONFIGURATION_DEFINES)

DDEFINES +=				\
	$(UNIVERSAL_TARGET_DEFINES)	\
	$(CONFIGURATION_DEFINES)


#############################################################
# Recursion Magic - Don't touch this!!
#
# Each subtree potentially has an include directory
#   corresponding to the common APIs applicable to modules
#   rooted at that subtree. Accordingly, the INCLUDE PATH
#   of a module can only contain the include directories up
#   its parent path, and not its siblings
#
# Required for each makefile to inherit from the parent
#

INCLUDES := $(INCLUDES) \
	-I $(PDIR)/firmware/include

PDIR := ../$(PDIR)
sinclude $(PDIR)Makefile

.PHONY: flash flash_map2 flash_map3 flash_map5


ESPTOOL = esptool.py --baud 1152000 write_flash -u --flash_mode qio \
		  --flash_freq 40m

# 1024KB( 512KB+ 512KB)
flash_map2:
	make clean
	make COMPILE=gcc BOOT=new APP=1 SPI_SPEED=40 SPI_MODE=QIO SPI_SIZE_MAP=2
	$(ESPTOOL) --flash_size 1MB  \
		0x0 	../bin/boot_v1.7.bin \
		0x1000  ../bin/upgrade/user1.1024.new.2.bin \
		0xfc000 ../bin/esp_init_data_default_v08.bin \
		0xfb000 ../bin/blank.bin \
		0xfe000 ../bin/blank.bin


# 2048KB( 512KB+ 512KB)
flash_map3:
	make clean
	make COMPILE=gcc BOOT=new APP=1 SPI_SPEED=40 SPI_MODE=QIO SPI_SIZE_MAP=3
	$(ESPTOOL) --flash_size 2MB  \
		0x0 	../bin/boot_v1.7.bin \
		0x1000  ../bin/upgrade/user1.2048.new.3.bin \
		0x1fc000 ../bin/esp_init_data_default_v08.bin \
		0x1fb000 ../bin/blank.bin

	#	0x1fe000 ../bin/blank.bin

# 4096KB( 512KB+ 512KB)
flash_map4:
	make clean
	make COMPILE=gcc BOOT=new APP=1 SPI_SPEED=40 SPI_MODE=QIO SPI_SIZE_MAP=4
	$(ESPTOOL) --flash_size 4MB  \
		0x0 	../bin/boot_v1.7.bin \
		0x1000  ../bin/upgrade/user1.4096.new.4.bin \
		0x3fc000 ../bin/esp_init_data_default_v08.bin \
		0x3fb000 ../bin/blank.bin
	#	0x3fe000 ../bin/blank.bin 

# 2048KB(1024KB+1024KB)
flash_map5:
	make clean
	make COMPILE=gcc BOOT=new APP=1 SPI_SPEED=40 SPI_MODE=QIO SPI_SIZE_MAP=5
	$(ESPTOOL) --flash_size 16m-c1  \
		0x0 	../bin/boot_v1.2.bin \
		0x1000  ../bin/upgrade/user1.2048.new.5.bin \
		0x1fc000 ../bin/esp_init_data_default_v05.bin \
		0x1fb000 ../bin/blank.bin \
		0x1fe000 ../bin/blank.bin 

# 4096KB(1024KB+1024KB)
flash_map6:
	make clean
	make COMPILE=gcc BOOT=new APP=1 SPI_SPEED=40 SPI_MODE=QIO SPI_SIZE_MAP=6
	$(ESPTOOL) --flash_size 32m-c1  \
		0x0 	../bin/boot_v1.7.bin \
		0x1000  ../bin/upgrade/user1.4096.new.6.bin \
		0x3fc000 ../bin/esp_init_data_default_v08.bin \
		0x3fb000 ../bin/blank.bin \
		0x3fe000 ../bin/blank.bin 


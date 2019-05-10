
SYSROOT=$(WP76XX_SYSROOT)
PREFIX=$(WP76XX_TOOLCHAIN_PREFIX)
TOOLCHAIN=$(WP76XX_TOOLCHAIN_DIR)

CC      = $(TOOLCHAIN)/$(PREFIX)gcc
CFLAGS  = -Wall
LDFLAGS =

OBJ = writeFile2Eeprom.o


#all:

writeFile2Eeprom: $(OBJ)
	$(CC) $(CFLAGS) --sysroot=$(SYSROOT) -o writeFile2Eeprom $(OBJ) $(LDFLAGS)


%.o: %.c
ifeq ($(TOOLCHAIN),)
    $(error toolchain directory is not set - please run this in Legato ./bin/legs environment and check the Makefile accordingly to your Hardware)
endif
ifeq ($(SYSROOT),)
    $(error sysroot is not set - please run this in Legato ./bin/legs environment and check the Makefile accordingly to your Hardware)
endif
ifeq ($(PREFIX),)
    $(error toolchain prefix is not set - please run this in Legato ./bin/legs environment and check the Makefile accordingly to your Hardware)
endif
	$(CC) $(CFLAGS) --sysroot=$(SYSROOT) -c $<


clean:
	rm -Rf writeFile2Eeprom *.o



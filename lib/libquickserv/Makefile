# Dependencies
LIBEVQUICK:=./libevquick

# Source files
SOURCES:=./libquickserv.c
SOURCES+=$(LIBEVQUICK)/libevquick.c

# Organization
PREFIX?=./build
LIBNAME:="libquickserv.a"

# Customization
CC:=gcc
AR:=ar
RANLIB:=ranlib
CFLAGS=-Wall

# Object files
OBJECTS:=$(SOURCES:.c=.o)

ifeq ($(DEBUG), 1)
CFLAGS+=-g -ggdb
endif

ifeq ($(DEBUG_QUICKSERV), 1)
EXTRA_CFLAGS+=-DDBG_QUICKSERV
endif

all: lib

# Compilation-step
%.o:%.c deps
	@echo "\t[CC] $@"
	$(CC) -c $(CFLAGS) $(EXTRA_CFLAGS) -o $@ $< -I. -I$(LIBEVQUICK)
	@mv $@ $(PREFIX)

deps:
	@echo "\t[MKDIR] $(PREFIX)/lib $(PREFIX)/include"
	@mkdir -p $(PREFIX)/lib
	@mkdir -p $(PREFIX)/include

# Link together in library
lib: $(OBJECTS)
	@cp -f *.h $(PREFIX)/include
	@echo "\t[AR] $(PREFIX)/lib/$(LIBNAME)"
	$(AR) cru $(PREFIX)/lib/$(LIBNAME) $(PREFIX)/*.o
	@echo "\t[RANLIB] $(PREFIX)/lib/$(LIBNAME)"
	@$(RANLIB) $(PREFIX)/lib/$(LIBNAME)
	@echo "\t[LIBSIZE] `du -b $(PREFIX)/lib/$(LIBNAME)`"
	@echo "`size -t $(PREFIX)/lib/$(LIBNAME)`"

clean:
	@echo "\t[CLEAN] $(PREFIX)/"
	@rm -rf $(PREFIX)

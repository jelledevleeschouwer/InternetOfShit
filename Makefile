APP_SOURCE?=app.c

# Customization
CC:=gcc
CFLAGS=-Wall

# Organization
PREFIX?=$(PWD)/build
LIBDIR?=$(PWD)/lib

# Dependencies
MCP3008?=$(LIBDIR)/mcp3008
QUICKSERV?=$(LIBDIR)/libquickserv

# Log
DEBUG_APP?=1

# Include debugging-symbols
ifeq ($(DEBUG), 1)
	CFLAGS+=-g
	CFLAGS+=-ggdb
endif

ifeq ($(DEBUG_APP), 1)
	EXTRA_CFLAGS+=-DDBG_APP
endif
ifeq ($(DEBUG_MCP3008), 1) # Debug MCP3008 library
	EXTRA_CFLAGS+=-DDBG_MCP3008
endif

all: app

app: $(PREFIX)/app.o $(PREFIX)/mcp3008.o $(PREFIX)/lib/libquickserv.a
	$(CC) -o $(PREFIX)/$@ $^ $(DEBUG_FLAGS) -lbcm2835

$(PREFIX)/app.o: $(APP_SOURCE) deps
	$(CC) -o $@ -c $< $(EXTRA_CFLAGS) $(CFLAGS) -I$(MCP3008) -I$(PREFIX)/include

$(PREFIX)/mcp3008.o: $(MCP3008)/mcp3008.c
	$(CC) -o $@ -c $< $(EXTRA_CFLAGS) $(CFLAGS)

deps:
	@echo "\t[MKDIR] $(PREFIX)"
	@mkdir -p $(PREFIX)
	@make -C $(QUICKSERV) lib PREFIX=$(PREFIX) $(DEBUG)

clean:
	@echo "\t[CLEAN] $(PREFIX)/"
	rm -rf $(PREFIX)
	@make -C $(QUICKSERV) clean

run:
	@echo "\n\nStarting application..."
	@sudo ./app

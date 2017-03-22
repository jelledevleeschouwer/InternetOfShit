APP_SOURCE?=app.c

# Customization
CC:=gcc
CFLAGS=-Wall

# Organization
PREFIX?=$(PWD)/build

# Dependencies
LIBEVQUICK:=$(PWD)/libevquick
SERVKIT:=$(PWD)/servkit
MCP3008:=$(PWD)/mcp3008

# Log
DEBUG_APP?=1

# Include debugging-symbols
ifeq ($(DEBUG), 1)
	CFLAGS+=-g
	CFLAGS+=-ggdb
endif

ifeq ($(DEBUG_APP), 1)
	DEBUG_FLAGS+=-DDBG_APP
endif
ifeq ($(DEBUG_MCP3008), 1) # Debug MCP3008 library
	DEBUG_FLAGS+=-DDBG_MCP3008
endif
ifeq ($(DEBUG_SERVKIT), 1) # Debug Servkit library
	DEBUG_FLAGS+=-DDBG_SERVKIT
endif

$(PREFIX)/app.o: $(APP_SOURCE)
	@mkdir -p $(PREFIX)
	$(CC) -o $@ -c $^ $(DEBUG_FLAGS) $(CFLAGS) -I$(SERVKIT) -I$(MCP3008)

$(PREFIX)/mcp3008.o: $(MCP3008)/mcp3008.c
	@mkdir -p $(PREFIX)
	$(CC) -o $@ -c $^ $(DEBUG_FLAGS) $(CFLAGS)

$(PREFIX)/servkit.o: $(SERVKIT)/servkit.c
	@mkdir -p $(PREFIX)
	$(CC) -o $@ -c $^ $(DEBUG_FLAGS) $(CFLAGS) -I$(LIBEVQUICK)

$(PREFIX)/libevquick.o: $(LIBEVQUICK)/libevquick.c
	@mkdir -p $(PREFIX)
	$(CC) -o $@ -c $^ -O0 $(DEBUG_FLAGS) $(CFLAGS)

#############################
# TARGETS
#############################

all: app

app: $(PREFIX)/app.o $(PREFIX)/servkit.o $(PREFIX)/mcp3008.o $(LIBEVQUICK)/libevquick.o
	$(CC) -o $@ $^ $(DEBUG_FLAGS) -lbcm2835

clean:
	rm -f $(PREFIX)/*.o
	rm -f app

run:
	@echo "\n\nStarting application..."
	@sudo ./app

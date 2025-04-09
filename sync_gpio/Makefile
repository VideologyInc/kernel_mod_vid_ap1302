# tool macros
CC ?= gcc
CCFLAGS ?= -Iincludes/
DBGFLAGS ?= -g
LDLIBS = -lgpiod
CCOBJFLAGS = $(CCFLAGS) -c

# path macros
BIN_PATH := build/bin
OBJ_PATH := build/obj
SRC_PATH := src
DBG_PATH := build/debug

# compile macros
TARGET_NAME := gpio_sync_trigger
TARGET := $(BIN_PATH)/$(TARGET_NAME)
TARGET_DEBUG := $(DBG_PATH)/$(TARGET_NAME)

# src files & obj files
SRC := $(wildcard $(SRC_PATH)/*.c*)
OBJ := $(addprefix $(OBJ_PATH)/, $(addsuffix .o, $(notdir $(basename $(SRC)))))
OBJ_DEBUG := $(addprefix $(DBG_PATH)/, $(addsuffix .o, $(notdir $(basename $(SRC)))))

# clean files list
DISTCLEAN_LIST := $(OBJ) \
					$(OBJ_DEBUG)

CLEAN_LIST := $(TARGET) \
				$(TARGET_DEBUG) \
				$(DISTCLEAN_LIST)

# default rule
default: all

# non-phony targets
$(TARGET): $(OBJ)
	$(CC) $(CCFLAGS) -o $@ $(OBJ) $(LDFLAGS) $(LDLIBS)

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.c*
	$(CC) $(CCOBJFLAGS) -o $@ $<

$(DBG_PATH)/%.o: $(SRC_PATH)/%.c*
	$(CC) $(CCOBJFLAGS) $(DBGFLAGS) -o $@ $<

$(TARGET_DEBUG): $(OBJ_DEBUG)
	$(CC) $(CCFLAGS) $(DBGFLAGS) -o $@ $(OBJ_DEBUG) $(LDFLAGS) $(LDLIBS)

# phony rules
.PHONY: default makedir makedir_debug install

makedir:
	@mkdir -p $(BIN_PATH) $(OBJ_PATH)

makedir_debug:
	@mkdir -p $(DBG_PATH)

.PHONY: all
all: makedir $(TARGET)

.PHONY: debug
debug: makedir makedir_debug $(TARGET_DEBUG)

.PHONY: clean
clean:
	@echo CLEAN $(CLEAN_LIST)
	@rm -f $(CLEAN_LIST)

.PHONY: distclean
distclean:
	@echo CLEAN $(DISTCLEAN_LIST)
	@rm -f $(DISTCLEAN_LIST)

# Install target for Yocto
.PHONY: install
install:
	install -d $(DESTDIR)$(bindir)
	install -m 0755 $(TARGET) $(DESTDIR)$(bindir)/

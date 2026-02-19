CC      ?= clang
AR      ?= llvm-ar
LD      ?= ld.lld

BUILD_DIR := build
LIB_BUILD_DIR := $(BUILD_DIR)/lib
LIB_TARGET  := $(LIB_BUILD_DIR)/libapglue.a

TEST_BUILD_DIR := $(BUILD_DIR)/test
TEST_TARGET  := $(TEST_BUILD_DIR)/test.elf

LDSCRIPT := mod.ld
ARCHFLAGS := -target mips -mips2 -mabi=32 -O2 -G0 -mno-abicalls -mno-odd-spreg -mno-check-zero-division \
             -fomit-frame-pointer -ffast-math -fno-unsafe-math-optimizations -fno-builtin-memset
WARNFLAGS := -Wall -Wextra -Wno-incompatible-library-redeclaration -Wno-unused-parameter -Wno-unknown-pragmas -Wno-unused-variable \
             -Wno-missing-braces -Wno-unsupported-floating-point-opt -Werror=section
CFLAGS   := $(ARCHFLAGS) $(WARNFLAGS) -D_LANGUAGE_C -nostdinc -ffunction-sections
CPPFLAGS := -DMIPS -DF3DEX_GBI_2 -DF3DEX_GBI_PL -DGBI_DOWHILE -I include -idirafter include/libc
LIB_ARFLAGS := rc

TEST_LDFLAGS  := -nostdlib -T $(LDSCRIPT) -Map $(TEST_BUILD_DIR)/mod.map --unresolved-symbols=ignore-all --emit-relocs -e 0 --no-nmagic -gc-sections \
			-L ./build/lib -lapglue

rwildcard = $(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))
getdirs = $(sort $(dir $(1)))

LIB_C_SRCS := $(call rwildcard,src/c,*.c)
LIB_C_OBJS := $(addprefix $(LIB_BUILD_DIR)/, $(LIB_C_SRCS:.c=.o))
LIB_C_DEPS := $(addprefix $(LIB_BUILD_DIR)/, $(LIB_C_SRCS:.c=.d))

LIB_ALL_OBJS := $(LIB_C_OBJS)
LIB_ALL_DEPS := $(LIB_C_DEPS)
LIB_BUILD_DIRS := $(call getdirs,$(LIB_ALL_OBJS))

TEST_C_SRCS := $(call rwildcard,test/c,*.c)
TEST_C_OBJS := $(addprefix $(TEST_BUILD_DIR)/, $(TEST_C_SRCS:.c=.o))
TEST_C_DEPS := $(addprefix $(TEST_BUILD_DIR)/, $(TEST_C_SRCS:.c=.d))

TEST_ALL_OBJS := $(TEST_C_OBJS)
TEST_ALL_DEPS := $(TEST_C_DEPS)
TEST_BUILD_DIRS := $(call getdirs,$(TEST_ALL_OBJS))

all: $(LIB_TARGET) $(TEST_TARGET)

$(LIB_TARGET): $(LIB_ALL_OBJS) | $(LIB_BUILD_DIR)
	$(AR) $(LIB_ARFLAGS) $(LIB_TARGET) $(LIB_ALL_OBJS)

$(TEST_TARGET): $(TEST_ALL_OBJS) $(LDSCRIPT) $(LIB_TARGET) | $(TEST_BUILD_DIR)
	$(LD) $(TEST_ALL_OBJS) $(TEST_LDFLAGS) -o $@

$(BUILD_DIR) $(LIB_BUILD_DIR) $(LIB_BUILD_DIRS) $(TEST_BUILD_DIR) $(TEST_BUILD_DIRS):
ifeq ($(OS),Windows_NT)
	if not exist "$(subst /,\,$@)" mkdir "$(subst /,\,$@)"
else
	mkdir -p $@
endif

$(LIB_C_OBJS): $(LIB_BUILD_DIR)/%.o : %.c | $(LIB_BUILD_DIRS)
	$(CC) $(CFLAGS) $(CPPFLAGS) $< -MMD -MF $(@:.o=.d) -c -o $@

$(TEST_C_OBJS): $(TEST_BUILD_DIR)/%.o : %.c | $(TEST_BUILD_DIRS)
	$(CC) $(CFLAGS) $(CPPFLAGS) $< -MMD -MF $(@:.o=.d) -c -o $@

clean:
ifeq ($(OS),Windows_NT)
	if exist $(BUILD_DIR) rmdir /S /Q $(BUILD_DIR)
else
	rm -rf $(BUILD_DIR)
endif

-include $(LIB_ALL_DEPS)

.PHONY: clean all

# Print target for debugging
print-% : ; $(info $* is a $(flavor $*) variable set to [$($*)]) @true
# Shared makefile rules for libraries

# Directories to store object and library files
OBJ_DIR := linux/obj/$(LIB_NAME)/$(TT_CONFIG)
LIB_DIR := linux/lib/$(TT_CONFIG)
OUT     := $(LIB_DIR)/$(LIB_PREFIX)$(LIB_NAME).a

# Include Files
INC_DIR := linux/inc shared/inc
INCLUDES := $(addprefix -I, $(INC_DIR))

# Source Files
CPP_SOURCES  := $(foreach sdir, $(SRC_DIR), $(wildcard $(sdir)/*.cpp))
CPP_SOURCES  := $(filter-out $(EXCLUDE_SRC), $(CPP_SOURCES))
C_SOURCES    := $(foreach sdir, $(SRC_DIR), $(wildcard $(sdir)/*.c))
C_SOURCES    := $(filter-out $(EXCLUDE_SRC), $(C_SOURCES))

# Generate a list of objects (based on src dirs)
CPP_OBJECTS := ${CPP_SOURCES:%.cpp=${OBJ_DIR}/%.o}
C_OBJECTS   := ${C_SOURCES:%.c=${OBJ_DIR}/%.o}
OBJECTS := $(CPP_OBJECTS) $(C_OBJECTS)

all: $(OUT)

# Link library
$(OUT): $(OBJECTS)
	@mkdir -p $(dir $@)
	ar rcs $(OUT) $(OBJECTS)

# Compile C++ files
${OBJ_DIR}/%.o: %.cpp
	@mkdir -p $(dir $@)
	@echo Compiling $< to $@
	$(CCPP) $(DEFINES) $(CFLAGS) $(CPPFLAGS) -c $< -o $@ $(INCLUDES)

# Compile C files
${OBJ_DIR}/%.o: %.c
	@mkdir -p $(dir $@)
	@echo Compiling $< to $@
	$(CC) $(DEFINES) $(CFLAGS) -c $< -o $@ $(INCLUDES)

.PHONY: all clean rebuild test gen_revision

gen_revision:
ifeq ($(LIB_NAME), shared)
	$(TTDEV_ROOT)/tools/linux/revision_info/update_revision_info.sh $(TTDEV_ROOT)/libs $(TTDEV_ROOT)/libs/shared/inc/tt/version $(TTDEV_ROOT)/tools/linux/revision_info/__revision_template_libs.h
endif

clean:
	@rm -vrf $(OUT) $(OBJ_DIR)

test:
	@echo $(CPP_SOURCES)

rebuild: clean all


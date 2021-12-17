# Common mk Definitions for TT libs

# Set build independent flags
ifeq ($(WARNING_FULL), False)
	CFLAGS := -w
else
	CFLAGS := -Wall -Wextra
endif
DEFINITIONS := TT_PLATFORM_LNX

CC   := gcc
CCPP := g++
CPPFLAGS := -std=gnu++0x

# Ignore the following warnings
CFLAGS := $(CFLAGS) -Wno-missing-field-initializers
CFLAGS := $(CFLAGS) -Wno-ignored-qualifiers
CFLAGS := $(CFLAGS) -Wno-missing-braces # Seems to be incorrectly triggered by OpenALSoundSystem

# 32-bit build
CFLAGS := $(CFLAGS) -m32

# Check build configuration
# TODO: Check if config is valid
ifndef $(TT_CONFIG)
	TT_CONFIG := LNX_Dev
endif

ifeq ($(TT_CONFIG), LNX_Dev)
	DEFINITIONS := $(DEFINITIONS) TT_BUILD_DEV _DEBUG TT_ASSERT_ON
	CFLAGS := $(CFLAGS) -g
endif

ifeq ($(TT_CONFIG), LNX_Test)
	DEFINITIONS := $(DEFINITIONS) TT_BUILD_TEST NDEBUG TT_ASSERT_ON
	CFLAGS := $(CFLAGS) -g -O2
endif

ifeq ($(TT_CONFIG), LNX_Final)
	DEFINITIONS := $(DEFINITIONS) TT_BUILD_FINAL NDEBUG
	CFLAGS := $(CFLAGS) -O3
endif

# Expand definitions
DEFINES := $(addprefix -D, $(DEFINITIONS))


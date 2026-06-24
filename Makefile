# ---------------------------------------------------------------------------
# Build configurations
#   make                 Debug build (default): -Og + debug info, plus the
#                        AddressSanitizer and UndefinedBehaviorSanitizer
#                        runtime checks (buffer overflows, use-after-free,
#                        leaks, integer/UB bugs).
#   make release         Optimized production build: -O3, LTO, asserts off.
#   make BUILD=release   Same as `make release`.
#   make clean           Remove all build artifacts.
#   make help            Show this summary.
#
# Switching between debug and release automatically forces a full rebuild, so
# you never link objects that were compiled with mismatched flags.
# ---------------------------------------------------------------------------

CC     := gcc
TARGET := myls
OBJS   := main.o entry.o mode.o print.o

# Active configuration; override on the command line with BUILD=release.
BUILD ?= debug

# Flags shared by every configuration.
CFLAGS_COMMON := -std=gnu11 -Wall -Wextra

# Debug: keep code debuggable (-Og -g3) and instrument it. ASan catches memory
# bugs (overflows, use-after-free, leaks); UBSan catches undefined behavior.
# -fno-sanitize-recover=all makes the program abort on the first error found so
# bugs can't slip by unnoticed. -fno-omit-frame-pointer gives clean stack
# traces. The sanitizer flags are needed at both compile and link time; because
# the link recipe below reuses $(CFLAGS), listing them here covers both.
CFLAGS_debug := -Og -g3 -fno-omit-frame-pointer \
                -fsanitize=address,undefined -fno-sanitize-recover=all

# Release: full optimization, drop assert()s (NDEBUG), link-time optimization.
CFLAGS_release := -O3 -DNDEBUG -flto

ifeq ($(BUILD),debug)
    CFLAGS := $(CFLAGS_COMMON) $(CFLAGS_debug)
else ifeq ($(BUILD),release)
    CFLAGS := $(CFLAGS_COMMON) $(CFLAGS_release)
else
    $(error Unknown BUILD '$(BUILD)' -- use BUILD=debug or BUILD=release)
endif

# Stamp file recording the active configuration. Every object depends on it, so
# changing BUILD makes the stamp stale, which wipes and rebuilds everything.
STAMP := .build-$(BUILD)

.PHONY: all debug release clean help

all: $(TARGET)

# Convenience aliases for the two configurations.
debug:
	$(MAKE) BUILD=debug
release:
	$(MAKE) BUILD=release

# Link with $(CFLAGS) so the sanitizer / LTO flags apply to this step too.
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

# Objects are compiled by make's built-in rule; these lines only declare the
# header and configuration-stamp prerequisites that trigger a rebuild.
main.o:  main.c entry.h print.h $(STAMP)
entry.o: entry.c entry.h mode.h $(STAMP)
mode.o:  mode.c mode.h $(STAMP)
print.o: print.c print.h entry.h $(STAMP)

# Creating the stamp for a new configuration removes artifacts left over from
# the other one, forcing a clean rebuild with the correct flags.
$(STAMP):
	rm -f .build-* $(OBJS) $(TARGET)
	touch $@

clean:
	rm -f $(TARGET) $(OBJS) .build-*

help:
	@echo "make           Debug build  (ASan + UBSan, -Og -g3)"
	@echo "make release   Release build (-O3, LTO, -DNDEBUG)"
	@echo "make clean     Remove build artifacts"
	@echo "Override per call: make BUILD=release"

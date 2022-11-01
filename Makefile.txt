APPS = main

# Where are the dependencies included?
SRC = src
INCLUDE = include
OBJ = obj

# Add all dependencies to the main application
deps := lib

# Get targets and dependencies from different folders
VPATH=$(SRC):$(INCLUDE):$(OBJ)

# Set the C++ compiler and flags
CC=gcc
CFLAGS= -O2 -g -Wall

# Automatically create object files needed to compile the
# main application
deps-objs := $(patsubst %,%.o,$(deps))
deps-objs-paths := $(patsubst %,$(OBJ)/%.o,$(deps))
final-targets := $(patsubst %,%.out,$(APPS))

# Set verbose flag `make V=1` to print each command
# that `make` executes.
ifeq ($(V),1)
	Q =
	msg =

else
	Q = @
	msg = @printf '  %-8s %s\n' \
	      "$(1)"                \
              "$(if $(2), $(2))";
endif

.PHONY: all
all: $(final-targets)

.PHONY: clean
clean:
	$(call msg,CLEAN)
	$(Q)rm -rf $(APPS) $(APPS).out $(APPS).out.dSYM/ $(deps-objs-paths)

$(deps-objs): %.o: %.cc %.h
	$(call msg,OBJ,$@)
	$(Q)$(CC) $(CFLAGS) -c $< -I$(INCLUDE) -o $(OBJ)/$@

$(final-targets): %.out: %.cc %.h $(deps-objs)
	$(call msg,BINARY,$@)
	$(Q)$(CC) $(CFLAGS) $< -I$(INCLUDE) $(deps-objs-paths) -o $@
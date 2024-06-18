EXE_NAME = ../bin/Linux-G13-Driver
SRC_DIR  = cpp
OBJ_DIR  = obj
OBJS     = \
$(SRC_DIR)/G13.o \
$(SRC_DIR)/G13Action.o \
$(SRC_DIR)/Macro.o \
$(SRC_DIR)/MacroAction.o \
$(SRC_DIR)/Main.o \
$(SRC_DIR)/Output.o \
$(SRC_DIR)/PassThroughAction.o
CC       = g++
FLAGS    = -L /lib64 -l pthread
LIBS     = -lusb-1.0

# Default (=first) target to clean and build the executable.
default: clean target
	echo "Created file $(EXE_NAME)"

# Target to build the actual executable target.
target: $(EXE_NAME)

# Target to remove all generated files.
clean:
	rm -f $(OBJS) $(EXE_NAME)

# Target rule	to compile Compile C++ files to object files.
.cpp.o:
	$(CC) $(FLAGS) -c $< -o $@

# Executable rule to build executable from the compiled objects.
$(EXE_NAME): $(OBJS)
	$(CC) $(OBJS) -o $(EXE_NAME) $(FLAGS) $(LIBS)


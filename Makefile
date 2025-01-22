# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -std=c11

# Source files
SRCS = main.c

# Object files directory
OBJDIR = ./out

# Object files
OBJS = $(patsubst %.c,$(OBJDIR)/%.o,$(SRCS))

# Executable name
TARGET = $(OBJDIR)/crcbmp

# Default target
all: $(TARGET)

# Link object files to create the executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# Compile source files to object files
$(OBJDIR)/%.o: %.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Create the output directory if it doesn't exist
$(OBJDIR):
	mkdir -p $(OBJDIR)

# Run the executable
run: $(TARGET)
	$(TARGET)

# Clean up build files
clean:
	rm -f $(OBJS) $(TARGET)

# Phony targets
.PHONY: all clean run
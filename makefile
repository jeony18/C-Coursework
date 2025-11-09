# Compiler
CC = clang

# Compiler flags
CFLAGS = -Wall -Wextra -std=c11

# Source files
SRCS = main.c graphics.c

# Object files
OBJS = $(SRCS:.c=.o)

# Executable name
TARGET = out

# Default rule
all: $(TARGET)

# Link step
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

# Compile step
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up
clean:
	rm -f $(OBJS) $(TARGET)
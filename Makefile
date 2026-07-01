# Define the compiler to use
CC = gcc

# Define any compile-time flags (e.g., -Wall turns on warnings, -g adds debugging info)
CFLAGS = -Wall -g
SOURCEDIR = src
BUILDDIR = build


# Define the target executable name
TARGET = $(BUILDDIR)/svm

# The 'all' target is the default when you run 'make'
all: dir $(TARGET)

dir:
	mkdir -p $(BUILDDIR)

SRCS := $(wildcard $(SOURCEDIR)/*.c)
OBJS := $(patsubst $(SOURCEDIR)/%.c, $(BUILDDIR)/%.o, $(SRCS))

# Rule to link object files into the final executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $^

# Rule to compile main.c into main.o
$(BUILDDIR)/%.o: $(SOURCEDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Rule to clean up build files
clean:
	rm -f $(BUILDDIR)/*.o $(TARGET)

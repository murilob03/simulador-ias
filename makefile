# Define compiler
CC=gcc

# Define flags
CFLAGS=-I.

# Define the executable file name
TARGET=ias

# Define source files
SOURCES=src/main.c src/ias.c src/memory.c src/conversor.c

# Default rule to build the program
all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET)

# Rule for cleaning up the project
clean:
	rm -f $(TARGET)

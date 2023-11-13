# Define the compiler
CC=gcc

# Define the compiler flags
CFLAGS=-Wall -Wextra -std=c99 -pedantic -g

# Define the target executable name
TARGET=dnote

all: $(TARGET)

$(TARGET): dnote.c
	$(CC) $(CFLAGS) dnote.c -o $(TARGET)

clean:
	rm -f $(TARGET)


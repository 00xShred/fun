CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS = -lm
TARGET = nn
SOURCES = main.c neural_network.c dataset.c
OBJECTS = $(SOURCES:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	rm -f $(OBJECTS) $(TARGET)

run: $(TARGET)
	./$(TARGET) -e 5000 -l 0.1 -h 16 -b 64 -m 0.9 -d sine -n 1000 -v

run-xor:
	./$(TARGET) -e 5000 -l 0.1 -h 8 -b 32 -m 0.9 -d xor -ha relu -oa sigmoid -v

run-circle:
	./$(TARGET) -e 5000 -l 0.1 -h 16 -b 64 -m 0.9 -d circle -ha relu -oa sigmoid -v

test: $(TARGET)
	./$(TARGET) -e 10 -d xor -g -seed 42
	./$(TARGET) -e 50 -d sine -seed 42
	./$(TARGET) -e 50 -d circle -seed 42

.PHONY: all clean run run-xor run-circle test

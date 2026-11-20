CC = cc
CFLAGS = -Wall -Wextra -O2

SRC = $(wildcard *.c)
BIN = $(SRC:.c=)

all: $(BIN)

%: %.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(BIN)

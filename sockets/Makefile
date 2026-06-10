CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS = -lssl -lcrypto

all: bin/http_client bin/http_server bin/https_client

bin:
	mkdir -p bin

bin/http_client: http_client.c | bin
	$(CC) $(CFLAGS) http_client.c -o bin/http_client

bin/http_server: http_server.c | bin
	$(CC) $(CFLAGS) http_server.c -o bin/http_server

bin/https_client: https_client.c | bin
	$(CC) $(CFLAGS) https_client.c -o bin/https_client $(LDFLAGS)

clean:
	rm -rf bin

.PHONY: all clean

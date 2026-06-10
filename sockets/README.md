# sockets

A progressive set of C socket programming examples (HTTP and HTTPS).

## Build

```sh
make
```

## Run

### HTTP Server & Client
```sh
./bin/http_server
# Then visit http://localhost:8001 or run:
./bin/http_client localhost
```

### HTTPS Client (requires OpenSSL)
```sh
./bin/https_client google.com
```

/* https_client.c */
/* Compile with: gcc https_client.c -o bin/https_client -lssl -lcrypto */

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

/* OpenSSL headers for encryption */
#include <openssl/err.h>
#include <openssl/ssl.h>

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: %s <hostname>\n", argv[0]);
        return 1;
    }

    char* hostname = argv[1];
    int port = 443; /* default HTTPS port */

    /* initialize OpenSSL libraries */
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();

    /* create a new SSL context */
    const SSL_METHOD* method = TLS_client_method();
    SSL_CTX* ctx = SSL_CTX_new(method);

    if (ctx == NULL) {
        ERR_print_errors_fp(stderr);
        return 1;
    }

    /* standard TCP connection setup */
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    struct hostent* server = gethostbyname(hostname);

    if (server == NULL) {
        fprintf(stderr, "No such host\n");
        return 1;
    }

    struct sockaddr_in remote_address;
    remote_address.sin_family = AF_INET;
    remote_address.sin_port = htons(port);
    memcpy(&remote_address.sin_addr, server->h_addr, server->h_length);

    if (connect(client_socket, (struct sockaddr*)&remote_address, sizeof(remote_address)) != 0) {
        perror("Could not connect to TCP socket");
        return 1;
    }

    printf("TCP connection established...\n");

    /* create an SSL structure and attach it to the socket */
    SSL* ssl = SSL_new(ctx);
    SSL_set_fd(ssl, client_socket);

    /* SNI: set the hostname for the handshake (required by many modern servers) */
    SSL_set_tlsext_host_name(ssl, hostname);

    /* perform the SSL/TLS handshake */
    if (SSL_connect(ssl) != 1) {
        fprintf(stderr, "SSL Handshake failed:\n");
        ERR_print_errors_fp(stderr);
        return 1;
    }

    printf("SSL Handshake successful! Cipher: %s\n", SSL_get_cipher(ssl));

    /* build the HTTP request */
    char request[2048];
    sprintf(request, "GET / HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", hostname);

    printf("Sending request:\n%s\n", request);

    /* use SSL_write instead of send */
    SSL_write(ssl, request, strlen(request));

    char response[4096];
    int bytes;

    printf(" --- Response ---\n");

    /* use SSL_read instead of recv */
    while ((bytes = SSL_read(ssl, response, sizeof(response) - 1)) > 0) {
        response[bytes] = 0;
        printf("%s", response);
    }

    /* clean up SSL resources */
    SSL_shutdown(ssl);
    SSL_free(ssl);

    close(client_socket);
    SSL_CTX_free(ctx);

    return 0;
}

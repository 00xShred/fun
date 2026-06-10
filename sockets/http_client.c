/* http_client.c */
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>

#include <arpa/inet.h>
#include <netinet/in.h>

/* needed for gethostbyname */
#include <netdb.h>

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: %s <hostname>\n", argv[0]);
        return 1;
    }

    /* take the hostname (e.g., google.com) from command line arguments */
    char* address;
    address = argv[1];

    /* create a TCP socket */
    int client_socket;
    client_socket = socket(AF_INET, SOCK_STREAM, 0);

    /* define remote address configuration */
    struct sockaddr_in remote_address;
    remote_address.sin_family = AF_INET;
    remote_address.sin_port = htons(80); /* HTTP uses port 80 */

    /* resolve the hostname to an IP address */
    struct hostent* server = gethostbyname(address);

    if (server == NULL) {
        fprintf(stderr, "Error: No such host\n");
        return 1;
    }

    /* copy the resolved IP address to our address structure */
    memcpy(&remote_address.sin_addr, server->h_addr, server->h_length);

    /* connect to the server */
    connect(client_socket, (struct sockaddr*)&remote_address, sizeof(remote_address));

    /* minimal HTTP GET request */
    char request[] = "GET / HTTP/1.1\r\n\r\n";
    char response[4096];

    /* send the request */
    send(client_socket, request, sizeof(request), 0);

    /* receive the response */
    recv(client_socket, &response, sizeof(response), 0);

    /* print the full response (headers + body) */
    printf("response from the server: %s\n", response);

    close(client_socket);

    return 0;
}

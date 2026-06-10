/* http_server.c */
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/types.h>

#include <netinet/in.h>
#include <unistd.h>

int main() {

    /* open the file we want to serve */
    FILE* html_data;
    html_data = fopen("index.html", "r");

    /* read the file content into a buffer */
    char response_data[1024];
    fread(response_data, sizeof(char), 1024, html_data);
    fclose(html_data);

    /* create a standard HTTP header */
    /* \r\n\r\n indicates the end of the header and start of body */
    char http_header[2048] = "HTTP/1.1 200 OK\r\n\r\n";

    /* add the file content to the response */
    strcat(http_header, response_data);

    /* create the server socket */
    int server_socket;
    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    /* define the address */
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(8001);
    server_address.sin_addr.s_addr = INADDR_ANY;

    /* bind and listen */
    bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address));
    listen(server_socket, 5);

    int client_socket;

    /* infinite loop to keep the server running */
    while (1) {
        /* accept incoming connection */
        client_socket = accept(server_socket, NULL, NULL);

        /* send the HTTP response */
        send(client_socket, http_header, sizeof(http_header), 0);

        /* close the connection to this client */
        close(client_socket);
    }

    return 0;
}

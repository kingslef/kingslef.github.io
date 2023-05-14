#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

int main(int argc, const char *argv[])
{
    int ret;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return -1;
    }

    /* Initialize a socket with type as IPv4 (AF_INET) using stream connections
       (i.e. TCP) (SOCK_STREAM). */
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("socket failed");
        return -1;
    }

    struct sockaddr_in sock_addr = {0};
    /* Set correct internet family to sockaddr. */
    sock_addr.sin_family = AF_INET;
    /* Use INADDR_ANY to say that we are interested in all IP addresses assigned
       to us. */
    sock_addr.sin_addr.s_addr = INADDR_ANY;
    /* Set correct port to sockaddr. It needs to be in network byte order. */
    sock_addr.sin_port = htons(atoi(argv[1]));

    /* Try bind to our socket. */
    ret = bind(sock_fd, (const struct sockaddr *)&sock_addr, sizeof(sock_addr));
    if (ret < 0) {
        perror("bind failed");
        close(sock_fd);
        return -1;
    }

    /* Create a buffer for received stuff. */
    char buf[1024];

    /* Inform that we are interested in new connections on this socket. */
    ret = listen(sock_fd, 1);
    if (ret < 0) {
        perror("listen failed");
        close(sock_fd);
        return -1;
    }

    int client_sock;

    printf("waiting for clients...\n");
    while ((client_sock = accept(sock_fd, NULL, NULL)) >= 0) {
        if (client_sock < 0) {
            perror("accept failed");
            close(sock_fd);
            return -1;
        }

        printf("new client found!\n");

        /* Receive. */
        ssize_t received_bytes = recv(client_sock, buf, sizeof(buf), 0);
        if (received_bytes < 0) {
            perror("recv failed");
            close(client_sock);
            close(sock_fd);
            return -1;
        } else if (received_bytes == 0) {
            fprintf(stderr, "client closed the connection\n");
        } else {
            /* Someting was received, print it. */
            printf("received: '");
            for (ssize_t i = 0; i < received_bytes; i++) {
                putchar(buf[i]);
            }
            printf("'\n");
        }

        close(client_sock);
        printf("waiting for clients...\n");
    }

    if (client_sock < 0) {
        perror("accept failed");
        close(sock_fd);
        return -1;
    }

    /* Close the socket. */
    close(sock_fd);

    return 0;
}

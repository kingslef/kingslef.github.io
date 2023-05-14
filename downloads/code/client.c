#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main(int argc, const char *argv[])
{
    int ret;

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <ip_address_to_connect> <port>\n", argv[0]);
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

    /* Transform given IPv4 address to network format. */
    ret = inet_pton(AF_INET, argv[1], &sock_addr.sin_addr);
    if (ret != 1) {
        fprintf(stderr, "inet_pton failed: %d\n", ret);
        return -1;
    }

    /* Set correct internet family to sockaddr. */
    sock_addr.sin_family = AF_INET;
    /* Set correct port to sockaddr. It needs to be in network byte order. */
    sock_addr.sin_port = htons(atoi(argv[2]));

    /* Try to connect given IP address to a socket. */
    ret = connect(sock_fd, (const struct sockaddr *)&sock_addr, sizeof(sock_addr));
    if (ret < 0) {
        perror("connect failed");
        return -1;
    }

    /* Send something */
    const char buf[] = "test string";

    printf("sending..\n");
    ssize_t sent_bytes = send(sock_fd, buf, sizeof(buf), 0);
    if (sent_bytes < 0) {
        perror("send failed");
        return -1;
    }

    /* Close the socket. */
    close(sock_fd);

    return 0;
}

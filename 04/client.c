#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <memory.h>

int main() {
    int client_socket;

    size_t buf_size = 2048;
    char buffer[buf_size];

    struct sockaddr_in server_addr;
    socklen_t addr_size;

    // Internet domain, Stream socket, Default protocol (TCP)
    client_socket = socket(PF_INET, SOCK_STREAM, 0);

    // Configure settings of the server address struct
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(7891);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Set all bits of the padding field to 0
    memset(server_addr.sin_zero, '\0', sizeof server_addr.sin_zero);

    // Connect the socket to the server using the address struct
    addr_size = sizeof server_addr;
    connect(client_socket, (struct sockaddr *) &server_addr, addr_size);

    // Read the message from the server into the buffer
    recv(client_socket, buffer, buf_size, 0);

    // Print the received message
    printf("%s\n", buffer);

    return 0;
}
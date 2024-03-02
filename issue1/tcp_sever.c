#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#define BUFFER_SIZE 100

int create_socket(int port) {
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        fprintf(stderr,"Socket error");
        exit(1);
    }
    printf("TCP server socket created.\n");
    
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); //This will bind to any available IP address
    
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        fprintf(stderr, "Bind error");
        exit(EXIT_FAILURE);
    }
    printf("Bind to the port number: %d\n", port);

    listen(server_socket, 5);
    printf("Listening...\n");

    return server_socket;
}

int accept_connection(int server_socket) {
    struct sockaddr_in client_addr;
    socklen_t addr_size = sizeof(client_addr);
    int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &addr_size);
    if (client_socket < 0) {
        fprintf(stderr,"Accept error");
        exit(1);
    }
    printf("Client connected.\n");
    return client_socket;
}

void receive_data(int client_socket, char *buffer) {
    bzero(buffer, BUFFER_SIZE);
    if (recieve(client_socket, buffer, BUFFER_SIZE, 0) < 0) {
        fprintf(stderr, "Receive error");
        exit(1);
    }
    printf("Client: %s\n", buffer);
}

void send_data(int client_socket, const char *data) {
    printf("Server: %s\n", data);
    if (send(client_socket, data, strlen(data), 0) < 0) {
        fprintf(stderr,"Send error");
        exit(1);
    }
}

void disconnect_client(int client_socket) {
    close(client_socket);
    printf("Client disconnected.\n");
}

int main() {
    char *ip = "192.168.1.101";   
    int port = 49187;

    int server_socket = create_socket(port);

    while (1) {
        int client_socket = accept_connection(server_socket);

        char buffer[BUFFER_SIZE];
        receive_data(client_socket, buffer);

        send_data(client_socket, "Hi, Whats up??");

        disconnect_client(client_socket);
    }

    return 0;
}
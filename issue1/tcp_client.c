#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#define BUFFER_SIZE 100 


int create_socket(){ //error showing here 
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        fprintf(stderr,"Failed to create a socket(%d)\n", client_socket);
        exit(1);
    }
    printf("TCP server socket created.\n");
    return client_socket;
}

void connect_to_server(int client_socket, const char *ip, int port) {
    struct sockaddr_in addr;
    memset(&addr, '\0', sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip);

    if (connect(client_socket, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        fprintf(stderr, "Failed to connect to server. (%d)\n", client_socket);
        exit(1);
    }
    printf("Connected to the server.\n");
}

void send_data(int client_socket, const char *data) {
    printf("Client: %s\n", data);
    if (send(client_socket, data, strlen(data), 0) < 0) {
        fprintf(stderr, "Send error\n");
        exit(1);
    }
}

void receive_data(int client_socket, char *buffer, int buffer_size) {
    bzero(buffer, buffer_size);                              //buffer is used to store the data that is being                                                                     
    if (recv(client_socket, buffer, buffer_size, 0) < 0) {            //sent or recieved.
        fprintf(stderr, "Receive error\n");                             
        exit(1);
    }                                                                               
    printf("Server: %s\n", buffer);
}

void disconnect_from_server(int client_socket) {
    close(client_socket);
    printf("Disconnected from the server.\n");
}

int main() {
    char *ip = "192.168.1.101";
    int port = 49187;

    int client_socket = create_socket();
    connect_to_server(client_socket, ip, port);

    char buffer[BUFFER_SIZE];
    send_data(client_socket, "Hello,This is the client speaking.");
    receive_data(client_socket, buffer, BUFFER_SIZE);

    disconnect_from_server(client_socket);

    return 0;
}


//udp client server program that prints simple messages
//A program that can send commands and recieve telemetry.


#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SERVER_PORT 3000
#define CLIENT_PORT 4000
#define CMD_PACKET_LEN 512

void udp_tlm_thread_fn() {
    int client_fd;
    struct sockaddr_in server_addr;
    char telemetry_data[CMD_PACKET_LEN] = "Telemetry data";

    client_fd = socket(AF_INET, SOCK_DGRAM, 0);

   
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Change to RPi address
    server_addr.sin_port = htons(SERVER_PORT);

 
    sendto(client_fd, telemetry_data, strlen(telemetry_data), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));

    close(client_fd);
}

void udp_cmd_thread_fn() {
    int server_fd;
    struct sockaddr_in server_addr, client_addr;
    char command_data[CMD_PACKET_LEN];

    // Create socket for server
    server_fd = socket(AF_INET, SOCK_DGRAM, 0);

    // Bind the socket
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(CLIENT_PORT);
    bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));

    printf("Server listening on port %d\n", CLIENT_PORT);

    // Receive and print messages from clients
    socklen_t client_len = sizeof(client_addr);
    recvfrom(server_fd, command_data, CMD_PACKET_LEN, 0, (struct sockaddr *)&client_addr, &client_len);
    printf("Received command from client: %s\n", command_data);
   
 
    close(server_fd);
}

int main() {
    udp_tlm_thread_fn();
    udp_cmd_thread_fn();
    return 0;
}
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

int main(){
    int client_fd;
    struct sockaddr_in server_addr, client_addr;
    char command_data[CMD_PACKET_LEN] = "Test Command";
    char telemetry_data[CMD_PACKET_LEN];

    // Create socket for client
    client_fd = socket(AF_INET, SOCK_DGRAM, 0);

    // Prepare server address structure
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(SERVER_PORT);

    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    client_addr.sin_port = htons(CLIENT_PORT);


    // Send command message to server
    sendto(client_fd, command_data, strlen(command_data), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
    printf("Command sent to server: %s\n", command_data);

    // Receive telemetry data from server
    socklen_t server_len = sizeof(server_addr);
    ssize_t bytes = recvfrom(client_fd, telemetry_data, CMD_PACKET_LEN, 0, (struct sockaddr *)&server_addr, &server_len);
    if (bytes < 0) {
        perror("recvfrom error");
        return 1;
    }

    printf("Received telemetry from server: %s\n", telemetry_data);

    // Close socket
    close(client_fd);

    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>

#define SERVER_PORT 3000
#define CLIENT_PORT 4000
#define CMD_PACKET_LEN 512

void init_udp_sockets(int *server_fd, struct sockaddr_in *server_addr) {
    // Create socket for server
    *server_fd = socket(AF_INET, SOCK_DGRAM, 0);

    // Bind the socket
    memset(server_addr, 0, sizeof(*server_addr));
    server_addr->sin_family = AF_INET;
    server_addr->sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr->sin_port = htons(CLIENT_PORT);
    bind(*server_fd, (struct sockaddr *)server_addr, sizeof(*server_addr));
}

void* udp_tlm_thread_fn(void* arg) {
    int client_fd;
    struct sockaddr_in server_addr;
    char telemetry_data[CMD_PACKET_LEN] = "Telemetry data";

    client_fd = socket(AF_INET, SOCK_DGRAM, 0);

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(SERVER_PORT);

    // Send telemetry data
    sendto(client_fd, telemetry_data, strlen(telemetry_data), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));

    close(client_fd);
    sleep_ms(1000);
}

void* udp_cmd_thread_fn(void* arg) {
    int server_fd;
    struct sockaddr_in server_addr, client_addr;
    char command_data[CMD_PACKET_LEN];

    // Initialize UDP sockets
    init_udp_sockets(&server_fd, &server_addr);

    printf("Server listening on port %d\n", CLIENT_PORT);

    // Receive and print messages from clients
    socklen_t client_len = sizeof(client_addr);
    recvfrom(server_fd, command_data, CMD_PACKET_LEN, 0, (struct sockaddr *)&client_addr, &client_len);
    printf("Received command from client: %s\n", command_data);

    close(server_fd);
}

void* udpcomms_thread(void* arg) {
    // Additional code for UDP communications thread
    return NULL;
}

void sleep_ms(int milliseconds) {
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;
    while (nanosleep(&ts, &ts) && errno == EINTR);
}


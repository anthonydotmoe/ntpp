#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#define BUFFER_SIZE 48
#define NTP_TIMESTAMP_DELTA 2208988800ull

void initialize_socket_address(struct sockaddr_in *address, int port) {
	address->sin_family = AF_INET;
	address->sin_addr.s_addr = htonl(INADDR_ANY);
	address->sin_port = htons(port);
}

void apply_offset(uint8_t *buffer, int offset_seconds) {
	uint32_t seconds;

	// Extract the receive timestamp seconds (bytes 32-35)
	memcpy(&seconds, &buffer[32], sizeof(uint32_t));
	seconds = ntohl(seconds);

	// Apply the offset
	seconds += offset_seconds;

	// Put the modified seconds back into the buffer
	seconds = htonl(seconds);
	memcpy(&buffer[32], &seconds, sizeof(uint32_t));

	// Extract the transmit timestamp seconds (bytes 40-43)
	memcpy(&seconds, &buffer[40], sizeof(uint32_t));
	seconds = ntohl(seconds);

	// Apply the offset
	seconds += offset_seconds;

	// Put the modified seconds back into the buffer
	seconds = htonl(seconds);
	memcpy(&buffer[40], &seconds, sizeof(uint32_t));
}

int main(int argc, char *argv[]) {
	if (argc != 5) {
		printf("Usage: %s <listen_port> <upstream_ntp_server_ip> <upstream_ntp_server_port> <offset_seconds>\n", argv[0]);
		return 1;
	}

	unsigned int listen_port = atoi(argv[1]);
	char *upstream_ntp_server_ip = argv[2];
	unsigned int upstream_ntp_server_port = atoi(argv[3]);
	int offset_seconds = atoi(argv[4]);

	int server_socket, client_socket;
	struct sockaddr_in server_addr, client_addr, upstream_addr;
	socklen_t client_addr_size = sizeof(client_addr);
	uint8_t buffer[BUFFER_SIZE];

	// Initialize the server and upstream socket addresses
	initialize_socket_address(&server_addr, listen_port);
	initialize_socket_address(&upstream_addr, upstream_ntp_server_port);

	// Convert the upstream NTP server IP address
	if (inet_pton(AF_INET, upstream_ntp_server_ip, &upstream_addr.sin_addr) <= 0) {
		perror("Invalid upstream NTP server IP address");
		return 1;
	}

	// Create the server socket
	server_socket = socket(AF_INET, SOCK_DGRAM, 0);
	if (server_socket < 0) {
		perror("Failed to create server socket");
		return 1;
	}

	// Bind the server socket
	if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
		perror("Failed to bind server socket");
		close(server_socket);
		return 1;
	}

	while (1) {
		// Receive the NTP query
		if (recvfrom(server_socket, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &client_addr_size) < 0) {
			perror("Failed to receive NTP query");
			continue;
		}

		// Create the client socket
		client_socket = socket(AF_INET, SOCK_DGRAM, 0);
		if (client_socket < 0) {
			perror("Failed to create client socket");
			continue;
		}

		// Send the NTP query to the upstream NTP server
		if (sendto(client_socket, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&upstream_addr, sizeof(upstream_addr)) < 0) {
			perror("Failed to send NTP query to upstream NTP server");
			close(client_socket);
			continue;
		}

		// Receive the NTP response from the upstream NTP server
		if (recvfrom(client_socket, buffer, BUFFER_SIZE, 0, NULL, NULL) < 0) {
			perror("Failed to receive NTP response from upstream NTP server");
			close(client_socket);
			continue;
		}

		// Apply the desired offset
		apply_offset(buffer, offset_seconds);

		// Send the modified NTP response back to the original client
		if (sendto(server_socket, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, client_addr_size) < 0) {
			perror("Failed to send modified NTP response");
		}

		// Close the client socket
		close(client_socket);
	}

	// Close the server socket
	close(server_socket);
	return 0;
}


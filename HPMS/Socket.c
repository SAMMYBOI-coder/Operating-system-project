/*
 * HPMS Socket Demo - Remote Patient Data Access
 * Scenario: Doctor workstation connects to central HPMS server
 * 
 * Compile: gcc socket_demo.c -o socket_demo
 * Run: ./socket_demo
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    pid_t pid;
    
    printf("================================================================================\n");
    printf("   POSIX SOCKET DEMONSTRATION - Remote Access\n");
    printf("================================================================================\n");
    printf("Scenario: Doctor workstation → HPMS central server communication\n");
    printf("Security: TCP socket (would use TLS in production)\n\n");
    
    pid = fork();
    
    if (pid == 0) {
        // Child process - CLIENT (Doctor workstation)
        sleep(1); // Let server start first
        
        int sock;
        struct sockaddr_in server_addr;
        char buffer[BUFFER_SIZE] = {0};
        
        printf("[Doctor Workstation] Connecting to HPMS central server...\n");
        
        // Create socket
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            perror("Client socket creation failed");
            exit(1);
        }
        
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(PORT);
        server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        
        // Connect to server
        if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
            perror("Connection failed");
            exit(1);
        }
        
        printf("[Doctor Workstation] ✓ Connected to server (127.0.0.1:%d)\n", PORT);
        printf("[Doctor Workstation] Requesting patient #1234 data...\n");
        
        // Send request
        char request[] = "GET_PATIENT_DATA:1234";
        send(sock, request, strlen(request), 0);
        
        // Receive response
        int bytes_received = recv(sock, buffer, BUFFER_SIZE, 0);
        if (bytes_received > 0) {
            printf("[Doctor Workstation] ✓ Received patient data:\n");
            printf("[Doctor Workstation]   %s\n", buffer);
        }
        
        close(sock);
        printf("[Doctor Workstation] Connection closed\n");
        
    } else {
        // Parent process - SERVER (HPMS central server)
        int server_fd, client_sock;
        struct sockaddr_in address;
        int addrlen = sizeof(address);
        char buffer[BUFFER_SIZE] = {0};
        
        printf("[HPMS Server] Starting on port %d...\n", PORT);
        
        // Create socket
        server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd == 0) {
            perror("Server socket creation failed");
            exit(1);
        }
        
        // Allow port reuse
        int opt = 1;
        setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(PORT);
        
        // Bind
        if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
            perror("Bind failed");
            exit(1);
        }
        
        // Listen
        if (listen(server_fd, 3) < 0) {
            perror("Listen failed");
            exit(1);
        }
        
        printf("[HPMS Server] ✓ Server listening on port %d\n", PORT);
        printf("[HPMS Server] Waiting for doctor connections...\n\n");
        
        // Accept connection
        client_sock = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        if (client_sock < 0) {
            perror("Accept failed");
            exit(1);
        }
        
        printf("[HPMS Server] ✓ Doctor workstation connected from %s\n", 
               inet_ntoa(address.sin_addr));
        
        // Receive request
        int bytes_received = recv(client_sock, buffer, BUFFER_SIZE, 0);
        if (bytes_received > 0) {
            printf("[HPMS Server] Request received: %s\n", buffer);
        }
        
        // Send patient data
        char patient_data[] = "PatientID=1234 | Name=REDACTED | Diagnosis=Cardiac | Status=Stable";
        send(client_sock, patient_data, strlen(patient_data), 0);
        printf("[HPMS Server] ✓ Patient data sent to doctor workstation\n\n");
        
        close(client_sock);
        close(server_fd);
        
        wait(NULL); // Wait for child to finish
        
        printf("\n================================================================================\n");
        printf("POSIX Socket Features:\n");
        printf("================================================================================\n");
        printf("✓ Bidirectional communication (request/response)\n");
        printf("✓ Network-based (supports remote doctor access)\n");
        printf("✓ Platform-independent (Linux/Windows compatible)\n");
        printf("✓ TCP reliable delivery (no data loss)\n");
        printf("⚠️  PRODUCTION: Must use TLS/SSL encryption for HIPAA compliance\n");
        printf("⚠️  PRODUCTION: Implement authentication and authorization\n");
        printf("================================================================================\n");
    }
    
    return 0;
}
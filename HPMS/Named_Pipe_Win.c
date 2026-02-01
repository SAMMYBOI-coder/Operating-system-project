/*
 * HPMS Named Pipe (FIFO) Demo - POSIX IPC
 * Scenario: Registration → Validation → Database pipeline
 * 
 * Compile: gcc pipe_posix.c -o pipe_demo
 * Run: ./pipe_demo
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h> 

#define FIFO_NAME "/tmp/hpms_registration_pipe"

int main() {
    pid_t pid;
    int fd;
    char buffer[256];
    
    printf("========================================\n");
    printf("   POSIX NAMED PIPE DEMONSTRATION\n");
    printf("========================================\n");
    printf("Scenario: Registration → Validation Pipeline\n");
    printf("Security: Filesystem permissions (0600)\n\n");
    
    // Create named pipe with secure permissions
    mkfifo(FIFO_NAME, 0600);
    
    pid = fork();
    
    if (pid == 0) {
        // Child process - Validation stage
        printf("[Validation Process] Waiting for patient data...\n");
        
        fd = open(FIFO_NAME, O_RDONLY);
        read(fd, buffer, sizeof(buffer));
        close(fd);
        
        printf("[Validation Process] ✓ Received: %s\n", buffer);
        printf("[Validation Process] Validating patient information...\n");
        sleep(1);
        printf("[Validation Process] ✓ Validation complete - Forwarding to database\n");
        
    } else {
        // Parent process - Registration entry
        sleep(1); // Ensure child is ready
        
        printf("[Registration Entry] Entering patient data...\n");
        char patient_data[] = "PatientID=101 | Name=REDACTED | Emergency=HIGH | Age=45";
        
        fd = open(FIFO_NAME, O_WRONLY);
        write(fd, patient_data, strlen(patient_data) + 1);
        close(fd);
        
        printf("[Registration Entry] ✓ Data sent to validation pipeline\n\n");
        
        wait(NULL); // Wait for child
        
        printf("\n========================================\n");
        printf("POSIX Named Pipe Features:\n");
        printf("✓ Unidirectional data flow (Registration → Validation)\n");
        printf("✓ Built-in buffering (handles speed differences)\n");
        printf("✓ Sequential processing (validation after entry)\n");
        printf("✓ Filesystem-based security (chmod 0600)\n");
        printf("========================================\n");
    }
    
    // Cleanup
    unlink(FIFO_NAME);
    
    return 0;
}
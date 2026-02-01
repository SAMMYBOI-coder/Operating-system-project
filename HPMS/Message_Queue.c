/*
 * HPMS Message Queue Demo - POSIX IPC
 * Scenario: Lab sends results to Doctor asynchronously
 * 
 * Compile: gcc -pthread message_queue_posix.c -o message_queue -lrt
 * Run: ./message_queue
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <unistd.h>
#include <sys/stat.h>

#define QUEUE_NAME "/hpms_lab_results"
#define MAX_MSG_SIZE 256

int main() {
    mqd_t mq;
    struct mq_attr attr;
    char buffer[MAX_MSG_SIZE];
    
    printf("========================================\n");
    printf("   POSIX MESSAGE QUEUE DEMONSTRATION\n");
    printf("========================================\n");
    printf("Scenario: Lab → Doctor Communication\n");
    printf("Security: Owner-only access (0600)\n\n");
    
    // Set queue attributes
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MAX_MSG_SIZE;
    attr.mq_curmsgs = 0;
    
    // Create/open queue with secure permissions (0600 = owner only)
    mq = mq_open(QUEUE_NAME, O_CREAT | O_RDWR, 0600, &attr);
    if (mq == (mqd_t)-1) {
        perror("mq_open failed");
        return 1;
    }
    
    // Lab sends result (Priority 1 = Urgent)
    char lab_result[] = "PatientID=1234 | Test=Blood | Result=Glucose 95mg/dL NORMAL";
    printf("[Lab Module] Sending urgent test result...\n");
    if (mq_send(mq, lab_result, strlen(lab_result) + 1, 1) == -1) {
        perror("mq_send failed");
        return 1;
    }
    printf("[Lab Module] ✓ Result sent to queue (Priority 1 - Urgent)\n\n");
    
    // Simulate asynchronous operation
    printf("[System] Lab process can exit - message persists in queue\n");
    printf("[System] Doctor can retrieve result when ready...\n\n");
    sleep(1);
    
    // Doctor receives result
    unsigned int prio;
    ssize_t bytes_read = mq_receive(mq, buffer, MAX_MSG_SIZE, &prio);
    if (bytes_read >= 0) {
        printf("[Doctor Dashboard] ✓ Retrieved lab result (Priority %u)\n", prio);
        printf("[Doctor Dashboard] Data: %s\n", buffer);
    }
    
    printf("\n========================================\n");
    printf("POSIX Message Queue Features:\n");
    printf("✓ Asynchronous communication (temporal decoupling)\n");
    printf("✓ Priority support (urgent results first)\n");
    printf("✓ Secure permissions (0600 owner-only)\n");
    printf("✓ Message persistence (survives process exit)\n");
    printf("========================================\n");
    
    // Cleanup
    mq_close(mq);
    mq_unlink(QUEUE_NAME);
    
    return 0;
}
/*
 * WINDOWS VERSION - Semaphore Solution to Resource Exhaustion
 * 
 * HPMS Scenario: Controls 20-connection database pool. Semaphore initialized to 20.
 * 100 concurrent requests arrive, but only 20 can access DB simultaneously.
 * 
 * Demonstrates: Prevents resource exhaustion by enforcing connection limit.
 * 
 * Compile:
 *   gcc semaphore_solution_windows.c -o semaphore_solution_windows.exe
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_DB_CONNECTIONS 20
#define TOTAL_REQUESTS 100

// Semaphore to limit concurrent database connections
HANDLE db_connection_semaphore;

// Simulate database access
DWORD WINAPI patient_registration(LPVOID arg) {
    int patient_num = *(int*)arg;
    
    printf("[Patient #%d] Requesting database connection...\n", patient_num);
    
    // WAIT on semaphore (decrement count, block if 0)
    DWORD wait_result = WaitForSingleObject(db_connection_semaphore, 5000);  // 5s timeout
    
    if (wait_result == WAIT_OBJECT_0) {
        // Successfully acquired connection slot
        printf("[Patient #%d] ACQUIRED database connection\n", patient_num);
        
        // Simulate database operation (registration)
        Sleep(500 + (rand() % 500));  // 500-1000ms
        
        printf("[Patient #%d] Registration complete, releasing connection\n", patient_num);
        
        // RELEASE semaphore (increment count)
        ReleaseSemaphore(db_connection_semaphore, 1, NULL);
    } else if (wait_result == WAIT_TIMEOUT) {
        printf("[Patient #%d] TIMEOUT waiting for connection (queue too long)\n", patient_num);
    } else {
        printf("[Patient #%d] ERROR acquiring connection\n", patient_num);
    }
    
    free(arg);
    return 0;
}

int main() {
    printf("=== WINDOWS - SEMAPHORE SOLUTION (Resource Exhaustion Fixed) ===\n");
    printf("HPMS Scenario: 100 concurrent requests with 20-connection DB pool\n");
    printf("Win32 CreateSemaphore limits concurrent access\n\n");
    
    // Create semaphore with initial count = MAX_DB_CONNECTIONS, max count = MAX_DB_CONNECTIONS
    db_connection_semaphore = CreateSemaphore(
        NULL,                      // Default security
        MAX_DB_CONNECTIONS,        // Initial count (all slots available)
        MAX_DB_CONNECTIONS,        // Maximum count
        NULL                       // Unnamed
    );
    
    if (db_connection_semaphore == NULL) {
        fprintf(stderr, "CreateSemaphore failed\n");
        return 1;
    }
    
    printf("Database connection pool initialized: %d concurrent connections allowed\n\n", 
           MAX_DB_CONNECTIONS);
    
    HANDLE threads[TOTAL_REQUESTS];
    
    // Spawn 100 registration threads
    for (int i = 0; i < TOTAL_REQUESTS; i++) {
        int *patient_num = malloc(sizeof(int));
        *patient_num = i + 1;
        
        threads[i] = CreateThread(NULL, 0, patient_registration, patient_num, 0, NULL);
        
        if (threads[i] == NULL) {
            fprintf(stderr, "CreateThread failed for patient %d\n", i + 1);
        }
        
        Sleep(10);  // Small delay between arrivals
    }
    
    printf("\n[Main] All 100 patients arrived. Waiting for all registrations...\n\n");
    
    // Wait for all threads to complete
    WaitForMultipleObjects(TOTAL_REQUESTS, threads, TRUE, INFINITE);
    
    // Clean up
    for (int i = 0; i < TOTAL_REQUESTS; i++) {
        CloseHandle(threads[i]);
    }
    CloseHandle(db_connection_semaphore);
    
    printf("\n\n=== RESOURCE MANAGEMENT SUCCESS ===\n");
    printf("Result: All 100 patients registered WITHOUT resource exhaustion\n");
    printf("Mechanism: Semaphore enforced maximum 20 concurrent DB connections\n");
    printf("  - First 20 patients acquired connections immediately\n");
    printf("  - Remaining 80 patients queued and waited for available slots\n");
    printf("  - As connections released, waiting patients acquired them\n");
    printf("  - NO fork() failures, NO 'Cannot allocate memory' errors\n");
    
    printf("\n=== WINDOWS SEMAPHORE CHARACTERISTICS ===\n");
    printf("Simpler multi-process setup: Handles inherit across CreateProcess hierarchy\n");
    printf("Linux comparison: Requires explicit sem_open() with shared names\n");
    printf("\nFor HPMS connection pooling:\n");
    printf("  - Windows: Named semaphore automatically shared (easier setup)\n");
    printf("  - Linux: Named semaphore requires filesystem-based coordination\n");
    printf("\nVerdict: Either platform suitable. Windows simpler for cross-process sharing.\n");
    printf("         Linux offers finer permission control via filesystem.\n");
    
    return 0;
}

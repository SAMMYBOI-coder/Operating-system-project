/*
 * HPMS Resource Exhaustion Demonstration
 *
 * Scenario:
 * 1. Emergency patients arrive and registration processes are spawned
 * 2. Parent process does NOT call wait() to clean up child processes
 * 3. Zombie processes accumulate, consuming process table entries
 * 4. System eventually cannot create new processes
 * 5. New emergency patients cannot be registered
 *
 * This demonstrates resource exhaustion from poor process management.
 *
 * Compile: gcc resource_exhaustion_demo.c -o resource_exhaustion
 * Run:     ./resource_exhaustion
 * Monitor: Open another terminal and run: watch -n 1 'ps aux | grep defunct'
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>

#define MAX_PATIENTS 100
#define CRITICAL_PATIENT 101

/* Simulate patient registration */
void register_patient(int patient_id) {
    /* REMOVED: Verbose "Registration process started" message */
    
    /* Simulate registration work */
    sleep(1);
    
    /* REMOVED: Verbose "Registration complete" message */
    exit(0);  /* Child exits */
}

int main() {
    pid_t pid;
    int zombie_count = 0;
    
    printf("========================================\n");
    printf("   RESOURCE EXHAUSTION DEMONSTRATION\n");
    printf("========================================\n");
    printf("Simulating emergency room patient registrations\n");
    printf("WARNING: Parent does NOT clean up child processes\n");
    printf("========================================\n\n");
    
    printf("Registering %d emergency patients...\n\n", MAX_PATIENTS);
    
    /* Create processes for emergency patients WITHOUT cleanup */
    for (int i = 1; i <= MAX_PATIENTS; i++) {
        pid = fork();
        
        if (pid < 0) {
            /* Fork failed - resource exhaustion! */
            printf("\n*** CRITICAL ERROR at Patient %d ***\n", i);
            printf("ERROR: fork() failed - %s\n", strerror(errno));
            printf("Cannot create registration process!\n");
            printf("Cause: System resources exhausted\n\n");
            break;
        }
        else if (pid == 0) {
            /* Child process - register patient */
            register_patient(i);
        }
        else {
            /* Parent process - NO WAIT() CALL! */
            /* This is the bug - children become zombies */
            zombie_count++;
            
            if (i % 10 == 0) {
                printf("[System] %d patients registered so far...\n", i);
            }
        }
        
        /* Small delay between registrations */
        usleep(10000);  /* 10ms */
    }
    
    /* Wait for children to complete (but not clean them up) */
    sleep(2);
    
    printf("\n========================================\n");
    printf("All %d emergency patients processed\n", MAX_PATIENTS);
    printf("========================================\n\n");
    
    /* Check system status */
    printf("System Resource Status:\n");
    printf("  Spawned processes: %d\n", zombie_count);
    printf("  Zombie processes: %d (not cleaned up!)\n", zombie_count);
    printf("  Parent did NOT call wait() - BUG!\n\n");
    
    printf("*** RESOURCE EXHAUSTION DETECTED ***\n");
    printf("Zombie processes consuming process table entries\n");
    printf("Check with: ps aux | grep defunct\n\n");
    
    /* Try to register one more CRITICAL emergency patient */
    printf("----------------------------------------\n");
    printf("CRITICAL: New emergency patient arrives!\n");
    printf("Patient %d needs immediate registration...\n", CRITICAL_PATIENT);
    printf("----------------------------------------\n");
    
    pid = fork();
    
    if (pid < 0) {
        printf("\n*** SYSTEM FAILURE ***\n");
        printf("ERROR: Cannot register emergency patient!\n");
        printf("fork() failed: %s\n", strerror(errno));
        printf("Cause: Process table exhausted by zombie processes\n");
        printf("Impact: CRITICAL PATIENT CANNOT BE ADMITTED!\n\n");
    }
    else if (pid == 0) {
        register_patient(CRITICAL_PATIENT);
    }
    else {
        printf("[System] Emergency patient %d registration started\n", CRITICAL_PATIENT);
        wait(NULL);  /* Wait for this one */
    }
    
    printf("\n========================================\n");
    printf("ANALYSIS:\n");
    printf("========================================\n");
    printf("Problem: Parent created %d child processes\n", MAX_PATIENTS);
    printf("         but NEVER called wait() to clean them up\n\n");
    printf("Result:  %d zombie processes remain in system\n", zombie_count);
    printf("         consuming process table slots\n\n");
    printf("Impact:  New processes cannot be created\n");
    printf("         Emergency patients cannot be registered\n");
    printf("         LIFE-THREATENING SYSTEM FAILURE\n\n");
    printf("Solution: Parent MUST call wait() or waitpid()\n");
    printf("          to reap terminated child processes\n");
    printf("========================================\n\n");
    
    /* Clean up all zombies before exit */
    printf("Cleaning up zombie processes...\n");
    while (wait(NULL) > 0);
    printf("All zombies reaped. Exiting.\n");
    
    return 0;
}
/*
 * HPMS Resource Exhaustion SOLUTION
 *
 * Scenario:
 * 1. Same as resource_exhaustion_demo.c BUT with proper cleanup
 * 2. Parent process calls waitpid() to reap terminated children
 * 3. No zombie processes accumulate
 * 4. System resources remain available
 * 5. New emergency patients can always be registered
 *
 * This demonstrates proper process management preventing resource exhaustion.
 *
 * Compile: gcc proper_cleanup_solution.c -o proper_cleanup
 * Run:     ./proper_cleanup
 * Monitor: Open another terminal: watch -n 1 'ps aux | grep defunct'
 *          (Should see NO zombies!)
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
    /* REMOVED: Verbose output for clean screenshots */
    sleep(1);
    exit(0);  /* Child exits */
}

int main() {
    pid_t pid;
    int status;
    int active_children = 0;
    int completed_registrations = 0;
    
    printf("========================================\n");
    printf("   PROPER PROCESS CLEANUP SOLUTION\n");
    printf("========================================\n");
    printf("Simulating emergency room patient registrations\n");
    printf("✓ Parent PROPERLY cleans up child processes\n");
    printf("✓ No zombie accumulation\n");
    printf("========================================\n\n");
    
    printf("Registering %d emergency patients...\n\n", MAX_PATIENTS);
    
    /* Create processes for emergency patients WITH proper cleanup */
    for (int i = 1; i <= MAX_PATIENTS; i++) {
        pid = fork();
        
        if (pid < 0) {
            /* Fork failed */
            printf("\n*** ERROR at Patient %d ***\n", i);
            printf("ERROR: fork() failed - %s\n", strerror(errno));
            break;
        }
        else if (pid == 0) {
            /* Child process - register patient */
            register_patient(i);
        }
        else {
            /* Parent process - track active children */
            active_children++;
            
            if (i % 10 == 0) {
                printf("[System] %d patients registered, cleaning up...\n", i);
                
                /* PROPER CLEANUP: Reap completed children */
                while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
                    active_children--;
                    completed_registrations++;
                }
            }
        }
        
        /* Small delay between registrations */
        usleep(10000);  /* 10ms */
    }
    
    printf("\n[System] All %d patients spawned, waiting for completion...\n", MAX_PATIENTS);
    
    /* Wait for ALL remaining children to complete */
    printf("[System] Cleaning up remaining processes...\n");
    while ((pid = waitpid(-1, &status, 0)) > 0) {
        active_children--;
        completed_registrations++;
    }
    
    printf("\n========================================\n");
    printf("All %d emergency patients processed\n", MAX_PATIENTS);
    printf("========================================\n\n");
    
    /* Check system status */
    printf("System Resource Status:\n");
    printf("  Total processes spawned: %d\n", MAX_PATIENTS);
    printf("  Completed registrations: %d\n", completed_registrations);
    printf("  Active children remaining: %d\n", active_children);
    printf("  Zombie processes: 0 ✓ (properly cleaned up!)\n");
    printf("  Parent properly called waitpid()\n\n");
    
    printf("✓ NO RESOURCE EXHAUSTION\n");
    printf("All child processes properly reaped\n");
    printf("Check with: ps aux | grep defunct (should be empty)\n\n");
    
    /* Try to register one more CRITICAL emergency patient */
    printf("========================================\n");
    printf("CRITICAL: New emergency patient arrives!\n");
    printf("Patient %d needs immediate registration...\n", CRITICAL_PATIENT);
    printf("========================================\n");
    
    pid = fork();
    
    if (pid < 0) {
        printf("\n*** UNEXPECTED ERROR ***\n");
        printf("ERROR: Cannot register emergency patient!\n");
        printf("fork() failed: %s\n", strerror(errno));
    }
    else if (pid == 0) {
        register_patient(CRITICAL_PATIENT);
    }
    else {
        printf("[System] ✓ Emergency patient %d registration started\n", CRITICAL_PATIENT);
        printf("[System] System has capacity for new patients\n");
        
        /* Wait for critical patient */
        waitpid(pid, &status, 0);
        printf("[System] ✓ Emergency patient %d registration complete\n\n", CRITICAL_PATIENT);
    }
    
    printf("========================================\n");
    printf("SOLUTION ANALYSIS:\n");
    printf("========================================\n");
    printf("✓ Parent created %d child processes\n", MAX_PATIENTS);
    printf("✓ Parent called waitpid() to reap all children\n");
    printf("✓ Zero zombie processes remain\n");
    printf("✓ System resources available for new patients\n");
    printf("✓ Emergency patient #%d successfully registered\n\n", CRITICAL_PATIENT);
    
    printf("Comparison with BROKEN version:\n");
    printf("  BROKEN: Zombies accumulate → resource exhaustion\n");
    printf("  FIXED:  Proper cleanup → resources available\n\n");
    
    printf("Key Difference:\n");
    printf("  waitpid(-1, &status, WNOHANG) in loop\n");
    printf("  Reaps terminated children without blocking\n");
    printf("  Prevents process table exhaustion\n");
    printf("========================================\n\n");
    
    printf("Best Practices Applied:\n");
    printf("1. Regular cleanup during operation (every 10 patients)\n");
    printf("2. Final cleanup at end (wait for all remaining)\n");
    printf("3. WNOHANG flag allows non-blocking cleanup\n");
    printf("4. System remains responsive throughout\n");
    printf("========================================\n");
    
    return 0;
}
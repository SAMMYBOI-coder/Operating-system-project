/*
 * WINDOWS VERSION - Resource Exhaustion Problem
 * 
 * HPMS Scenario: 100 emergency registrations spawn processes. Without proper
 * cleanup, zombie/orphaned processes accumulate until system resources are exhausted.
 * 
 * Demonstrates: CreateProcess without proper handle cleanup leads to handle
 * table exhaustion preventing new patient registration.
 * 
 * Compile:
 *   gcc resource_exhaustion_problem_windows.c -o resource_exhaustion_problem_windows.exe
 * 
 * Note: Windows auto-reaps terminated processes more aggressively than Linux,
 * but handle leaks still cause resource exhaustion.
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_REGISTRATIONS 100

// Simulate a quick registration process
void spawn_registration_process(int patient_num) {
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    
    // Spawn a simple process that exits immediately (simulates registration completing)
    // Using cmd.exe /c exit as a dummy process
    char cmd[256];
    sprintf(cmd, "cmd.exe /c exit");
    
    if (CreateProcess(NULL, cmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        printf("[Parent] Patient #%d registration process created (PID: %lu)\n", 
               patient_num, pi.dwProcessId);
        
        // CRITICAL BUG: NOT closing handles!
        // In real HPMS, parent should call:
        //   CloseHandle(pi.hProcess);
        //   CloseHandle(pi.hThread);
        // Without closing, handles accumulate even after process terminates
        
        // Let process complete
        WaitForSingleObject(pi.hProcess, 1000);  // Wait up to 1 second
        
        // Still not closing handles - they leak!
    } else {
        printf("[ERROR] Failed to create registration process for Patient #%d\n", patient_num);
        printf("        Error code: %lu\n", GetLastError());
        printf("        Likely cause: Handle table exhaustion\n");
    }
}

int main() {
    printf("=== WINDOWS - RESOURCE EXHAUSTION PROBLEM ===\n");
    printf("HPMS Scenario: 100 emergency registrations without handle cleanup\n");
    printf("Windows: Handle leaks accumulate (even with auto-reaping)\n\n");
    
    printf("Starting emergency registration simulation...\n\n");
    
    // Spawn 100 registration processes WITHOUT cleaning up handles
    for (int i = 1; i <= MAX_REGISTRATIONS + 1; i++) {
        if (i <= MAX_REGISTRATIONS) {
            printf("\n--- Emergency Patient #%d arrives ---\n", i);
            spawn_registration_process(i);
            Sleep(10);  // Small delay between registrations
        } else {
            // Patient #101 arrives - should fail due to resource exhaustion
            printf("\n\n=== CRITICAL EMERGENCY: Patient #101 arrives ===\n");
            printf("Attempting registration...\n");
            spawn_registration_process(i);
        }
    }
    
    printf("\n\n=== RESOURCE EXHAUSTION ANALYSIS ===\n");
    printf("Problem: CreateProcess without CloseHandle() leaks handles\n");
    printf("Impact: Eventually CreateProcess fails even though processes terminated\n");
    printf("Windows Difference: Auto-reaps processes but handles still leak\n");
    printf("Linux Difference: Zombie processes accumulate in process table\n");
    printf("\nIn production HPMS:\n");
    printf("  - Windows: Must close BOTH hProcess and hThread handles\n");
    printf("  - Linux: Must call wait() or waitpid() to reap zombies\n");
    
    return 0;
}

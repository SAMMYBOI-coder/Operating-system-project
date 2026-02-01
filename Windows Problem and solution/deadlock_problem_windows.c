/*
 * WINDOWS VERSION - Deadlock Problem
 * 
 * HPMS Scenario: Doctor needs Patient Record + Medication Inventory locks.
 * Pharmacy needs Medication Inventory + Patient Record locks.
 * Inconsistent lock ordering causes circular wait = DEADLOCK.
 * 
 * Demonstrates: Both threads frozen indefinitely. System requires manual restart.
 * 
 * Compile:
 *   gcc deadlock_problem_windows.c -o deadlock_problem_windows.exe
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

// Two shared resources (represented by mutexes)
HANDLE patient_record_mutex;
HANDLE medication_inventory_mutex;

// Doctor thread - acquires locks in order: Patient Record FIRST, then Medication
DWORD WINAPI doctor_thread(LPVOID arg) {
    printf("[Doctor] Attempting to access patient record at t=0s...\n");
    
    // Acquire Patient Record lock FIRST
    WaitForSingleObject(patient_record_mutex, INFINITE);
    printf("[Doctor] ACQUIRED Patient Record lock at t=0s\n");
    
    // Simulate reading patient data
    Sleep(1000);  // 1 second processing
    printf("[Doctor] Reading patient diagnosis... (holding Patient Record lock)\n");
    
    // Now needs Medication Inventory lock
    printf("[Doctor] Now need Medication Inventory lock at t=1s...\n");
    printf("[Doctor] WAITING for Medication Inventory lock...\n");
    
    WaitForSingleObject(medication_inventory_mutex, INFINITE);
    // ^^^ DEADLOCK: This will never return because Pharmacy holds it
    
    printf("[Doctor] ACQUIRED Medication Inventory lock (WILL NEVER PRINT)\n");
    
    // Would release locks here (never reached)
    ReleaseMutex(medication_inventory_mutex);
    ReleaseMutex(patient_record_mutex);
    
    return 0;
}

// Pharmacy thread - acquires locks in OPPOSITE order: Medication FIRST, then Patient Record
DWORD WINAPI pharmacy_thread(LPVOID arg) {
    Sleep(500);  // Start 0.5s after doctor
    
    printf("[Pharmacy] Attempting to access medication inventory at t=0.5s...\n");
    
    // Acquire Medication Inventory lock FIRST (opposite order from doctor!)
    WaitForSingleObject(medication_inventory_mutex, INFINITE);
    printf("[Pharmacy] ACQUIRED Medication Inventory lock at t=0.5s\n");
    
    // Simulate checking medication stock
    Sleep(1000);  // 1 second processing
    printf("[Pharmacy] Checking medication stock... (holding Medication Inventory lock)\n");
    
    // Now needs Patient Record lock
    printf("[Pharmacy] Now need Patient Record lock at t=1.5s...\n");
    printf("[Pharmacy] WAITING for Patient Record lock...\n");
    
    WaitForSingleObject(patient_record_mutex, INFINITE);
    // ^^^ DEADLOCK: This will never return because Doctor holds it
    
    printf("[Pharmacy] ACQUIRED Patient Record lock (WILL NEVER PRINT)\n");
    
    // Would release locks here (never reached)
    ReleaseMutex(patient_record_mutex);
    ReleaseMutex(medication_inventory_mutex);
    
    return 0;
}

int main() {
    printf("=== WINDOWS - DEADLOCK PROBLEM ===\n");
    printf("HPMS Scenario: Doctor and Pharmacy acquire locks in inconsistent order\n");
    printf("Using Win32 mutexes WITHOUT lock ordering protocol\n\n");
    
    // Create mutexes (unnamed, not inheritable)
    patient_record_mutex = CreateMutex(NULL, FALSE, NULL);
    medication_inventory_mutex = CreateMutex(NULL, FALSE, NULL);
    
    if (patient_record_mutex == NULL || medication_inventory_mutex == NULL) {
        fprintf(stderr, "CreateMutex failed\n");
        return 1;
    }
    
    HANDLE threads[2];
    
    // Create both threads
    threads[0] = CreateThread(NULL, 0, doctor_thread, NULL, 0, NULL);
    threads[1] = CreateThread(NULL, 0, pharmacy_thread, NULL, 0, NULL);
    
    if (threads[0] == NULL || threads[1] == NULL) {
        fprintf(stderr, "CreateThread failed\n");
        return 1;
    }
    
    // Wait for threads... but they will DEADLOCK
    printf("\n[Main] Waiting for threads to complete...\n");
    printf("[Main] (They will never complete - circular wait detected)\n\n");
    
    // Wait with timeout to demonstrate deadlock
    DWORD wait_result = WaitForMultipleObjects(2, threads, TRUE, 5000);  // 5 second timeout
    
    if (wait_result == WAIT_TIMEOUT) {
        printf("\n\n=== DEADLOCK DETECTED ===\n");
        printf("Both threads frozen for 5+ seconds. Circular dependency:\n");
        printf("  - Doctor holds Patient Record, waits for Medication Inventory\n");
        printf("  - Pharmacy holds Medication Inventory, waits for Patient Record\n");
        printf("\nWindows Detection:\n");
        printf("  - Use Task Manager → Details → Right-click → Analyze Wait Chain\n");
        printf("  - Shows which process/thread is blocking which\n");
        printf("\nRecovery: Manual termination required (Ctrl+C or Task Manager)\n");
        printf("\n*** HPMS IMPACT: No patient records accessible. System restart needed. ***\n");
        
        // Terminate threads forcefully
        TerminateThread(threads[0], 1);
        TerminateThread(threads[1], 1);
    }
    
    CloseHandle(threads[0]);
    CloseHandle(threads[1]);
    CloseHandle(patient_record_mutex);
    CloseHandle(medication_inventory_mutex);
    
    return 0;
}

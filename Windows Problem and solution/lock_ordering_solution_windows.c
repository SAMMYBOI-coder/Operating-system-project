/*
 * WINDOWS VERSION - Lock Ordering Solution to Deadlock
 * 
 * HPMS Scenario: Doctor and Pharmacy BOTH acquire locks in SAME order:
 * ALWAYS Patient Record FIRST, then Medication Inventory SECOND.
 * 
 * Demonstrates: Consistent lock ordering eliminates circular wait condition.
 * 
 * Compile:
 *   gcc lock_ordering_solution_windows.c -o lock_ordering_solution_windows.exe
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

// Two shared resources (represented by mutexes)
HANDLE patient_record_mutex;
HANDLE medication_inventory_mutex;

// Doctor thread - acquires locks in CONSISTENT order
DWORD WINAPI doctor_thread(LPVOID arg) {
    printf("[Doctor] Starting patient treatment workflow at t=0s...\n");
    
    // STEP 1: Acquire Patient Record lock FIRST (enforced order)
    printf("[Doctor] Acquiring Patient Record lock (Step 1)...\n");
    DWORD result = WaitForSingleObject(patient_record_mutex, 5000);  // 5s timeout
    
    if (result == WAIT_TIMEOUT) {
        printf("[Doctor] TIMEOUT acquiring Patient Record lock (deadlock avoided)\n");
        return 1;
    }
    printf("[Doctor] ACQUIRED Patient Record lock at t=0s\n");
    
    // Simulate reading patient data
    Sleep(1000);
    printf("[Doctor] Reading patient diagnosis...\n");
    
    // STEP 2: Acquire Medication Inventory lock SECOND (enforced order)
    printf("[Doctor] Acquiring Medication Inventory lock (Step 2)...\n");
    result = WaitForSingleObject(medication_inventory_mutex, 5000);  // 5s timeout
    
    if (result == WAIT_TIMEOUT) {
        printf("[Doctor] TIMEOUT acquiring Medication lock\n");
        ReleaseMutex(patient_record_mutex);  // Release first lock before failing
        return 1;
    }
    printf("[Doctor] ACQUIRED Medication Inventory lock at t=1s\n");
    
    // Now holding both locks - can safely prescribe
    printf("[Doctor] Prescribing medication (holding both locks safely)\n");
    Sleep(500);
    
    // Release locks in REVERSE order (best practice)
    ReleaseMutex(medication_inventory_mutex);
    printf("[Doctor] Released Medication Inventory lock\n");
    
    ReleaseMutex(patient_record_mutex);
    printf("[Doctor] Released Patient Record lock\n");
    printf("[Doctor] Treatment workflow complete!\n");
    
    return 0;
}

// Pharmacy thread - acquires locks in SAME CONSISTENT order as doctor
DWORD WINAPI pharmacy_thread(LPVOID arg) {
    Sleep(500);  // Start 0.5s after doctor
    
    printf("[Pharmacy] Starting medication verification workflow at t=0.5s...\n");
    
    // STEP 1: Acquire Patient Record lock FIRST (SAME order as doctor!)
    printf("[Pharmacy] Acquiring Patient Record lock (Step 1 - SAME order as doctor)...\n");
    DWORD result = WaitForSingleObject(patient_record_mutex, 5000);  // 5s timeout
    
    if (result == WAIT_TIMEOUT) {
        printf("[Pharmacy] TIMEOUT acquiring Patient Record lock\n");
        return 1;
    }
    printf("[Pharmacy] ACQUIRED Patient Record lock (waited for doctor to release)\n");
    
    // Simulate checking patient allergies
    Sleep(1000);
    printf("[Pharmacy] Checking patient allergies...\n");
    
    // STEP 2: Acquire Medication Inventory lock SECOND (SAME order as doctor!)
    printf("[Pharmacy] Acquiring Medication Inventory lock (Step 2 - SAME order)...\n");
    result = WaitForSingleObject(medication_inventory_mutex, 5000);  // 5s timeout
    
    if (result == WAIT_TIMEOUT) {
        printf("[Pharmacy] TIMEOUT acquiring Medication lock\n");
        ReleaseMutex(patient_record_mutex);
        return 1;
    }
    printf("[Pharmacy] ACQUIRED Medication Inventory lock\n");
    
    // Now holding both locks - can safely dispense
    printf("[Pharmacy] Dispensing medication (holding both locks safely)\n");
    Sleep(500);
    
    // Release locks in REVERSE order
    ReleaseMutex(medication_inventory_mutex);
    printf("[Pharmacy] Released Medication Inventory lock\n");
    
    ReleaseMutex(patient_record_mutex);
    printf("[Pharmacy] Released Patient Record lock\n");
    printf("[Pharmacy] Verification workflow complete!\n");
    
    return 0;
}

int main() {
    printf("=== WINDOWS - LOCK ORDERING SOLUTION (Deadlock Prevented) ===\n");
    printf("HPMS Scenario: Doctor and Pharmacy acquire locks in CONSISTENT order\n");
    printf("Protocol: ALWAYS Patient Record FIRST, Medication Inventory SECOND\n");
    printf("Using Win32 mutexes with 5-second timeout safety net\n\n");
    
    // Create mutexes
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
    
    printf("\n[Main] Both workflows started. Monitoring for deadlock...\n\n");
    
    // Wait for both threads to complete
    DWORD wait_result = WaitForMultipleObjects(2, threads, TRUE, 10000);  // 10s timeout
    
    if (wait_result == WAIT_TIMEOUT) {
        printf("\n[Main] DEADLOCK DETECTED (shouldn't happen with lock ordering)\n");
        TerminateThread(threads[0], 1);
        TerminateThread(threads[1], 1);
    } else {
        printf("\n\n=== DEADLOCK PREVENTION SUCCESS ===\n");
        printf("Result: Both doctor and pharmacy completed workflows WITHOUT deadlock\n");
        printf("Mechanism: Consistent lock ordering eliminates circular wait\n");
        printf("  - Doctor: Patient Record (t=0s) → Medication (t=1s)\n");
        printf("  - Pharmacy: Patient Record (waits) → Medication (after doctor)\n");
        printf("  - NO circular dependency possible\n");
    }
    
    CloseHandle(threads[0]);
    CloseHandle(threads[1]);
    CloseHandle(patient_record_mutex);
    CloseHandle(medication_inventory_mutex);
    
    printf("\n=== WINDOWS DEADLOCK RECOVERY TOOLS ===\n");
    printf("Development: Visual Studio Concurrency Visualizer detects lock issues\n");
    printf("Production: Task Manager → Analyze Wait Chain shows blocking processes\n");
    printf("  - Right-click frozen process → 'Analyze Wait Chain'\n");
    printf("  - Shows: Process A waits for Process B waits for Process A\n");
    printf("  - Advantage over Linux: Runtime detection without instrumentation\n");
    printf("\nLinux comparison:\n");
    printf("  - Development: Valgrind Helgrind/DRD (better static analysis)\n");
    printf("  - Production: Manual timeout detection only\n");
    
    printf("\n=== HPMS BEST PRACTICES ===\n");
    printf("1. Document lock hierarchy in code comments:\n");
    printf("   /* LOCK ORDER: Patient Record → Medication → Pharmacy → Lab */\n");
    printf("2. Use mandatory timeouts (5 seconds) on ALL lock acquisitions\n");
    printf("3. Release locks in REVERSE order of acquisition\n");
    printf("4. Code review enforcement: Flag any inconsistent lock ordering\n");
    printf("5. Testing: Use Wait Chain (Windows) or Valgrind (Linux) during QA\n");
    
    printf("\nVerdict: Lock ordering mandatory on BOTH platforms.\n");
    printf("         Use Valgrind during Linux development.\n");
    printf("         Use Wait Chain for Windows production troubleshooting.\n");
    
    return 0;
}

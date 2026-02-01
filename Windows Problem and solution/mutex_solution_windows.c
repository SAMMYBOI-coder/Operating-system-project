/*
 * WINDOWS VERSION - Mutex Solution to Race Condition
 * 
 * HPMS Scenario: Doctor prescribes medication while nurse updates allergies
 * WITH mutex synchronization preventing TOCTOU vulnerability.
 * 
 * Demonstrates: Serialized access prevents stale reads. Doctor either sees
 * "No allergies" OR "Penicillin allergy" - never both.
 * 
 * Compile:
 *   gcc mutex_solution_windows.c -o mutex_solution_windows.exe
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

// Shared patient record (PROTECTED by mutex)
typedef struct {
    int patient_id;
    char allergy[64];
    char prescription[64];
} PatientRecord;

PatientRecord patient = {1234, "No allergies", "None"};

// Mutex for protecting patient record access
HANDLE patient_record_mutex;

// Doctor thread - acquires mutex before reading/prescribing
DWORD WINAPI doctor_thread(LPVOID arg) {
    printf("[Doctor] Requesting patient record lock at t=0ms...\n");
    
    // ACQUIRE MUTEX before accessing shared data
    WaitForSingleObject(patient_record_mutex, INFINITE);
    printf("[Doctor] ACQUIRED mutex at t=0ms\n");
    
    // SAFE: Read allergy status while holding lock
    char allergy_status[64];
    strcpy(allergy_status, patient.allergy);
    printf("[Doctor] Read: '%s' at t=0ms (protected read)\n", allergy_status);
    
    // Simulate processing delay
    Sleep(100);  // 100ms delay
    
    // SAFE: Prescribe based on current data (nurse blocked from updating)
    if (strcmp(allergy_status, "No allergies") == 0) {
        strcpy(patient.prescription, "Penicillin 500mg");
        printf("[Doctor] Prescribed: Penicillin at t=100ms (safe - nurse blocked)\n");
    } else {
        strcpy(patient.prescription, "Alternative medication");
        printf("[Doctor] Prescribed: Alternative (allergy detected)\n");
    }
    
    // RELEASE MUTEX
    ReleaseMutex(patient_record_mutex);
    printf("[Doctor] Released mutex at t=100ms\n");
    
    return 0;
}

// Nurse thread - blocked until doctor releases mutex
DWORD WINAPI nurse_thread(LPVOID arg) {
    Sleep(50);  // Attempt to update 50ms after doctor starts
    
    printf("[Nurse] Requesting patient record lock at t=50ms...\n");
    
    // BLOCKED: Doctor holds mutex, nurse must wait
    printf("[Nurse] WAITING for mutex (doctor holds it)...\n");
    
    WaitForSingleObject(patient_record_mutex, INFINITE);
    printf("[Nurse] ACQUIRED mutex at t=100ms+ (doctor released it)\n");
    
    // SAFE: Update allergy AFTER doctor completed prescription
    strcpy(patient.allergy, "Penicillin allergy");
    printf("[Nurse] Updated allergy to: '%s'\n", patient.allergy);
    
    ReleaseMutex(patient_record_mutex);
    printf("[Nurse] Released mutex\n");
    
    return 0;
}

int main() {
    printf("=== WINDOWS - MUTEX SOLUTION (Race Condition Fixed) ===\n");
    printf("HPMS Scenario: Doctor and nurse access patient record WITH mutex\n");
    printf("Win32 CreateMutex + WaitForSingleObject serializes access\n\n");
    
    // Create unnamed mutex (not inheritable, initially not owned)
    patient_record_mutex = CreateMutex(NULL, FALSE, NULL);
    
    if (patient_record_mutex == NULL) {
        fprintf(stderr, "CreateMutex failed\n");
        return 1;
    }
    
    HANDLE threads[2];
    
    // Create doctor and nurse threads WITH synchronization
    threads[0] = CreateThread(NULL, 0, doctor_thread, NULL, 0, NULL);
    threads[1] = CreateThread(NULL, 0, nurse_thread, NULL, 0, NULL);
    
    if (threads[0] == NULL || threads[1] == NULL) {
        fprintf(stderr, "CreateThread failed\n");
        return 1;
    }
    
    // Wait for both threads to complete
    WaitForMultipleObjects(2, threads, TRUE, INFINITE);
    
    CloseHandle(threads[0]);
    CloseHandle(threads[1]);
    CloseHandle(patient_record_mutex);
    
    printf("\n=== FINAL STATE (CONSISTENT) ===\n");
    printf("Patient #%d:\n", patient.patient_id);
    printf("  Allergy: %s\n", patient.allergy);
    printf("  Prescription: %s\n", patient.prescription);
    printf("\n*** SUCCESS: Race condition prevented by mutex serialization ***\n");
    printf("Doctor completed prescription BEFORE nurse updated allergy.\n");
    printf("If order reversed, doctor would see allergy and prescribe alternative.\n");
    printf("Either outcome is SAFE - no TOCTOU vulnerability.\n");
    
    printf("\n=== WINDOWS MUTEX CHARACTERISTICS ===\n");
    printf("Performance: Kernel-level (slower than Linux futex under high load)\n");
    printf("Advantage: Auto-detects abandoned mutex if thread crashes\n");
    printf("  - If doctor thread crashes while holding mutex,\n");
    printf("    WaitForSingleObject returns WAIT_ABANDONED\n");
    printf("  - Nurse can detect and recover (Linux POSIX mutex stays locked forever)\n");
    printf("\nVerdict: Windows mutex better for frontend (crash recovery)\n");
    printf("         Linux futex better for backend (performance ~40%% faster)\n");
    
    return 0;
}

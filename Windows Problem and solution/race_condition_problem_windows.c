/*
 * WINDOWS VERSION - Race Condition Problem
 * 
 * HPMS Scenario: Doctor prescribes medication while nurse updates allergies
 * for same patient WITHOUT synchronization.
 * 
 * Demonstrates: Time-Of-Check-Time-Of-Use (TOCTOU) vulnerability causing
 * fatal prescription to be recorded despite allergy being documented.
 * 
 * Compile on Windows with MinGW or MSVC:
 *   gcc race_condition_problem_windows.c -o race_condition_problem_windows.exe
 * 
 * Or with MSVC:
 *   cl race_condition_problem_windows.c
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

// Shared patient record (NO synchronization protection)
typedef struct {
    int patient_id;
    char allergy[64];
    char prescription[64];
} PatientRecord;

PatientRecord patient = {1234, "No allergies", "None"};

// Doctor thread - reads allergy, prescribes medication
DWORD WINAPI doctor_thread(LPVOID arg) {
    printf("[Doctor] Checking allergies for Patient #%d at t=0ms...\n", patient.patient_id);
    
    // VULNERABLE: Read allergy status (Time-Of-Check)
    char allergy_status[64];
    strcpy(allergy_status, patient.allergy);
    printf("[Doctor] Read: '%s' at t=0ms\n", allergy_status);
    
    // Simulate processing delay (reading charts, consulting guidelines)
    Sleep(100);  // 100ms delay
    
    // VULNERABLE: Prescribe based on stale read (Time-Of-Use)
    if (strcmp(allergy_status, "No allergies") == 0) {
        strcpy(patient.prescription, "Penicillin 500mg");
        printf("[Doctor] Prescribed: Penicillin at t=100ms (based on stale allergy read)\n");
    }
    
    return 0;
}

// Nurse thread - updates patient allergies
DWORD WINAPI nurse_thread(LPVOID arg) {
    // Nurse updates allergy information DURING doctor's processing
    Sleep(50);  // Start 50ms after doctor
    
    printf("[Nurse] Updating allergy information at t=50ms...\n");
    strcpy(patient.allergy, "Penicillin allergy");
    printf("[Nurse] Updated allergy to: '%s' at t=50ms\n", patient.allergy);
    
    return 0;
}

int main() {
    printf("=== WINDOWS - RACE CONDITION PROBLEM ===\n");
    printf("HPMS Scenario: Doctor prescribing while nurse updates allergies\n");
    printf("WITHOUT mutex protection (Win32 threads)\n\n");
    
    HANDLE threads[2];
    
    // Create doctor and nurse threads without synchronization
    threads[0] = CreateThread(NULL, 0, doctor_thread, NULL, 0, NULL);
    threads[1] = CreateThread(NULL, 0, nurse_thread, NULL, 0, NULL);
    
    if (threads[0] == NULL || threads[1] == NULL) {
        fprintf(stderr, "CreateThread failed\n");
        return 1;
    }
    
    // Wait for both threads to complete
    WaitForMultipleObjects(2, threads, TRUE, INFINITE);
    
    // Close thread handles
    CloseHandle(threads[0]);
    CloseHandle(threads[1]);
    
    printf("\n=== FINAL STATE (CORRUPTED) ===\n");
    printf("Patient #%d:\n", patient.patient_id);
    printf("  Allergy: %s\n", patient.allergy);
    printf("  Prescription: %s\n", patient.prescription);
    printf("\n*** FATAL ERROR: Penicillin prescribed despite Penicillin allergy! ***\n");
    printf("*** Race condition caused medical record corruption ***\n");
    
    return 0;
}

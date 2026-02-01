/*
 * HPMS Race Condition Demonstration (TOCTOU)
 *
 * Scenario:
 * 1. Patient record initially shows no allergies
 * 2. Doctor reads allergy information
 * 3. Nurse updates allergy information shortly after
 * 4. Doctor makes a medical decision using stale data
 *
 * This demonstrates a logical race condition caused by
 * unsynchronized access to shared state.
 *
 * Compile: gcc -pthread race_condition_demo.c -o race_condition
 * Run:     ./race_condition
 */

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

/* Shared patient record */
typedef struct {
    int patient_id;
    char allergy_info[100];
    char prescription[100];
} PatientRecord;

/* Global shared object (NO SYNCHRONIZATION) */
PatientRecord patient;

/* Doctor thread */
void* doctor_thread(void* arg) {
    char local_allergy[100];

    printf("[Doctor] Reading patient allergy information...\n");

    /* TIME OF CHECK */
    strcpy(local_allergy, patient.allergy_info);
    printf("[Doctor] Allergy recorded as: '%s'\n", local_allergy);

    /* Simulate time taken to reason / decide */
    usleep(100000);  // 100 ms

    /* TIME OF USE */
    if (strcmp(local_allergy, "None") == 0) {
        strcpy(patient.prescription, "Penicillin 500mg");
        printf("[Doctor] Prescribing Penicillin based on earlier reading\n");
    } else {
        strcpy(patient.prescription, "Alternative antibiotic");
        printf("[Doctor] Prescribing alternative due to allergy\n");
    }

    return NULL;
}

/* Nurse thread */
void* nurse_thread(void* arg) {
    /* Ensure doctor reads first */
    usleep(50000);  // 50 ms

    printf("[Nurse] Updating allergy information...\n");
    strcpy(patient.allergy_info, "Penicillin Allergy");
    printf("[Nurse] Allergy updated to: 'Penicillin Allergy'\n");

    return NULL;
}

int main() {
    pthread_t doctor, nurse;

    printf("========================================\n");
    printf("   RACE CONDITION (TOCTOU) DEMO\n");
    printf("========================================\n");
    printf("WARNING: No synchronization mechanisms used\n\n");

    /* Initial state */
    patient.patient_id = 1234;
    strcpy(patient.allergy_info, "None");
    strcpy(patient.prescription, "Not prescribed");

    printf("Initial Patient Record:\n");
    printf("  ID: %d\n", patient.patient_id);
    printf("  Allergy: %s\n", patient.allergy_info);
    printf("  Prescription: %s\n\n", patient.prescription);

    /* Start threads */
    pthread_create(&doctor, NULL, doctor_thread, NULL);
    pthread_create(&nurse, NULL, nurse_thread, NULL);

    pthread_join(doctor, NULL);
    pthread_join(nurse, NULL);

    printf("\nFinal Patient Record:\n");
    printf("  Allergy: %s\n", patient.allergy_info);
    printf("  Prescription: %s\n", patient.prescription);

    /* Detect unsafe outcome */
    if (strstr(patient.allergy_info, "Penicillin") &&
        strstr(patient.prescription, "Penicillin")) {

        printf("\n*** INCONSISTENT STATE DETECTED ***\n");
        printf("Doctor prescribed Penicillin despite Penicillin allergy\n");
        printf("Root Cause: TOCTOU race condition (stale read)\n");
    }

    return 0;
}
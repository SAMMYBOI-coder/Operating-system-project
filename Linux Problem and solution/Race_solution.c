/*
 * HPMS Race Condition SOLUTION with Mutex
 *
 * Scenario:
 * 1. Same as race_condition_demo.c BUT with mutex protection
 * 2. Doctor must acquire lock before reading patient data
 * 3. Nurse must acquire lock before updating patient data
 * 4. Mutex ensures operations are serialized (no concurrent access)
 * 5. Race condition is PREVENTED - data consistency maintained
 *
 * This demonstrates how mutex synchronization prevents TOCTOU bugs.
 *
 * Compile: gcc -pthread mutex_solution.c -o mutex_solution
 * Run:     ./mutex_solution
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

/* Global shared object WITH MUTEX PROTECTION */
PatientRecord patient;
pthread_mutex_t patient_mutex = PTHREAD_MUTEX_INITIALIZER;

/* Doctor thread WITH MUTEX */
void* doctor_thread(void* arg) {
    char local_allergy[100];

    printf("[Doctor] Attempting to read patient allergy information...\n");
    
    /* ACQUIRE MUTEX before accessing shared data */
    pthread_mutex_lock(&patient_mutex);
    printf("[Doctor] ✓ Mutex ACQUIRED - Safe to read\n");

    /* TIME OF CHECK (now protected) */
    strcpy(local_allergy, patient.allergy_info);
    printf("[Doctor] Allergy recorded as: '%s'\n", local_allergy);

    /* Simulate time taken to reason / decide */
    printf("[Doctor] Analyzing patient condition...\n");
    usleep(100000);  // 100 ms

    /* TIME OF USE (still holding lock - data cannot change) */
    if (strcmp(local_allergy, "None") == 0) {
        strcpy(patient.prescription, "Penicillin 500mg");
        printf("[Doctor] Prescribing Penicillin (safe - no allergies)\n");
    } else {
        strcpy(patient.prescription, "Alternative antibiotic");
        printf("[Doctor] Prescribing alternative due to allergy: %s\n", local_allergy);
    }
    
    /* RELEASE MUTEX after completing operation */
    pthread_mutex_unlock(&patient_mutex);
    printf("[Doctor] ✓ Mutex RELEASED\n");

    return NULL;
}

/* Nurse thread WITH MUTEX */
void* nurse_thread(void* arg) {
    /* Ensure doctor attempts to read first */
    usleep(50000);  // 50 ms

    printf("\n[Nurse] Patient reports allergy, need to update system...\n");
    printf("[Nurse] Attempting to acquire mutex...\n");
    
    /* TRY TO ACQUIRE MUTEX - will wait if doctor holds it */
    pthread_mutex_lock(&patient_mutex);
    printf("[Nurse] ✓ Mutex ACQUIRED - Safe to write\n");
    
    printf("[Nurse] Updating allergy information...\n");
    strcpy(patient.allergy_info, "Penicillin Allergy");
    printf("[Nurse] Allergy updated to: 'Penicillin Allergy'\n");
    
    /* RELEASE MUTEX */
    pthread_mutex_unlock(&patient_mutex);
    printf("[Nurse] ✓ Mutex RELEASED\n");

    return NULL;
}

int main() {
    pthread_t doctor, nurse;

    printf("========================================\n");
    printf("   MUTEX SOLUTION DEMONSTRATION\n");
    printf("========================================\n");
    printf("✓ Mutex synchronization ENABLED\n");
    printf("✓ Race condition PREVENTED\n");
    printf("========================================\n\n");

    /* Initial state */
    patient.patient_id = 1234;
    strcpy(patient.allergy_info, "None");
    strcpy(patient.prescription, "Not prescribed");

    printf("Initial Patient Record:\n");
    printf("  ID: %d\n", patient.patient_id);
    printf("  Allergy: %s\n", patient.allergy_info);
    printf("  Prescription: %s\n\n", patient.prescription);

    printf("========================================\n");
    printf("Starting Doctor and Nurse threads...\n");
    printf("========================================\n\n");

    /* Start threads */
    pthread_create(&doctor, NULL, doctor_thread, NULL);
    pthread_create(&nurse, NULL, nurse_thread, NULL);

    pthread_join(doctor, NULL);
    pthread_join(nurse, NULL);

    printf("\n========================================\n");
    printf("HOW MUTEX PREVENTED RACE CONDITION:\n");
    printf("========================================\n");
    printf("1. Doctor acquired mutex FIRST\n");
    printf("2. Doctor read allergy: 'None'\n");
    printf("3. Nurse tried to acquire mutex → BLOCKED (waiting)\n");
    printf("4. Doctor made decision & prescribed based on 'None'\n");
    printf("5. Doctor released mutex\n");
    printf("6. Nurse acquired mutex (now available)\n");
    printf("7. Nurse updated allergy to 'Penicillin Allergy'\n");
    printf("8. Nurse released mutex\n");
    printf("\nResult: Operations SERIALIZED (one after another)\n");
    printf("        Doctor's decision used CONSISTENT data\n");
    printf("        No TOCTOU bug possible!\n");
    printf("========================================\n");

    /* Cleanup */
    pthread_mutex_destroy(&patient_mutex);

    return 0;
}
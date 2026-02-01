
/*
 * HPMS Deadlock SOLUTION with Lock Ordering
 *
 * Scenario:
 * 1. Same as deadlock_demo.c BUT with consistent lock ordering
 * 2. BOTH Doctor and Pharmacy acquire locks in SAME order
 * 3. Order: Patient Record (1st) → Medication Inventory (2nd)
 * 4. No circular wait possible
 * 5. Deadlock is PREVENTED - operations complete successfully
 *
 * This demonstrates deadlock prevention through lock ordering protocol.
 *
 * Compile: gcc -pthread lock_ordering_solution.c -o lock_ordering
 * Run:     ./lock_ordering
 */

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

/* Two mutexes representing two shared resources */
pthread_mutex_t patient_record_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t medication_inventory_mutex = PTHREAD_MUTEX_INITIALIZER;

/* Shared resources */
typedef struct {
    int patient_id;
    char diagnosis[100];
    char allergy[100];
} PatientRecord;

typedef struct {
    char medication_name[50];
    int stock_available;
} MedicationInventory;

PatientRecord patient = {1234, "Not diagnosed", "None"};
MedicationInventory medication = {"Nitroglycerin", 50};

/*
 * LOCK ORDERING PROTOCOL:
 * Rule: ALWAYS acquire locks in this order:
 *   1. Patient Record Mutex (FIRST)
 *   2. Medication Inventory Mutex (SECOND)
 * 
 * NEVER acquire in reverse order!
 */

/* Doctor thread - follows lock ordering protocol */
void* doctor_thread(void* arg) {
    printf("\n[Doctor] Emergency patient #1234 requires treatment\n");
    printf("[Doctor] Following lock ordering protocol...\n");
    printf("[Doctor] Protocol: Patient Record (1st) → Medication (2nd)\n");
    
    /* STEP 1: Lock patient record (ORDER 1) */
    printf("\n[Doctor] Step 1: Acquiring Patient Record Lock...\n");
    pthread_mutex_lock(&patient_record_mutex);
    printf("[Doctor] ✓ Patient Record Lock ACQUIRED\n");
    
    /* Work with patient record */
    strcpy(patient.diagnosis, "Severe chest pain - cardiac event");
    strcpy(patient.allergy, "None");
    printf("[Doctor] Recording diagnosis: %s\n", patient.diagnosis);
    printf("[Doctor] Verified allergies: %s\n", patient.allergy);
    sleep(1);
    
    /* STEP 2: Lock medication inventory (ORDER 2) */
    printf("[Doctor] Step 2: Acquiring Medication Inventory Lock...\n");
    pthread_mutex_lock(&medication_inventory_mutex);
    printf("[Doctor] ✓ Medication Inventory Lock ACQUIRED\n");
    
    /* Work with medication inventory */
    printf("[Doctor] Checking %s stock: %d units available\n", 
           medication.medication_name, medication.stock_available);
    printf("[Doctor] Prescribing %s for cardiac emergency\n", 
           medication.medication_name);
    medication.stock_available--;
    
    /* Release locks in REVERSE order */
    pthread_mutex_unlock(&medication_inventory_mutex);
    printf("[Doctor] ✓ Medication Inventory Lock RELEASED\n");
    
    pthread_mutex_unlock(&patient_record_mutex);
    printf("[Doctor] ✓ Patient Record Lock RELEASED\n");
    
    printf("[Doctor] ✓ Treatment complete - patient stabilized\n");
    return NULL;
}

/* Pharmacy thread - ALSO follows same lock ordering protocol */
void* pharmacy_thread(void* arg) {
    /* Small delay to create potential conflict */
    usleep(500000);  /* 0.5 seconds */
    
    printf("\n[Pharmacy] Preparing to dispense emergency medication\n");
    printf("[Pharmacy] Following lock ordering protocol...\n");
    printf("[Pharmacy] Protocol: Patient Record (1st) → Medication (2nd)\n");
    
    /* STEP 1: Lock patient record (ORDER 1) - SAME AS DOCTOR! */
    printf("\n[Pharmacy] Step 1: Acquiring Patient Record Lock...\n");
    pthread_mutex_lock(&patient_record_mutex);
    printf("[Pharmacy] ✓ Patient Record Lock ACQUIRED\n");
    
    /* Verify patient information */
    printf("[Pharmacy] Verifying patient #%d allergies: %s\n", 
           patient.patient_id, patient.allergy);
    printf("[Pharmacy] Diagnosis: %s\n", patient.diagnosis);
    sleep(1);
    
    /* STEP 2: Lock medication inventory (ORDER 2) - SAME AS DOCTOR! */
    printf("[Pharmacy] Step 2: Acquiring Medication Inventory Lock...\n");
    pthread_mutex_lock(&medication_inventory_mutex);
    printf("[Pharmacy] ✓ Medication Inventory Lock ACQUIRED\n");
    
    /* Dispense medication */
    printf("[Pharmacy] Dispensing %s\n", medication.medication_name);
    printf("[Pharmacy] Updated stock: %d units remaining\n", 
           medication.stock_available);
    
    /* Release locks in REVERSE order */
    pthread_mutex_unlock(&medication_inventory_mutex);
    printf("[Pharmacy] ✓ Medication Inventory Lock RELEASED\n");
    
    pthread_mutex_unlock(&patient_record_mutex);
    printf("[Pharmacy] ✓ Patient Record Lock RELEASED\n");
    
    printf("[Pharmacy] ✓ Medication dispensed successfully\n");
    return NULL;
}

int main() {
    pthread_t doctor, pharmacy;
    
    printf("========================================\n");
    printf("   LOCK ORDERING SOLUTION\n");
    printf("========================================\n");
    printf("✓ Consistent lock ordering protocol\n");
    printf("✓ Deadlock PREVENTED\n");
    printf("========================================\n");
    
    printf("\nLock Ordering Protocol:\n");
    printf("  Rule 1: Patient Record Mutex acquired FIRST\n");
    printf("  Rule 2: Medication Inventory Mutex acquired SECOND\n");
    printf("  Rule 3: BOTH threads follow SAME order\n");
    printf("  Rule 4: Release in REVERSE order\n");
    
    printf("\nInitial State:\n");
    printf("  Patient #%d\n", patient.patient_id);
    printf("  Diagnosis: %s\n", patient.diagnosis);
    printf("  Allergy: %s\n", patient.allergy);
    printf("  Medication: %s (%d units)\n", 
           medication.medication_name, medication.stock_available);
    
    printf("\n========================================\n");
    printf("Starting Doctor and Pharmacy threads...\n");
    printf("========================================\n");
    
    /* Create both threads */
    pthread_create(&doctor, NULL, doctor_thread, NULL);
    pthread_create(&pharmacy, NULL, pharmacy_thread, NULL);
    
    /* Wait for both threads to complete */
    pthread_join(doctor, NULL);
    pthread_join(pharmacy, NULL);
    
    printf("\n========================================\n");
    printf("*** OPERATIONS COMPLETED SUCCESSFULLY ***\n");
    printf("========================================\n");
    
    printf("\nFinal State:\n");
    printf("  Patient #%d\n", patient.patient_id);
    printf("  Diagnosis: %s\n", patient.diagnosis);
    printf("  Allergy: %s\n", patient.allergy);
    printf("  Medication: %s (%d units)\n", 
           medication.medication_name, medication.stock_available);
    
    printf("\n========================================\n");
    printf("HOW LOCK ORDERING PREVENTED DEADLOCK:\n");
    printf("========================================\n");
    printf("Timeline:\n");
    printf("1. Doctor acquired Patient Record (1st) ✓\n");
    printf("2. Doctor acquired Medication (2nd) ✓\n");
    printf("3. Doctor completed work, released both locks ✓\n");
    printf("4. Pharmacy acquired Patient Record (1st) ✓\n");
    printf("5. Pharmacy acquired Medication (2nd) ✓\n");
    printf("6. Pharmacy completed work, released both locks ✓\n");
    printf("\nKey Points:\n");
    printf("✓ BOTH threads followed SAME lock order (1→2)\n");
    printf("✓ NO circular wait possible\n");
    printf("✓ Operations serialized successfully\n");
    printf("✓ Patient received treatment without delay\n");
    printf("✓ Medication dispensed correctly\n");
    
    printf("\n========================================\n");
    printf("Comparison with BROKEN version:\n");
    printf("========================================\n");
    printf("BROKEN (deadlock_demo.c):\n");
    printf("  Doctor:   Patient (1) → Medication (2)\n");
    printf("  Pharmacy: Medication (2) → Patient (1)\n");
    printf("  Result:   DEADLOCK (circular wait)\n");
    printf("\nFIXED (this program):\n");
    printf("  Doctor:   Patient (1) → Medication (2)\n");
    printf("  Pharmacy: Patient (1) → Medication (2)\n");
    printf("  Result:   NO DEADLOCK (same order)\n");
    
    printf("\n========================================\n");
    printf("Best Practices:\n");
    printf("========================================\n");
    printf("1. Define total ordering for all locks\n");
    printf("2. Document the ordering clearly\n");
    printf("3. ALL code must follow same order\n");
    printf("4. Code review to enforce compliance\n");
    printf("5. Release locks in reverse acquisition order\n");
    printf("========================================\n");
    
    /* Cleanup */
    pthread_mutex_destroy(&patient_record_mutex);
    pthread_mutex_destroy(&medication_inventory_mutex);
    
    return 0;
}

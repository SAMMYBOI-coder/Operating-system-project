/*
 * HPMS Deadlock Demonstration
 *
 * Scenario:
 * 1. Doctor needs to update patient diagnosis AND check medication availability
 * 2. Pharmacy needs to verify patient allergies AND dispense medication
 * 3. Doctor locks Patient Record, then requests Medication Inventory
 * 4. Pharmacy locks Medication Inventory, then requests Patient Record
 * 5. DEADLOCK: Both wait for each other's locks indefinitely
 *
 * This demonstrates circular wait deadlock condition.
 *
 * Compile: gcc -pthread deadlock_demo.c -o deadlock
 * Run:     ./deadlock
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

PatientRecord patient = {1234, "Not diagnosed", "Unknown"};
MedicationInventory medication = {"Nitroglycerin", 50};

/* Doctor thread - acquires locks in ORDER 1 */
void* doctor_thread(void* arg) {
    printf("\n[Doctor] Emergency patient #1234 requires treatment\n");
    printf("[Doctor] Starting diagnosis update...\n");
    
    /* STEP 1: Lock patient record */
    printf("[Doctor] Acquiring Patient Record Lock...\n");
    pthread_mutex_lock(&patient_record_mutex);
    printf("[Doctor] ✓ Patient Record Lock ACQUIRED\n");
    
    /* Simulate working with patient record */
    strcpy(patient.diagnosis, "Severe chest pain - cardiac event");
    printf("[Doctor] Recording diagnosis: %s\n", patient.diagnosis);
    sleep(1);
    
    /* STEP 2: Need to check medication availability */
    printf("[Doctor] Need to verify medication availability...\n");
    printf("[Doctor] Requesting Medication Inventory Lock...\n");
    
    /* DEADLOCK OCCURS HERE - waiting for pharmacy's lock */
    pthread_mutex_lock(&medication_inventory_mutex);
    printf("[Doctor] ✓ Medication Inventory Lock ACQUIRED\n");
    
    /* This code will NEVER execute due to deadlock */
    printf("[Doctor] Checking %s stock: %d units\n", 
           medication.medication_name, medication.stock_available);
    printf("[Doctor] Prescribing %s\n", medication.medication_name);
    
    /* Release locks */
    pthread_mutex_unlock(&medication_inventory_mutex);
    pthread_mutex_unlock(&patient_record_mutex);
    
    printf("[Doctor] Treatment complete\n");
    return NULL;
}

/* Pharmacy thread - acquires locks in ORDER 2 (OPPOSITE!) */
void* pharmacy_thread(void* arg) {
    /* Small delay to let doctor acquire first lock */
    usleep(500000);  /* 0.5 seconds */
    
    printf("\n[Pharmacy] Preparing to dispense emergency medication\n");
    
    /* STEP 1: Lock medication inventory */
    printf("[Pharmacy] Acquiring Medication Inventory Lock...\n");
    pthread_mutex_lock(&medication_inventory_mutex);
    printf("[Pharmacy] ✓ Medication Inventory Lock ACQUIRED\n");
    
    /* Simulate checking stock */
    printf("[Pharmacy] Checking %s stock: %d units available\n", 
           medication.medication_name, medication.stock_available);
    sleep(1);
    
    /* STEP 2: Need to verify patient allergies */
    printf("[Pharmacy] Need to verify patient allergy information...\n");
    printf("[Pharmacy] Requesting Patient Record Lock...\n");
    
    /* DEADLOCK OCCURS HERE - waiting for doctor's lock */
    pthread_mutex_lock(&patient_record_mutex);
    printf("[Pharmacy] ✓ Patient Record Lock ACQUIRED\n");
    
    /* This code will NEVER execute due to deadlock */
    printf("[Pharmacy] Verifying allergies: %s\n", patient.allergy);
    printf("[Pharmacy] Dispensing %s\n", medication.medication_name);
    
    /* Release locks */
    pthread_mutex_unlock(&patient_record_mutex);
    pthread_mutex_unlock(&medication_inventory_mutex);
    
    printf("[Pharmacy] Medication dispensed\n");
    return NULL;
}

int main() {
    pthread_t doctor, pharmacy;
    
    printf("========================================\n");
    printf("      DEADLOCK DEMONSTRATION\n");
    printf("========================================\n");
    printf("Scenario: Doctor and Pharmacy need same resources\n");
    printf("WARNING: Inconsistent lock ordering!\n");
    printf("========================================\n");
    
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
    
    /* Wait for 5 seconds to observe deadlock */
    printf("\n[System] Monitoring threads for 5 seconds...\n");
    sleep(5);
    
    /* Check if threads are still alive (deadlocked) */
    printf("\n========================================\n");
    printf("*** DEADLOCK DETECTED ***\n");
    printf("========================================\n");
    
    printf("\nDeadlock Analysis:\n");
    printf("------------------\n");
    printf("Doctor holds:  Patient Record Lock\n");
    printf("Doctor needs:  Medication Inventory Lock (held by Pharmacy)\n");
    printf("Doctor state:  WAITING...\n\n");
    
    printf("Pharmacy holds:  Medication Inventory Lock\n");
    printf("Pharmacy needs:  Patient Record Lock (held by Doctor)\n");
    printf("Pharmacy state:  WAITING...\n\n");
    
    printf("Circular Wait:\n");
    printf("  Doctor → Medication Inventory (Pharmacy has it)\n");
    printf("  Pharmacy → Patient Record (Doctor has it)\n");
    printf("  Result: Both waiting indefinitely\n\n");
    
    printf("System Impact:\n");
    printf("  ✗ Patient diagnosis NOT recorded\n");
    printf("  ✗ Medication NOT dispensed\n");
    printf("  ✗ Emergency patient NOT receiving treatment\n");
    printf("  ✗ Time elapsed: 5+ seconds (CRITICAL DELAY)\n\n");
    
    printf("Root Cause:\n");
    printf("  Inconsistent lock ordering between threads\n");
    printf("  Doctor: Patient → Medication (order 1-2)\n");
    printf("  Pharmacy: Medication → Patient (order 2-1)\n\n");
    
    printf("Solution:\n");
    printf("  ENFORCE consistent lock ordering:\n");
    printf("  ALWAYS: Patient Record (1st) → Medication Inventory (2nd)\n");
    printf("  BOTH threads must use same order\n");
    
    printf("\n========================================\n");
    printf("Note: Program will hang here (deadlock)\n");
    printf("Press Ctrl+C to terminate\n");
    printf("========================================\n");
    
    /* These will never return due to deadlock */
    pthread_join(doctor, NULL);
    pthread_join(pharmacy, NULL);
    
    /* This code is unreachable */
    printf("Program completed successfully\n");
    
    return 0;
}
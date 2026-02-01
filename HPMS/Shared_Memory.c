/*
 * HPMS Shared Memory Demo - POSIX IPC
 * Scenario: Monitoring device shares vitals with multiple displays
 * 
 * Compile: gcc -pthread shared_memory_posix.c -o shared_memory -lrt
 * Run: ./shared_memory
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#define SHM_NAME "/hpms_vitals"
#define SHM_SIZE 1024

typedef struct {
    int heart_rate;
    int blood_pressure_systolic;
    int blood_pressure_diastolic;
    int oxygen_saturation;
    char status[50];
} VitalsData;

int main() {
    int shm_fd;
    VitalsData *vitals;
    
    printf("========================================\n");
    printf("   POSIX SHARED MEMORY DEMONSTRATION\n");
    printf("========================================\n");
    printf("Scenario: Real-time vitals monitoring\n");
    printf("Security: CRITICAL - Owner-only (0600)\n\n");
    
    // Create shared memory with SECURE permissions (0600 = owner only)
    shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0600);
    if (shm_fd == -1) {
        perror("shm_open failed");
        return 1;
    }
    
    // Set size
    if (ftruncate(shm_fd, sizeof(VitalsData)) == -1) {
        perror("ftruncate failed");
        return 1;
    }
    
    // Map to memory
    vitals = (VitalsData *)mmap(0, sizeof(VitalsData), 
                                PROT_READ | PROT_WRITE, 
                                MAP_SHARED, shm_fd, 0);
    if (vitals == MAP_FAILED) {
        perror("mmap failed");
        return 1;
    }
    
    // Monitoring device writes vitals
    printf("[Monitoring Device] Writing patient vitals to shared memory...\n");
    vitals->heart_rate = 145;
    vitals->blood_pressure_systolic = 150;
    vitals->blood_pressure_diastolic = 95;
    vitals->oxygen_saturation = 92;
    strcpy(vitals->status, "CRITICAL - Hypertensive Emergency");
    printf("[Monitoring Device] ✓ Vitals updated\n\n");
    
    // Multiple displays read simultaneously (no copying!)
    printf("[Bedside Monitor] Reading vitals from shared memory:\n");
    printf("  Heart Rate: %d bpm\n", vitals->heart_rate);
    printf("  Blood Pressure: %d/%d mmHg\n", 
           vitals->blood_pressure_systolic, vitals->blood_pressure_diastolic);
    printf("  O2 Saturation: %d%%\n", vitals->oxygen_saturation);
    printf("  Status: %s\n\n", vitals->status);
    
    printf("[Nurse Station] Reading same data (zero latency):\n");
    printf("  Patient Status: %s ⚠️\n", vitals->status);
    printf("  Vitals: HR=%d BP=%d/%d O2=%d%%\n\n", 
           vitals->heart_rate, 
           vitals->blood_pressure_systolic,
           vitals->blood_pressure_diastolic,
           vitals->oxygen_saturation);
    
    printf("========================================\n");
    printf("⚠️  SECURITY WARNING - CRITICAL:\n");
    printf("========================================\n");
    printf("✗ NEVER store patient names in shared memory\n");
    printf("✗ NEVER store medical record numbers\n");
    printf("✓ ONLY non-identifiable vitals (HIPAA compliant)\n");
    printf("✓ MANDATORY 0600 permissions (owner-only)\n");
    printf("✓ Microsecond latency for life-critical monitoring\n");
    printf("========================================\n");
    
    // Cleanup
    munmap(vitals, sizeof(VitalsData));
    close(shm_fd);
    shm_unlink(SHM_NAME);
    
    return 0;
}
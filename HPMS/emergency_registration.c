// emergency_regis.c
#include <stdio.h>
#include <windows.h>

int main() {
    DWORD pid = GetCurrentProcessId();

    printf("Emergency Registration Process Started | PID: %lu\n", pid);

    volatile unsigned long long i = 0;

    // Infinite CPU load loop
    while (1) {
        i++;   // keep CPU busy
    }

    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#endif

/**
 * Validates if string contains only digits
 * @param str Input string
 * @return 1 if valid, 0 otherwise
 */
int isNumeric(const char* str) {
    if (str == NULL || *str == '\\0') {
        return 0;
    }
    
    while (*str) {
        if (!isdigit(*str)) {
            return 0;
        }
        str++;
    }
    return 1;
}

/**
 * Terminates a process by PID with security validations
 * @param pidStr Process ID as string
 * @return 1 if successful, 0 otherwise
 */
int terminateProcess(const char* pidStr) {
    // Input validation - prevent injection attacks
    if (pidStr == NULL || strlen(pidStr) == 0) {
        fprintf(stderr, "Error: PID cannot be null or empty\\n");
        return 0;
    }
    
    // Validate PID is numeric only (prevent command injection)
    if (!isNumeric(pidStr)) {
        fprintf(stderr, "Error: PID must contain only digits\\n");
        return 0;
    }
    
    // Convert to long and validate
    char* endptr;
    long pid = strtol(pidStr, &endptr, 10);
    
    // Check conversion errors
    if (*endptr != '\\0') {
        fprintf(stderr, "Error: Invalid PID format\\n");
        return 0;
    }
    
    // Validate PID is positive
    if (pid <= 0) {
        fprintf(stderr, "Error: PID must be positive\\n");
        return 0;
    }
    
    // Security: prevent terminating system critical PIDs (1-1000)
    if (pid <= 1000) {
        fprintf(stderr, "Warning: Cannot terminate system process (PID <= 1000)\\n");
        return 0;
    }
    
#ifdef _WIN32
    // Windows implementation
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, (DWORD)pid);
    if (hProcess == NULL) {
        fprintf(stderr, "Error: Cannot open process %ld\\n", pid);
        return 0;
    }
    
    BOOL result = TerminateProcess(hProcess, 1);
    CloseHandle(hProcess);
    
    if (result) {
        printf("Process %ld terminated successfully\\n", pid);
        return 1;
    } else {
        fprintf(stderr, "Failed to terminate process %ld\\n", pid);
        return 0;
    }
#else
    // Unix/Linux implementation
    // Check if process exists
    if (kill(pid, 0) == -1) {
        fprintf(stderr, "Error: Process %ld does not exist or no permission\\n", pid);
        return 0;
    }
    
    // Attempt termination
    if (kill(pid, SIGTERM) == 0) {
        printf("Process %ld terminated successfully\\n", pid);
        return 1;
    } else {
        fprintf(stderr, "Failed to terminate process %ld\\n", pid);
        return 0;
    }
#endif
}

int main(int argc, char* argv[]) {
    printf("=== Process Termination Test Cases ===\\n\\n");
    
    // Test Case 1: Valid PID format
    printf("Test 1: Valid PID (12345)\\n");
    terminateProcess("12345");
    printf("\\n");
    
    // Test Case 2: Invalid PID (negative)
    printf("Test 2: Negative PID (-100)\\n");
    terminateProcess("-100");
    printf("\\n");
    
    // Test Case 3: Invalid PID (non-numeric)
    printf("Test 3: Non-numeric PID (abc123)\\n");
    terminateProcess("abc123");
    printf("\\n");
    
    // Test Case 4: System process protection
    printf("Test 4: System process (PID 1)\\n");
    terminateProcess("1");
    printf("\\n");
    
    // Test Case 5: Empty/null input
    printf("Test 5: Empty PID\\n");
    terminateProcess("");
    printf("\\n");
    
    // Command line argument handling
    if (argc > 1) {
        printf("=== Command Line Execution ===\\n");
        for (int i = 1; i < argc; i++) {
            printf("Attempting to terminate PID: %s\\n", argv[i]);
            terminateProcess(argv[i]);
        }
    }
    
    return 0;
}

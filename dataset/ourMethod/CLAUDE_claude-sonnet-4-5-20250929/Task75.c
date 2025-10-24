
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#endif

/* CWE-20: Input Validation - Parse and validate PID from string */
/* Ensures input is a valid positive integer within system limits */
int parsePID(const char* str, long* pid) {
    size_t i = 0;
    size_t j = 0;
    char* endptr = NULL;
    long value = 0;
    
    /* Null pointer check - CWE-476 */
    if (str == NULL || pid == NULL) {
        return 0;
    }
    
    /* Check for empty string */
    if (str[0] == '\\0') {
        return 0;
    }
    
    /* Skip leading whitespace */
    while (str[i] == ' ' || str[i] == '\\t') {
        i++;
    }
    
    /* Check for invalid characters (negative, positive sign, empty after whitespace) */
    if (str[i] == '\\0' || str[i] == '-' || str[i] == '+') {
        return 0;
    }
    
    /* Validate all characters are digits - CWE-20 */
    for (j = i; str[j] != '\\0'; j++) {
        if (str[j] < '0' || str[j] > '9') {
            return 0;
        }
    }
    
    /* Use strtol with comprehensive error checking - CWE-190 overflow prevention */
    errno = 0;
    value = strtol(str, &endptr, 10);
    
    /* Check for conversion errors */
    if (errno == ERANGE || endptr == str || *endptr != '\\0') {
        return 0;
    }
    
    /* Validate PID range (must be positive and within system limits) */
    if (value <= 0 || value > INT_MAX) {
        return 0;
    }
    
    *pid = value;
    return 1;
}

/* CWE-250, CWE-273: Proper privilege handling and secure process termination */
int terminateProcess(long pid) {
#ifdef _WIN32
    HANDLE hProcess = NULL;
    BOOL result = FALSE;
    DWORD error = 0;
    
    /* Windows implementation - open process with minimal privileges */
    hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, (DWORD)pid);
    
    if (hProcess == NULL) {
        fprintf(stderr, "Error: Cannot open process %ld. Error code: %lu\\n", 
                pid, GetLastError());
        return 0;
    }
    
    /* Attempt to terminate the process */
    result = TerminateProcess(hProcess, 1);
    error = GetLastError();
    CloseHandle(hProcess);
    
    if (!result) {
        fprintf(stderr, "Error: Failed to terminate process %ld. Error code: %lu\\n", 
                pid, error);
        return 0;
    }
    
    return 1;
#else
    /* Unix/Linux implementation using kill system call with SIGTERM */
    if (kill((pid_t)pid, SIGTERM) == -1) {
        fprintf(stderr, "Error: Failed to terminate process %ld. Error: %s\\n", 
                pid, strerror(errno));
        return 0;
    }
    
    return 1;
#endif
}

int main(int argc, char* argv[]) {
    long pid = 0;
    
    /* CWE-20: Validate command line argument count */
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <PID>\\n", argv[0] != NULL ? argv[0] : "program");
        return 1;
    }
    
    /* CWE-476: Null pointer check for argv[1] */
    if (argv[1] == NULL) {
        fprintf(stderr, "Error: Invalid argument\\n");
        return 1;
    }
    
    /* CWE-20: Strict input validation to prevent injection or overflow */
    if (!parsePID(argv[1], &pid)) {
        fprintf(stderr, "Error: Invalid PID. Must be a positive integer.\\n");
        return 1;
    }
    
    printf("Attempting to terminate process with PID: %ld\\n", pid);
    
    /* CWE-252: Check return value from terminateProcess */
    if (terminateProcess(pid)) {
        printf("Process %ld terminated successfully.\\n", pid);
        return 0;
    } else {
        fprintf(stderr, "Failed to terminate process %ld\\n", pid);
        return 1;
    }
}

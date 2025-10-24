#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

// OS-specific includes for process termination
#ifdef _WIN32
#include <windows.h>
#include <processthreadsapi.h>
#else
#include <signal.h>
#include <sys/types.h>
#endif

/**
 * @brief Terminates a process by its Process ID (PID).
 * @param pid The ID of the process to terminate.
 * @return 0 on success, -1 on failure.
 */
int terminateProcessById(long pid) {
    if (pid <= 0) {
        fprintf(stderr, "Error: PID must be a positive number.\n");
        return -1;
    }

    printf("Attempting to terminate process with PID: %ld\n", pid);

#ifdef _WIN32
    // Windows implementation
    HANDLE processHandle = OpenProcess(PROCESS_TERMINATE, FALSE, (DWORD)pid);
    if (processHandle == NULL) {
        fprintf(stderr, "Error: Could not open process with PID %ld. Windows API Error Code: %lu\n", pid, GetLastError());
        return -1;
    }

    // TerminateProcess returns non-zero for success
    BOOL success = TerminateProcess(processHandle, 1);
    DWORD lastError = GetLastError(); // Get error code before closing handle
    CloseHandle(processHandle);

    if (!success) {
        fprintf(stderr, "Error: Could not terminate process with PID %ld. Windows API Error Code: %lu\n", pid, lastError);
        return -1;
    }
    return 0; // Success

#else // POSIX-like systems (Linux, macOS)
    // kill returns 0 for success, -1 for failure
    if (kill((pid_t)pid, SIGTERM) == 0) {
        return 0; // Success
    } else {
        // errno is set on failure
        fprintf(stderr, "Error: Could not send signal to process with PID %ld. Reason: %s\n", pid, strerror(errno));
        return -1;
    }
#endif
}

/**
 * @brief Helper function for running test cases.
 * @param pid The PID to test termination with.
 */
void testTerminate(long pid) {
    printf("\n--- Testing with PID: %ld ---\n", pid);
    if (terminateProcessById(pid) == 0) {
        printf("Result: Termination signal sent successfully.\n");
    } else {
        printf("Result: Failed to send termination signal.\n");
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <PID>\n", argv[0]);
        printf("\n--- Running Built-in Test Cases ---\n");
        // Note: These test cases use placeholder PIDs.

        // Test 1: PID 1 (special process), will likely fail due to permissions.
        testTerminate(1);
        
        // Test 2: A PID that is highly unlikely to exist.
        testTerminate(999999);

        // Test 3: An invalid PID (zero).
        testTerminate(0);

        // Test 4: An invalid PID (negative).
        testTerminate(-10);

        // Test 5: A placeholder for a real PID you can test with.
        testTerminate(12345);
        printf("\n--- Test Cases Finished ---\n");
        return 1;
    }

    char* endptr;
    errno = 0; // To distinguish success/failure of strtol
    long pid = strtol(argv[1], &endptr, 10);

    // Robust input validation using strtol's error-checking features
    if (endptr == argv[1]) {
        fprintf(stderr, "Error: No digits were found in PID argument.\n");
        return 1;
    }
    if (*endptr != '\0') {
        fprintf(stderr, "Error: Trailing non-numeric characters found in PID: %s\n", endptr);
        return 1;
    }
    if (errno == ERANGE) {
        fprintf(stderr, "Error: PID is out of range for a long integer.\n");
        return 1;
    }

    if (terminateProcessById(pid) == 0) {
        printf("Termination signal was sent to process with PID: %ld\n", pid);
        return 0;
    } else {
        printf("Failed to terminate process with PID: %ld\n", pid);
        return 1;
    }
}
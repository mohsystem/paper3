#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Platform-specific includes and type definitions
#ifdef _WIN32
#include <windows.h>
#include <processthreadsapi.h>
typedef DWORD pid_type;
#else
#include <signal.h>
#include <sys/types.h>
#include <errno.h>
typedef pid_t pid_type;
#endif

/**
 * @brief Terminates a process identified by the given PID.
 *
 * This function is cross-platform. It uses TerminateProcess on Windows
 * and kill(pid, SIGKILL) on POSIX-compliant systems.
 *
 * @param pid The process ID to terminate.
 * @return 1 on success, 0 on failure.
 */
int terminateProcess(pid_type pid) {
#ifdef _WIN32
    // Windows implementation
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    if (hProcess == NULL) {
        fprintf(stderr, "Error: Could not open process with PID %lu. It may not exist or you may lack permissions. GetLastError(): %lu\n", pid, GetLastError());
        return 0;
    }

    // A non-zero exit code is conventional for forced termination
    if (!TerminateProcess(hProcess, 1)) {
        fprintf(stderr, "Error: Failed to terminate process with PID %lu. GetLastError(): %lu\n", pid, GetLastError());
        CloseHandle(hProcess);
        return 0;
    }

    printf("Successfully requested termination for PID: %lu\n", pid);
    CloseHandle(hProcess);
    return 1;

#else
    // POSIX (Linux, macOS) implementation
    if (kill(pid, SIGKILL) == 0) {
        printf("Successfully sent SIGKILL to PID: %d\n", (int)pid);
        return 1;
    } else {
        // kill() failed. Use perror to print a descriptive error based on errno.
        fprintf(stderr, "Error sending signal to PID %d: ", (int)pid);
        perror(NULL); 
        return 0;
    }
#endif
}

int main(int argc, char *argv[]) {
    /*
     --- Test Cases ---
     To run the test cases, compile this file (e.g., gcc -o terminate main.c) and run from the command line.
     You will need to find a valid PID on your system to test case 1.
     - On Linux/macOS, you can run `sleep 100 &` to get a background process and its PID.
     - On Windows, you can open Notepad and find its PID in Task Manager.

     1. Kill a valid process (e.g., a process with PID 1234):
        ./terminate 1234

     2. Attempt to kill a non-existent process:
        ./terminate 999999

     3. Attempt to kill a privileged process (e.g., PID 1 on Linux):
        ./terminate 1
        (This should fail with a permission error)

     4. Provide non-numeric input:
        ./terminate abc

     5. Provide an invalid PID number:
        ./terminate 0
    */
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <PID>\n", argv[0]);
        return 1;
    }

    char *endptr;
    // Set errno to 0 before calling strtol to detect its specific errors
    errno = 0;
    long pid_long = strtol(argv[1], &endptr, 10);

    // Input validation: check if strtol consumed the whole string
    if (*endptr != '\0' || endptr == argv[1]) {
        fprintf(stderr, "Error: Invalid PID provided. Please enter a purely numeric value.\n");
        return 1;
    }
    
    // Check for range errors (e.g., number too large for a long)
    if (errno == ERANGE) {
        fprintf(stderr, "Error: PID out of range for a long integer.\n");
        return 1;
    }

    if (pid_long <= 0) {
        fprintf(stderr, "Error: PID must be a positive integer.\n");
        return 1;
    }

    pid_type pid = (pid_type)pid_long;
    terminateProcess(pid);

    return 0;
}
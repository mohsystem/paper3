#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

// Platform-specific includes for process management
#ifdef _WIN32
#include <windows.h>
#include <processthreadsapi.h>
#else
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#endif

// Define a boolean type for C
typedef enum { false, true } bool;

/**
 * @brief Terminates a process identified by the given PID.
 * 
 * @param pid The process ID to terminate.
 * @return true if the termination signal was sent successfully, false otherwise.
 */
bool terminateProcess(long pid) {
    // Security: Validate PID. PIDs must be positive.
    if (pid <= 0) {
        fprintf(stderr, "Error: Invalid PID %ld. PID must be a positive number.\n", pid);
        return false;
    }

    // Security: Prevent self-termination.
#ifdef _WIN32
    if (pid == GetCurrentProcessId()) {
#else
    if (pid == getpid()) {
#endif
        fprintf(stderr, "Error: Attempting to terminate the current process is not allowed.\n");
        return false;
    }

    printf("Attempting to terminate PID: %ld\n", pid);

#ifdef _WIN32
    HANDLE processHandle = OpenProcess(PROCESS_TERMINATE, FALSE, (DWORD)pid);
    if (processHandle == NULL) {
        fprintf(stderr, "Error: Could not open process with PID %ld. It may not exist or permissions are denied. Error code: %lu\n", pid, GetLastError());
        return false;
    }
    // TerminateProcess is forceful, akin to SIGKILL.
    if (!TerminateProcess(processHandle, 1)) {
        fprintf(stderr, "Error: Failed to terminate process with PID %ld. Error code: %lu\n", pid, GetLastError());
        CloseHandle(processHandle);
        return false;
    }
    printf("Termination signal sent successfully to PID: %ld\n", pid);
    CloseHandle(processHandle);
    return true;
#else // POSIX
    // Using kill() with SIGKILL for forceful termination.
    if (kill((pid_t)pid, SIGKILL) == 0) {
        printf("Termination signal sent successfully to PID: %ld\n", pid);
        return true;
    } else {
        fprintf(stderr, "Error: Failed to send signal to PID %ld. Reason: %s\n", pid, strerror(errno));
        return false;
    }
#endif
}

void runTestCases() {
    printf("\n--- Running Test Suite ---\n");
#ifdef _WIN32
    printf("\n--- Test Case 1: Valid PID (Manual Test on Windows) ---\n");
    printf("To test, run a process like 'notepad.exe', find its PID, and run this program with the PID as an argument.\n");
#else // POSIX
    printf("\n--- Test Case 1: Valid PID ---\n");
    pid_t child_pid = fork();

    if (child_pid < 0) {
        fprintf(stderr, "Failed to fork child process for testing.\n");
    } else if (child_pid == 0) {
        // Child process
        execlp("sleep", "sleep", "300", NULL);
        exit(1);
    } else {
        // Parent process
        printf("Started child process with PID: %d\n", child_pid);
        sleep(1);
        terminateProcess(child_pid);
        
        sleep(1);
        int status;
        pid_t result = waitpid(child_pid, &status, WNOHANG);
        if (result == child_pid) {
            printf("Child process confirmed terminated.\n");
        } else if (result == 0) {
            fprintf(stderr, "Child process is still alive (termination failed).\n");
            kill(child_pid, SIGKILL);
        } else {
            perror("waitpid failed");
        }
    }
#endif

    printf("\n--- Test Case 2: Non-existent PID ---\n");
    terminateProcess(99999);

    printf("\n--- Test Case 3: Invalid PID (0) ---\n");
    terminateProcess(0);

    printf("\n--- Test Case 4: Invalid PID (-1) ---\n");
    terminateProcess(-1);

    printf("\n--- Test Case 5: Self-termination ---\n");
#ifdef _WIN32
    terminateProcess(GetCurrentProcessId());
#else
    terminateProcess(getpid());
#endif

    printf("\n--- Test Suite Finished ---\n");
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        char* end;
        // Security: Use strtol for safe conversion and to detect non-numeric input.
        long pid_to_kill = strtol(argv[1], &end, 10);
        if (*end != '\0' || argv[1] == end) {
            fprintf(stderr, "Error: Invalid input. PID must be a numeric value.\n");
            return 1;
        }
        terminateProcess(pid_to_kill);
    } else {
        printf("No PID provided. Running test cases...\n");
        runTestCases();
    }
    return 0;
}
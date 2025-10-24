#include <stdio.h>
#include <stdlib.h>

// Platform-specific headers
#ifdef _WIN32
#include <windows.h>
#include <processthreadsapi.h>
#else
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#endif

/**
 * Terminates a process by its Process ID (PID).
 * @param pid The PID of the process to terminate.
 * @return 1 (true) if successful, 0 (false) otherwise.
 */
int terminateProcess(int pid) {
    if (pid <= 0) return 0;

#ifdef _WIN32
    // Use the Windows API to terminate the process
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, 0, pid);
    if (hProcess == NULL) {
        return 0;
    }
    // TerminateProcess returns non-zero on success
    int result = TerminateProcess(hProcess, 1);
    CloseHandle(hProcess);
    return result != 0;
#else
    // Use the POSIX kill function to send a SIGKILL signal
    // kill returns 0 on success, -1 on error
    return kill(pid, SIGKILL) == 0;
#endif
}

/**
 * Starts a dummy background process for testing purposes.
 * @return The PID of the new process, or -1 on failure.
 */
int startDummyProcess() {
#ifdef _WIN32
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    
    char cmd[] = "cmd /c timeout 60";

    if (!CreateProcessA(NULL, cmd, NULL, NULL, 0, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        return -1;
    }
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return pi.dwProcessId;
#else
    pid_t pid = fork();
    if (pid == -1) {
        return -1; // Fork failed
    } else if (pid == 0) {
        // Child process
        execlp("sleep", "sleep", "60", (char *)NULL);
        exit(1); // Exit if execlp fails
    } else {
        // Parent process
        return pid;
    }
#endif
}

void run_tests() {
    printf("No PID provided. Running built-in test cases...\n");

    // Test Case 1: Terminate a valid process
    printf("\n--- Test Case 1: Terminate a valid process ---\n");
    int pid1 = startDummyProcess();
    if (pid1 != -1) {
        printf("Started dummy process with PID: %d\n", pid1);
        int result1 = terminateProcess(pid1);
        printf("Termination result: %s\n", result1 ? "SUCCESS" : "FAILURE");

        #ifndef _WIN32
            waitpid(pid1, NULL, 0); // Clean up zombie process on POSIX
        #endif

        // Test Case 5: Attempt to terminate an already terminated process
        printf("\n--- Test Case 5: Re-terminate the same process ---\n");
        printf("Attempting to terminate PID %d again.\n", pid1);
        int result5 = terminateProcess(pid1);
        printf("Second termination result: %s\n", result5 ? "SUCCESS" : "FAILURE (Expected)");
    } else {
        fprintf(stderr, "Skipping Test 1 & 5 as dummy process could not be started.\n");
    }

    // Test Case 2: Terminate another valid process
    printf("\n--- Test Case 2: Terminate another valid process ---\n");
    int pid2 = startDummyProcess();
    if (pid2 != -1) {
        printf("Started second dummy process with PID: %d\n", pid2);
        int result2 = terminateProcess(pid2);
        printf("Termination result: %s\n", result2 ? "SUCCESS" : "FAILURE");
        #ifndef _WIN32
            waitpid(pid2, NULL, 0);
        #endif
    } else {
        fprintf(stderr, "Skipping Test 2 as dummy process could not be started.\n");
    }

    // Test Case 3: Terminate a non-existent process
    printf("\n--- Test Case 3: Terminate a non-existent process ---\n");
    int nonExistentPid = 99999;
    printf("Attempting to terminate non-existent PID: %d\n", nonExistentPid);
    int result3 = terminateProcess(nonExistentPid);
    printf("Termination result: %s\n", result3 ? "SUCCESS" : "FAILURE (Expected)");

    // Test Case 4: Terminate an invalid PID (<=0)
    printf("\n--- Test Case 4: Terminate an invalid PID (e.g., 0) ---\n");
    int invalidPid = 0;
    printf("Attempting to terminate invalid PID: %d\n", invalidPid);
    int result4 = terminateProcess(invalidPid);
    printf("Termination result: %s\n", result4 ? "SUCCESS" : "FAILURE (Expected)");
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        int pid = atoi(argv[1]);
        if (pid == 0 && argv[1][0] != '0') { // Basic check for non-numeric input
             fprintf(stderr, "Invalid PID provided. Please enter a numeric PID.\n");
             return 1;
        }
        printf("Attempting to terminate process with PID: %d\n", pid);
        if (terminateProcess(pid)) {
            printf("Termination command for process %d sent successfully.\n", pid);
        } else {
            printf("Failed to terminate process %d. It might not exist or you may lack permissions.\n", pid);
        }
    } else {
        run_tests();
    }
    return 0;
}
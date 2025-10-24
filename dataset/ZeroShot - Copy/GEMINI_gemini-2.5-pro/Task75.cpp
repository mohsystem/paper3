#include <iostream>
#include <string>
#include <cstdlib>
#include <cerrno>
#include <cstring>

// Platform-specific includes for process management
#ifdef _WIN32
#include <windows.h>
#include <processthreadsapi.h>
#else
#include <csignal>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#endif

/**
 * @brief Terminates a process identified by the given PID.
 * 
 * @param pid The process ID to terminate.
 * @return true if the termination signal was sent successfully, false otherwise.
 */
bool terminateProcess(long pid) {
    // Security: Validate PID. PIDs must be positive.
    if (pid <= 0) {
        std::cerr << "Error: Invalid PID " << pid << ". PID must be a positive number." << std::endl;
        return false;
    }

    // Security: Prevent self-termination.
#ifdef _WIN32
    if (pid == GetCurrentProcessId()) {
#else
    if (pid == getpid()) {
#endif
        std::cerr << "Error: Attempting to terminate the current process is not allowed." << std::endl;
        return false;
    }

    std::cout << "Attempting to terminate PID: " << pid << std::endl;
    
#ifdef _WIN32
    HANDLE processHandle = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    if (processHandle == NULL) {
        std::cerr << "Error: Could not open process with PID " << pid << ". It may not exist or permissions are denied. Error code: " << GetLastError() << std::endl;
        return false;
    }
    // TerminateProcess is forceful, akin to SIGKILL.
    if (!TerminateProcess(processHandle, 1)) {
        std::cerr << "Error: Failed to terminate process with PID " << pid << ". Error code: " << GetLastError() << std::endl;
        CloseHandle(processHandle);
        return false;
    }
    std::cout << "Termination signal sent successfully to PID: " << pid << std::endl;
    CloseHandle(processHandle);
    return true;
#else // POSIX
    // Using kill() with SIGKILL for forceful termination.
    if (kill(pid, SIGKILL) == 0) {
        std::cout << "Termination signal sent successfully to PID: " << pid << std::endl;
        return true;
    } else {
        std::cerr << "Error: Failed to send signal to PID " << pid << ". Reason: " << strerror(errno) << std::endl;
        return false;
    }
#endif
}

void runTestCases() {
    std::cout << "\n--- Running Test Suite ---" << std::endl;
#ifdef _WIN32
    // Process creation on Windows is more complex. The test cases below that don't
    // require a child process will still run.
    std::cout << "\n--- Test Case 1: Valid PID (Manual Test on Windows) ---" << std::endl;
    std::cout << "To test, run a process like 'notepad.exe', find its PID, "
              << "and run this program with the PID as an argument." << std::endl;
#else // POSIX
    // Test Case 1: Terminate a valid child process.
    std::cout << "\n--- Test Case 1: Valid PID ---" << std::endl;
    pid_t child_pid = fork();

    if (child_pid < 0) {
        std::cerr << "Failed to fork child process for testing." << std::endl;
    } else if (child_pid == 0) {
        // In child process, execute a long-running command.
        execlp("sleep", "sleep", "300", NULL);
        exit(1); // Should only be reached if execlp fails.
    } else {
        // In parent process.
        std::cout << "Started child process with PID: " << child_pid << std::endl;
        sleep(1);
        terminateProcess(child_pid);
        
        sleep(1);
        int status;
        pid_t result = waitpid(child_pid, &status, WNOHANG);
        if (result == child_pid) {
            std::cout << "Child process confirmed terminated." << std::endl;
        } else if (result == 0) {
            std::cerr << "Child process is still alive (termination failed)." << std::endl;
            kill(child_pid, SIGKILL); // Final cleanup.
        } else {
            std::cerr << "waitpid failed." << std::endl;
        }
    }
#endif

    // Test Case 2: Attempt to terminate a non-existent PID.
    std::cout << "\n--- Test Case 2: Non-existent PID ---" << std::endl;
    terminateProcess(99999);

    // Test Case 3: Attempt to terminate PID 0 (rejected by validation).
    std::cout << "\n--- Test Case 3: Invalid PID (0) ---" << std::endl;
    terminateProcess(0);

    // Test Case 4: Attempt to terminate PID -1 (rejected by validation).
    std::cout << "\n--- Test Case 4: Invalid PID (-1) ---" << std::endl;
    terminateProcess(-1);

    // Test Case 5: Attempt to terminate the current process.
    std::cout << "\n--- Test Case 5: Self-termination ---" << std::endl;
#ifdef _WIN32
    terminateProcess(GetCurrentProcessId());
#else
    terminateProcess(getpid());
#endif

    std::cout << "\n--- Test Suite Finished ---" << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        char* end;
        // Security: Use strtol for safe conversion and to detect non-numeric input.
        long pid_to_kill = strtol(argv[1], &end, 10);
        if (*end != '\0' || argv[1] == end) {
            std::cerr << "Error: Invalid input. PID must be a numeric value." << std::endl;
            return 1;
        }
        terminateProcess(pid_to_kill);
    } else {
        std::cout << "No PID provided. Running test cases..." << std::endl;
        runTestCases();
    }
    return 0;
}
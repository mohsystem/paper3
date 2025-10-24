#include <iostream>
#include <string>
#include <cstdlib>

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
 * @return True if successful, false otherwise.
 */
bool terminateProcess(int pid) {
    if (pid <= 0) return false;

#ifdef _WIN32
    // Use the Windows API to terminate the process
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    if (hProcess == NULL) {
        return false; // Cannot open process (might not exist or no permissions)
    }
    BOOL result = TerminateProcess(hProcess, 1);
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
    
    // Command to run (must be a mutable char array for CreateProcess)
    char cmd[] = "cmd /c timeout 60";

    if (!CreateProcessA(NULL, cmd, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        return -1;
    }
    // Close handles we don't need to prevent resource leaks
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return pi.dwProcessId;
#else
    pid_t pid = fork();
    if (pid == -1) {
        // Fork failed
        return -1;
    } else if (pid == 0) {
        // This is the child process. Replace its image with 'sleep'.
        execlp("sleep", "sleep", "60", (char *)NULL);
        // If execlp returns, it must have failed
        exit(1);
    } else {
        // This is the parent process. Return the child's PID.
        return pid;
    }
#endif
}

void run_tests() {
    std::cout << "No PID provided. Running built-in test cases..." << std::endl;

    // Test Case 1: Terminate a valid process
    std::cout << "\n--- Test Case 1: Terminate a valid process ---" << std::endl;
    int pid1 = startDummyProcess();
    if (pid1 != -1) {
        std::cout << "Started dummy process with PID: " << pid1 << std::endl;
        bool result1 = terminateProcess(pid1);
        std::cout << "Termination result: " << (result1 ? "SUCCESS" : "FAILURE") << std::endl;
        
        #ifndef _WIN32
            waitpid(pid1, NULL, 0); // On POSIX, wait for child to prevent a zombie process
        #endif

        // Test Case 5: Attempt to terminate an already terminated process
        std::cout << "\n--- Test Case 5: Re-terminate the same process ---" << std::endl;
        std::cout << "Attempting to terminate PID " << pid1 << " again." << std::endl;
        bool result5 = terminateProcess(pid1);
        std::cout << "Second termination result: " << (result5 ? "SUCCESS" : "FAILURE (Expected)") << std::endl;
    } else {
        std::cerr << "Skipping Test 1 & 5 as dummy process could not be started." << std::endl;
    }

    // Test Case 2: Terminate another valid process
    std::cout << "\n--- Test Case 2: Terminate another valid process ---" << std::endl;
    int pid2 = startDummyProcess();
    if (pid2 != -1) {
        std::cout << "Started second dummy process with PID: " << pid2 << std::endl;
        bool result2 = terminateProcess(pid2);
        std::cout << "Termination result: " << (result2 ? "SUCCESS" : "FAILURE") << std::endl;
        #ifndef _WIN32
            waitpid(pid2, NULL, 0);
        #endif
    } else {
        std::cerr << "Skipping Test 2 as dummy process could not be started." << std::endl;
    }

    // Test Case 3: Terminate a non-existent process
    std::cout << "\n--- Test Case 3: Terminate a non-existent process ---" << std::endl;
    int nonExistentPid = 99999;
    std::cout << "Attempting to terminate non-existent PID: " << nonExistentPid << std::endl;
    bool result3 = terminateProcess(nonExistentPid);
    std::cout << "Termination result: " << (result3 ? "SUCCESS" : "FAILURE (Expected)") << std::endl;

    // Test Case 4: Terminate an invalid PID (<=0)
    std::cout << "\n--- Test Case 4: Terminate an invalid PID (e.g., 0) ---" << std::endl;
    int invalidPid = 0;
    std::cout << "Attempting to terminate invalid PID: " << invalidPid << std::endl;
    bool result4 = terminateProcess(invalidPid);
    std::cout << "Termination result: " << (result4 ? "SUCCESS" : "FAILURE (Expected)") << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        try {
            int pid = std::stoi(argv[1]);
            std::cout << "Attempting to terminate process with PID: " << pid << std::endl;
            if (terminateProcess(pid)) {
                std::cout << "Termination command for process " << pid << " sent successfully." << std::endl;
            } else {
                std::cout << "Failed to terminate process " << pid << ". It might not exist or you may lack permissions." << std::endl;
            }
        } catch (const std::invalid_argument& e) {
            std::cerr << "Invalid PID provided. Please enter a numeric PID." << std::endl;
            return 1;
        } catch (const std::out_of_range& e) {
            std::cerr << "PID out of range." << std::endl;
            return 1;
        }
    } else {
        run_tests();
    }
    return 0;
}
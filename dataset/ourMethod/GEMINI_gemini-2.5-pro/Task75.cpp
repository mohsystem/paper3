#include <iostream>
#include <string>
#include <stdexcept>

// OS-specific includes for process termination
#ifdef _WIN32
#include <windows.h>
#include <processthreadsapi.h>
#else
#include <csignal>
#include <sys/types.h>
#include <cerrno>
#include <cstring> // For strerror
#endif

/**
 * @brief Terminates a process by its Process ID (PID).
 * @param pid The ID of the process to terminate.
 * @return true if the termination signal was sent successfully, false otherwise.
 */
bool terminateProcessById(long long pid) {
    if (pid <= 0) {
        std::cerr << "Error: PID must be a positive number." << std::endl;
        return false;
    }

    std::cout << "Attempting to terminate process with PID: " << pid << std::endl;

#ifdef _WIN32
    // Windows implementation
    HANDLE processHandle = OpenProcess(PROCESS_TERMINATE, FALSE, static_cast<DWORD>(pid));
    if (processHandle == NULL) {
        std::cerr << "Error: Could not open process with PID " << pid << ". Windows API Error Code: " << GetLastError() << std::endl;
        return false;
    }

    // TerminateProcess returns non-zero for success
    BOOL success = TerminateProcess(processHandle, 1);
    DWORD lastError = GetLastError(); // Get error code before closing handle
    CloseHandle(processHandle);

    if (!success) {
        std::cerr << "Error: Could not terminate process with PID " << pid << ". Windows API Error Code: " << lastError << std::endl;
        return false;
    }
    return true;

#else // POSIX-like systems (Linux, macOS)
    // kill returns 0 for success, -1 for failure
    if (kill(static_cast<pid_t>(pid), SIGTERM) == 0) {
        return true;
    } else {
        // errno is set on failure
        std::cerr << "Error: Could not send signal to process with PID " << pid << ". Reason: " << strerror(errno) << std::endl;
        return false;
    }
#endif
}

/**
 * @brief Helper function for running test cases.
 * @param pid The PID to test termination with.
 */
void testTerminate(long long pid) {
    std::cout << "\n--- Testing with PID: " << pid << " ---" << std::endl;
    if (terminateProcessById(pid)) {
        std::cout << "Result: Termination signal sent successfully." << std::endl;
    } else {
        std::cout << "Result: Failed to send termination signal." << std::endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <PID>" << std::endl;
        std::cout << "\n--- Running Built-in Test Cases ---" << std::endl;
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
        std::cout << "\n--- Test Cases Finished ---" << std::endl;
        return 1;
    }

    try {
        // Use std::stoll for safe string-to-long-long conversion
        long long pid = std::stoll(argv[1]);
        if (terminateProcessById(pid)) {
            std::cout << "Termination signal was sent to process with PID: " << pid << std::endl;
            return 0;
        } else {
            std::cout << "Failed to terminate process with PID: " << pid << std::endl;
            return 1;
        }
    } catch (const std::invalid_argument& e) {
        std::cerr << "Error: Invalid PID provided. Not a number." << std::endl;
        return 1;
    } catch (const std::out_of_range& e) {
        std::cerr << "Error: PID provided is out of range." << std::endl;
        return 1;
    }
}
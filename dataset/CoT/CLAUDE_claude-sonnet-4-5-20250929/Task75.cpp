
#include <iostream>
#include <string>
#include <cstdlib>
#include <regex>
#include <csignal>

#ifdef _WIN32
#include <windows.h>
#include <tlhelp32.h>
#else
#include <unistd.h>
#include <sys/types.h>
#endif

class Task75 {
public:
    /**
     * Terminates a process by PID with security validations
     * @param pidStr Process ID as string
     * @return true if successful, false otherwise
     */
    static bool terminateProcess(const std::string& pidStr) {
        // Input validation - prevent injection attacks
        if (pidStr.empty()) {
            std::cerr << "Error: PID cannot be null or empty" << std::endl;
            return false;
        }
        
        // Validate PID is numeric only (prevent command injection)
        std::regex numericPattern("^[0-9]+$");
        if (!std::regex_match(pidStr, numericPattern)) {
            std::cerr << "Error: PID must contain only digits" << std::endl;
            return false;
        }
        
        try {
            long pid = std::stol(pidStr);
            
            // Validate PID is positive
            if (pid <= 0) {
                std::cerr << "Error: PID must be positive" << std::endl;
                return false;
            }
            
            // Security: prevent terminating system critical PIDs (1-1000)
            if (pid <= 1000) {
                std::cerr << "Warning: Cannot terminate system process (PID <= 1000)" << std::endl;
                return false;
            }
            
#ifdef _WIN32
            // Windows implementation
            HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, static_cast<DWORD>(pid));
            if (hProcess == NULL) {
                std::cerr << "Error: Cannot open process " << pid << std::endl;
                return false;
            }
            
            BOOL result = TerminateProcess(hProcess, 1);
            CloseHandle(hProcess);
            
            if (result) {
                std::cout << "Process " << pid << " terminated successfully" << std::endl;
                return true;
            } else {
                std::cerr << "Failed to terminate process " << pid << std::endl;
                return false;
            }
#else
            // Unix/Linux implementation
            if (kill(pid, 0) == -1) {
                std::cerr << "Error: Process " << pid << " does not exist or no permission" << std::endl;
                return false;
            }
            
            if (kill(pid, SIGTERM) == 0) {
                std::cout << "Process " << pid << " terminated successfully" << std::endl;
                return true;
            } else {
                std::cerr << "Failed to terminate process " << pid << std::endl;
                return false;
            }
#endif
            
        } catch (const std::invalid_argument& e) {
            std::cerr << "Error: Invalid PID format" << std::endl;
            return false;
        } catch (const std::out_of_range& e) {
            std::cerr << "Error: PID out of range" << std::endl;
            return false;
        } catch (...) {
            std::cerr << "Error: Unexpected error occurred" << std::endl;
            return false;
        }
    }
};

int main(int argc, char* argv[]) {
    std::cout << "=== Process Termination Test Cases ===" << std::endl << std::endl;
    
    // Test Case 1: Valid PID format
    std::cout << "Test 1: Valid PID (12345)" << std::endl;
    Task75::terminateProcess("12345");
    std::cout << std::endl;
    
    // Test Case 2: Invalid PID (negative)
    std::cout << "Test 2: Negative PID (-100)" << std::endl;
    Task75::terminateProcess("-100");
    std::cout << std::endl;
    
    // Test Case 3: Invalid PID (non-numeric)
    std::cout << "Test 3: Non-numeric PID (abc123)" << std::endl;
    Task75::terminateProcess("abc123");
    std::cout << std::endl;
    
    // Test Case 4: System process protection
    std::cout << "Test 4: System process (PID 1)" << std::endl;
    Task75::terminateProcess("1");
    std::cout << std::endl;
    
    // Test Case 5: Empty/null input
    std::cout << "Test 5: Empty PID" << std::endl;
    Task75::terminateProcess("");
    std::cout << std::endl;
    
    // Command line argument handling
    if (argc > 1) {
        std::cout << "=== Command Line Execution ===" << std::endl;
        for (int i = 1; i < argc; i++) {
            std::cout << "Attempting to terminate PID: " << argv[i] << std::endl;
            Task75::terminateProcess(argv[i]);
        }
    }
    
    return 0;
}

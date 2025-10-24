
#include <iostream>
#include <string>
#include <cstdlib>
#include <csignal>

#ifdef _WIN32
#include <windows.h>
#include <tlhelp32.h>
#else
#include <unistd.h>
#include <sys/types.h>
#endif

std::string terminateProcess(const std::string& pidStr) {
    try {
        // Validate PID is numeric
        int pid = std::stoi(pidStr);
        if (pid <= 0) {
            return "Error: Invalid PID. Must be a positive number.";
        }
        
#ifdef _WIN32
        // Windows implementation
        HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
        if (hProcess == NULL) {
            return "Error: Unable to open process " + std::to_string(pid) + ". Access denied or process not found.";
        }
        
        if (TerminateProcess(hProcess, 1)) {
            CloseHandle(hProcess);
            return "Process " + std::to_string(pid) + " terminated successfully.";
        } else {
            CloseHandle(hProcess);
            return "Error: Failed to terminate process " + std::to_string(pid);
        }
#else
        // Unix/Linux/Mac implementation
        if (kill(pid, SIGKILL) == 0) {
            return "Process " + std::to_string(pid) + " terminated successfully.";
        } else {
            if (errno == ESRCH) {
                return "Error: Process " + std::to_string(pid) + " not found.";
            } else if (errno == EPERM) {
                return "Error: Permission denied. Run as root/administrator.";
            } else {
                return "Error: Failed to terminate process " + std::to_string(pid);
            }
        }
#endif
    } catch (const std::invalid_argument& e) {
        return "Error: Invalid PID format. Must be a number.";
    } catch (const std::out_of_range& e) {
        return "Error: PID out of range.";
    } catch (const std::exception& e) {
        return "Error: " + std::string(e.what());
    }
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        std::cout << terminateProcess(argv[1]) << std::endl;
    } else {
        // Test cases
        std::cout << "Test Case 1 (Invalid PID - negative):" << std::endl;
        std::cout << terminateProcess("-1") << std::endl;
        
        std::cout << "\\nTest Case 2 (Invalid PID - zero):" << std::endl;
        std::cout << terminateProcess("0") << std::endl;
        
        std::cout << "\\nTest Case 3 (Invalid format):" << std::endl;
        std::cout << terminateProcess("abc") << std::endl;
        
        std::cout << "\\nTest Case 4 (Non-existent PID):" << std::endl;
        std::cout << terminateProcess("999999") << std::endl;
        
        std::cout << "\\nTest Case 5 (Empty string):" << std::endl;
        std::cout << terminateProcess("") << std::endl;
    }
    
    return 0;
}

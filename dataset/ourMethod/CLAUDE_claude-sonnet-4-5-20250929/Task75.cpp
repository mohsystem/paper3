
#include <iostream>
#include <string>
#include <cstring>
#include <cerrno>
#include <climits>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#endif

// CWE-20: Input Validation - Parse and validate PID from string
// Ensures input is a valid positive integer within system limits
bool parsePID(const char* str, long& pid) {
    if (str == nullptr || str[0] == '\\0') {
        return false;
    }
    
    // Check for non-digit characters (except leading whitespace)
    size_t i = 0;
    while (str[i] == ' ' || str[i] == '\\t') i++;
    
    if (str[i] == '\\0' || str[i] == '-' || str[i] == '+') {
        return false; // Empty, negative, or explicit positive sign not allowed
    }
    
    for (size_t j = i; str[j] != '\\0'; j++) {
        if (str[j] < '0' || str[j] > '9') {
            return false;
        }
    }
    
    // Use strtol with error checking to prevent overflow
    char* endptr = nullptr;
    errno = 0;
    long value = strtol(str, &endptr, 10);
    
    // Check for conversion errors
    if (errno == ERANGE || endptr == str || *endptr != '\\0') {
        return false;
    }
    
    // Validate PID range (must be positive and within system limits)
    if (value <= 0 || value > INT_MAX) {
        return false;
    }
    
    pid = value;
    return true;
}

// CWE-250, CWE-273: Proper privilege handling and secure process termination
bool terminateProcess(long pid) {
#ifdef _WIN32
    // Windows implementation using TerminateProcess
    // Open process with PROCESS_TERMINATE privilege only
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, static_cast<DWORD>(pid));
    
    if (hProcess == NULL) {
        std::cerr << "Error: Cannot open process " << pid 
                  << ". Error code: " << GetLastError() << std::endl;
        return false;
    }
    
    // Attempt to terminate the process
    BOOL result = TerminateProcess(hProcess, 1);
    DWORD error = GetLastError();
    CloseHandle(hProcess);
    
    if (!result) {
        std::cerr << "Error: Failed to terminate process " << pid 
                  << ". Error code: " << error << std::endl;
        return false;
    }
    
    return true;
#else
    // Unix/Linux implementation using kill system call
    // Use SIGTERM for graceful termination first
    if (kill(static_cast<pid_t>(pid), SIGTERM) == -1) {
        std::cerr << "Error: Failed to terminate process " << pid 
                  << ". Error: " << strerror(errno) << std::endl;
        return false;
    }
    
    return true;
#endif
}

int main(int argc, char* argv[]) {
    // CWE-20: Validate command line argument count
    if (argc != 2) {
        std::cerr << "Usage: " << (argv[0] ? argv[0] : "program") 
                  << " <PID>" << std::endl;
        return 1;
    }
    
    // CWE-476: Null pointer check
    if (argv[1] == nullptr) {
        std::cerr << "Error: Invalid argument" << std::endl;
        return 1;
    }
    
    long pid = 0;
    
    // CWE-20: Strict input validation to prevent injection or overflow
    if (!parsePID(argv[1], pid)) {
        std::cerr << "Error: Invalid PID. Must be a positive integer." << std::endl;
        return 1;
    }
    
    std::cout << "Attempting to terminate process with PID: " << pid << std::endl;
    
    // Attempt to terminate the process with proper error handling
    if (terminateProcess(pid)) {
        std::cout << "Process " << pid << " terminated successfully." << std::endl;
        return 0;
    } else {
        std::cerr << "Failed to terminate process " << pid << std::endl;
        return 1;
    }
}

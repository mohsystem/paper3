
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <signal.h>
#include <unistd.h>
#endif

void terminateProcess(const char* pidStr) {
    if (pidStr == NULL || strlen(pidStr) == 0) {
        printf("Error: Invalid PID format. Must be a number.\\n");
        return;
    }
    
    // Validate numeric input
    for (int i = 0; i < strlen(pidStr); i++) {
        if (pidStr[i] < '0' || pidStr[i] > '9') {
            printf("Error: Invalid PID format. Must be a number.\\n");
            return;
        }
    }
    
    long pid = atol(pidStr);
    if (pid <= 0) {
        printf("Error: Invalid PID. Must be a positive number.\\n");
        return;
    }
    
#ifdef _WIN32
    // Windows implementation
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, (DWORD)pid);
    if (hProcess == NULL) {
        printf("Error: Unable to open process %ld. Access denied or process not found.\\n", pid);
        return;
    }
    
    if (TerminateProcess(hProcess, 1)) {
        CloseHandle(hProcess);
        printf("Process %ld terminated successfully.\\n", pid);
    } else {
        CloseHandle(hProcess);
        printf("Error: Failed to terminate process %ld\\n", pid);
    }
#else
    // Unix/Linux/Mac implementation
    if (kill((pid_t)pid, SIGKILL) == 0) {
        printf("Process %ld terminated successfully.\\n", pid);
    } else {
        if (errno == ESRCH) {
            printf("Error: Process %ld not found.\\n", pid);
        } else if (errno == EPERM) {
            printf("Error: Permission denied. Run as root/administrator.\\n");
        } else {
            printf("Error: Failed to terminate process %ld\\n", pid);
        }
    }
#endif
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        terminateProcess(argv[1]);
    } else {
        // Test cases
        printf("Test Case 1 (Invalid PID - negative):\\n");
        terminateProcess("-1");
        
        printf("\\nTest Case 2 (Invalid PID - zero):\\n");
        terminateProcess("0");
        
        printf("\\nTest Case 3 (Invalid format):\\n");
        terminateProcess("abc");
        
        printf("\\nTest Case 4 (Non-existent PID):\\n");
        terminateProcess("999999");
        
        printf("\\nTest Case 5 (Empty string):\\n");
        terminateProcess("");
    }
    
    return 0;
}

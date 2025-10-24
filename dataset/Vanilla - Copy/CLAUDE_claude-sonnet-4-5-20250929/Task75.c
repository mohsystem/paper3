
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <signal.h>
    #include <errno.h>
#endif

void terminateProcess(int pid) {
    #ifdef _WIN32
        char command[256];
        sprintf(command, "taskkill /F /PID %d", pid);
        int result = system(command);
        if (result == 0) {
            printf("Process %d terminated successfully.\\n", pid);
        } else {
            printf("Failed to terminate process %d.\\n", pid);
        }
    #else
        if (kill(pid, SIGKILL) == 0) {
            printf("Process %d terminated successfully.\\n", pid);
        } else {
            printf("Failed to terminate process %d: %s\\n", pid, strerror(errno));
        }
    #endif
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        int pid = atoi(argv[1]);
        if (pid > 0) {
            terminateProcess(pid);
        } else {
            printf("Invalid PID format. Please provide a valid integer.\\n");
        }
    } else {
        printf("Running test cases...\\n");
        printf("\\nNote: These are demonstration test cases.\\n");
        printf("Please provide actual PIDs to terminate real processes.\\n");
        
        printf("\\nTest Case 1: PID 99999 (likely non-existent)\\n");
        terminateProcess(99999);
        
        printf("\\nTest Case 2: PID 88888 (likely non-existent)\\n");
        terminateProcess(88888);
        
        printf("\\nTest Case 3: PID 77777 (likely non-existent)\\n");
        terminateProcess(77777);
        
        printf("\\nTest Case 4: PID 66666 (likely non-existent)\\n");
        terminateProcess(66666);
        
        printf("\\nTest Case 5: PID 55555 (likely non-existent)\\n");
        terminateProcess(55555);
    }
    
    return 0;
}

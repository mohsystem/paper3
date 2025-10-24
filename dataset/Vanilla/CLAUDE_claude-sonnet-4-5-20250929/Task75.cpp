
#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <signal.h>
    #include <errno.h>
#endif

void terminateProcess(int pid) {
    #ifdef _WIN32
        std::string command = "taskkill /F /PID " + std::to_string(pid);
        int result = system(command.c_str());
        if (result == 0) {
            std::cout << "Process " << pid << " terminated successfully." << std::endl;
        } else {
            std::cout << "Failed to terminate process " << pid << "." << std::endl;
        }
    #else
        if (kill(pid, SIGKILL) == 0) {
            std::cout << "Process " << pid << " terminated successfully." << std::endl;
        } else {
            std::cout << "Failed to terminate process " << pid << ": " << strerror(errno) << std::endl;
        }
    #endif
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        try {
            int pid = std::stoi(argv[1]);
            terminateProcess(pid);
        } catch (const std::exception& e) {
            std::cout << "Invalid PID format. Please provide a valid integer." << std::endl;
        }
    } else {
        std::cout << "Running test cases..." << std::endl;
        std::cout << "\\nNote: These are demonstration test cases." << std::endl;
        std::cout << "Please provide actual PIDs to terminate real processes." << std::endl;
        
        std::cout << "\\nTest Case 1: PID 99999 (likely non-existent)" << std::endl;
        terminateProcess(99999);
        
        std::cout << "\\nTest Case 2: PID 88888 (likely non-existent)" << std::endl;
        terminateProcess(88888);
        
        std::cout << "\\nTest Case 3: PID 77777 (likely non-existent)" << std::endl;
        terminateProcess(77777);
        
        std::cout << "\\nTest Case 4: PID 66666 (likely non-existent)" << std::endl;
        terminateProcess(66666);
        
        std::cout << "\\nTest Case 5: PID 55555 (likely non-existent)" << std::endl;
        terminateProcess(55555);
    }
    
    return 0;
}

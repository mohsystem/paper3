
#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>

int runIdCommand(const char* username) {
    std::string command = "id ";
    command += username;
    
    int exitStatus = system(command.c_str());
    
    if (exitStatus == -1) {
        return -1;
    }
    
    // Extract actual exit status from system() return value
    if (WIFEXITED(exitStatus)) {
        return WEXITSTATUS(exitStatus);
    }
    
    return exitStatus;
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        const char* username = argv[1];
        int exitStatus = runIdCommand(username);
        std::cout << "Exit status: " << exitStatus << std::endl;
    } else {
        // Test cases
        const char* testUsers[] = {"root", "nobody", "daemon", "nonexistentuser123", "bin"};
        
        for (int i = 0; i < 5; i++) {
            std::cout << "\\n=== Testing with user: " << testUsers[i] << " ===" << std::endl;
            int exitStatus = runIdCommand(testUsers[i]);
            std::cout << "Exit status: " << exitStatus << std::endl;
        }
    }
    
    return 0;
}

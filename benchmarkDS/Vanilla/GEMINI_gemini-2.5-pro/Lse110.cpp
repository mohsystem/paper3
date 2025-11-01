#include <iostream>
#include <string>
#include <cstdlib> // For system()
#include <vector>
#include <sys/wait.h> // For WIFEXITED and WEXITSTATUS

/**
 * Runs the 'id <username>' command and returns its exit status.
 * @param username The username to look up.
 * @return The exit status of the 'id' command (0 for success, non-zero for failure).
 */
int runIdCommand(const std::string& username) {
    // Basic check to avoid empty commands or trivial injections
    if (username.empty() || username.find_first_of(" '`;|&") != std::string::npos) {
        return -1; // Invalid username
    }

    // Construct the command, redirecting output to /dev/null to keep the console clean
    std::string command = "id " + username + " > /dev/null 2>&1";
    
    int status = system(command.c_str());

    // system() returns the shell status. We need to extract the actual exit code.
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    
    // Command did not terminate normally
    return -1;
}

int main() {
    std::vector<std::string> testCases = {
        "root",                   // Test case 1: Should exist on most systems (success)
        "daemon",                 // Test case 2: Should also exist on most systems (success)
        "nonexistentuser12345",   // Test case 3: Should not exist (failure)
        "another_fake_user_xyz",  // Test case 4: Should not exist (failure)
        ""                        // Test case 5: Invalid input (failure)
    };
    
    std::cout << "--- Running CPP Test Cases ---" << std::endl;
    for (const auto& user : testCases) {
        int exitStatus = runIdCommand(user);
        std::cout << "Command 'id " << (user.empty() ? "<empty>" : user) 
                  << "' exited with status: " << exitStatus << std::endl;
    }
    
    return 0;
}
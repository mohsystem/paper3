#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>

/**
 * Runs the 'id <username>' command securely and returns its exit status.
 *
 * @param username The username to look up.
 * @return The exit status of the 'id' command. 0 for success, non-zero for failure.
 *         Returns -1 on a fork/exec error.
 */
int runIdCommand(const std::string& username) {
    if (username.empty()) {
        std::cerr << "Error: Username cannot be empty." << std::endl;
        return -1;
    }

    pid_t pid = fork();

    if (pid == -1) {
        // Fork failed
        perror("fork failed");
        return -1;
    } else if (pid == 0) {
        // Child process
        // Prepare arguments for execvp. It requires a null-terminated array of char*.
        // Using execvp is secure as it does not invoke a shell, preventing injection.
        const char* cmd = "id";
        char* argv[] = {const_cast<char*>("id"), const_cast<char*>(username.c_str()), nullptr};
        
        // Replace the child process with the 'id' command
        execvp(cmd, argv);
        
        // If execvp returns, an error occurred
        perror("execvp failed");
        _exit(127); // Use _exit in child to avoid flushing parent's stdio buffers
    } else {
        // Parent process
        int status;
        // Wait for the child process to terminate
        if (waitpid(pid, &status, 0) == -1) {
            perror("waitpid failed");
            return -1;
        }

        // Check if the child terminated normally
        if (WIFEXITED(status)) {
            // Return the child's exit status
            return WEXITSTATUS(status);
        } else {
            // Child terminated abnormally
            return -1;
        }
    }
}

int main(int argc, char* argv[]) {
    // 5 test cases
    std::vector<std::string> testUsernames = {
        "root",                 // A user that usually exists
        "nobody",               // Another user that often exists
        "nonexistentuser12345", // A user that should not exist
        "; whoami",             // A potential command injection attempt
        "test user"             // A username with a space
    };

    std::cout << "--- Running 5 Test Cases ---" << std::endl;
    for (const auto& user : testUsernames) {
        std::cout << "Testing username: '" << user << "'" << std::endl;
        int exitStatus = runIdCommand(user);
        std::cout << "Exit Status: " << exitStatus << std::endl << std::endl;
    }

    // Additionally, handle command line argument as per prompt description
    if (argc > 1) {
        std::cout << "--- Running with command line argument ---" << std::endl;
        std::string userFromArg = argv[1];
        std::cout << "Testing username: '" << userFromArg << "'" << std::endl;
        int exitStatus = runIdCommand(userFromArg);
        std::cout << "Exit Status: " << exitStatus << std::endl;
    }
    
    return 0;
}
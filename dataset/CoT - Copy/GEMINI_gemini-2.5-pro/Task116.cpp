#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <sys/wait.h>
#include <cstdio> // For perror

// Function to perform a privileged operation
bool performPrivilegedOperation(const std::string& username) {
    if (geteuid() != 0) {
        std::cerr << "Error: This operation requires root privileges." << std::endl;
        return false;
    }

    std::cout << "[PRIVILEGED] Simulating password change for user: " << username << std::endl;
    std::cout << "Attempting to create a test file in /root..." << std::endl;

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork failed");
        return false;
    } else if (pid == 0) {
        // Child process
        execlp("touch", "touch", "/root/privilege_test_file.cpp.txt", (char*)NULL);
        // If execlp returns, it must have failed
        perror("execlp failed");
        _exit(1);
    } else {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            std::cout << "Successfully created /root/privilege_test_file.cpp.txt" << std::endl;
            // Clean up
            remove("/root/privilege_test_file.cpp.txt");
            std::cout << "Cleaned up test file." << std::endl;
            return true;
        } else {
            std::cerr << "Privileged operation failed." << std::endl;
            return false;
        }
    }
}

// Function to drop root privileges
void dropPrivileges(const std::string& username) {
    if (geteuid() != 0) {
        std::cout << "Not running as root, no privileges to drop." << std::endl;
        return;
    }

    struct passwd* pw = getpwnam(username.c_str());
    if (pw == nullptr) {
        std::cerr << "Error: User '" << username << "' not found. Cannot drop privileges." << std::endl;
        exit(EXIT_FAILURE);
    }

    uid_t target_uid = pw->pw_uid;
    gid_t target_gid = pw->pw_gid;

    std::cout << "\nAttempting to drop privileges to user '" << username
              << "' (UID: " << target_uid << ", GID: " << target_gid << ")" << std::endl;

    // 1. Set supplementary groups
    if (initgroups(username.c_str(), target_gid) != 0) {
        perror("initgroups failed");
        exit(EXIT_FAILURE);
    }

    // 2. Set GID
    if (setgid(target_gid) != 0) {
        perror("setgid failed");
        exit(EXIT_FAILURE);
    }

    // 3. Set UID
    if (setuid(target_uid) != 0) {
        perror("setuid failed");
        exit(EXIT_FAILURE);
    }

    std::cout << "Successfully dropped privileges." << std::endl;
}

int main() {
    std::cout << "### C++ Privilege Dropping Demonstration ###" << std::endl;

    // Test Case 1: Check if running as root
    if (geteuid() != 0) {
        std::cerr << "This program must be run as root (using sudo) to demonstrate privilege dropping." << std::endl;
        return 1;
    }
    
    std::cout << "Initial state: Running as UID " << getuid() << ", GID " << getgid() << std::endl;

    // Test Case 2: Perform a privileged operation successfully
    std::cout << "\n--- Test Case 2: Perform privileged operation as root ---" << std::endl;
    performPrivilegedOperation("testuser1");

    // Test Case 3: Drop privileges to "nobody"
    std::cout << "\n--- Test Case 3: Dropping privileges ---" << std::endl;
    dropPrivileges("nobody");
    std::cout << "Current state: Running as UID " << getuid() << ", GID " << getgid() << std::endl;
    
    // Test Case 4: Attempt the same privileged operation, which should now fail
    std::cout << "\n--- Test Case 4: Attempt privileged operation after dropping privileges ---" << std::endl;
    if (!performPrivilegedOperation("testuser2")) {
        std::cout << "Test passed: Privileged operation failed as expected." << std::endl;
    }

    // Test Case 5: Dropping to a non-existent user (uncomment to test exit)
    // std::cout << "\n--- Test Case 5: Attempt to drop to non-existent user ---" << std::endl;
    // dropPrivileges("nonexistentuser12345");

    std::cout << "\nProgram finished. It would now continue running with reduced privileges." << std::endl;

    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <sys/wait.h>

// Function to perform a privileged operation
int perform_privileged_operation(const char* username) {
    if (geteuid() != 0) {
        fprintf(stderr, "Error: This operation requires root privileges.\n");
        return 0; // Failure
    }

    printf("[PRIVILEGED] Simulating password change for user: %s\n", username);
    printf("Attempting to create a test file in /root...\n");
    
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork failed");
        return 0; // Failure
    } else if (pid == 0) {
        // Child process
        execlp("touch", "touch", "/root/privilege_test_file.c.txt", (char*)NULL);
        // If execlp returns, it must have failed
        perror("execlp failed");
        _exit(1);
    } else {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            printf("Successfully created /root/privilege_test_file.c.txt\n");
            // Clean up
            remove("/root/privilege_test_file.c.txt");
            printf("Cleaned up test file.\n");
            return 1; // Success
        } else {
            fprintf(stderr, "Privileged operation failed.\n");
            return 0; // Failure
        }
    }
}

// Function to drop root privileges
void drop_privileges(const char* username) {
    if (geteuid() != 0) {
        printf("Not running as root, no privileges to drop.\n");
        return;
    }

    struct passwd* pw = getpwnam(username);
    if (pw == NULL) {
        fprintf(stderr, "Error: User '%s' not found. Cannot drop privileges.\n", username);
        exit(EXIT_FAILURE);
    }

    uid_t target_uid = pw->pw_uid;
    gid_t target_gid = pw->pw_gid;

    printf("\nAttempting to drop privileges to user '%s' (UID: %u, GID: %u)\n", 
           username, (unsigned int)target_uid, (unsigned int)target_gid);
    
    // 1. Set supplementary groups
    if (initgroups(username, target_gid) != 0) {
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

    printf("Successfully dropped privileges.\n");
}


int main() {
    printf("### C Privilege Dropping Demonstration ###\n");

    // Test Case 1: Check if running as root
    if (geteuid() != 0) {
        fprintf(stderr, "This program must be run as root (using sudo) to demonstrate privilege dropping.\n");
        return 1;
    }

    printf("Initial state: Running as UID %u, GID %u\n", getuid(), getgid());

    // Test Case 2: Perform a privileged operation successfully
    printf("\n--- Test Case 2: Perform privileged operation as root ---\n");
    perform_privileged_operation("testuser1");

    // Test Case 3: Drop privileges to "nobody"
    printf("\n--- Test Case 3: Dropping privileges ---\n");
    drop_privileges("nobody");
    printf("Current state: Running as UID %u, GID %u\n", getuid(), getgid());

    // Test Case 4: Attempt the same privileged operation, which should now fail
    printf("\n--- Test Case 4: Attempt privileged operation after dropping privileges ---\n");
    if (!perform_privileged_operation("testuser2")) {
        printf("Test passed: Privileged operation failed as expected.\n");
    }

    // Test Case 5: Dropping to a non-existent user (uncomment to test exit)
    // printf("\n--- Test Case 5: Attempt to drop to non-existent user ---\n");
    // drop_privileges("nonexistentuser12345");

    printf("\nProgram finished. It would now continue running with reduced privileges.\n");

    return 0;
}
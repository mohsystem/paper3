#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <ctype.h>

// This program is designed for POSIX-compliant systems (like Linux, macOS)
// and must be run with root privileges (e.g., `sudo ./your_executable`).
// It will not work on Windows.

/**
 * Validates a username against a safe pattern.
 * @param username The username to validate.
 * @return 1 if valid, 0 otherwise.
 */
int isUsernameValid(const char* username) {
    if (username == NULL || strlen(username) == 0 || strlen(username) > 31) {
        return 0;
    }
    if (!isalpha(username[0]) && username[0] != '_') {
        return 0; // Must start with a letter or underscore
    }
    for (size_t i = 0; i < strlen(username); ++i) {
        if (!isalnum(username[i]) && username[i] != '_' && username[i] != '-') {
            return 0; // Can only contain alphanumeric, underscore, or hyphen
        }
    }
    return 1;
}

/**
 * Simulates changing a user's password. Requires root.
 * @param username The user whose password will be changed.
 * @param newPassword The new password.
 * @return 0 on success, -1 on failure.
 */
int changeUserPassword(const char* username, const char* newPassword) {
    if (geteuid() != 0) {
        fprintf(stderr, "Error: This operation requires root privileges.\n");
        return -1;
    }

    if (!isUsernameValid(username)) {
        fprintf(stderr, "Error: Invalid username format for '%s'.\n", username);
        return -1;
    }
    if (newPassword == NULL || strlen(newPassword) == 0) {
        fprintf(stderr, "Error: Password cannot be empty.\n");
        return -1;
    }

    printf("[PRIVILEGED] Simulating password change for user '%s'...\n", username);
    // In a real scenario, use fork() and execvp() to call `passwd` securely.
    printf("[PRIVILEGED] Password for '%s' successfully changed.\n", username);
    return 0;
}

/**
 * Drops root privileges to the original user who invoked sudo.
 * @return 0 on success, -1 on failure.
 */
int dropPrivileges() {
    if (geteuid() != 0) {
        printf("Not running as root. No privileges to drop.\n");
        return 0; // Not a failure if not root
    }

    uid_t target_uid = -1;
    gid_t target_gid = -1;

    const char* sudo_uid_str = getenv("SUDO_UID");
    const char* sudo_gid_str = getenv("SUDO_GID");

    if (sudo_uid_str == NULL || sudo_gid_str == NULL) {
        fprintf(stderr, "Error: SUDO_UID or SUDO_GID not set. Cannot determine which user to drop to.\n");
        return -1;
    }
    
    errno = 0; // To distinguish between error and valid '0' result
    target_uid = (uid_t)strtol(sudo_uid_str, NULL, 10);
    if (errno != 0) {
         perror("strtol for SUDO_UID");
         return -1;
    }
    target_gid = (gid_t)strtol(sudo_gid_str, NULL, 10);
    if (errno != 0) {
         perror("strtol for SUDO_GID");
         return -1;
    }

    printf("\nDropping privileges to UID=%u, GID=%u...\n", target_uid, target_gid);

    // The order is important: drop group privileges first.
    if (setgid(target_gid) != 0) {
        perror("Error: Failed to set GID");
        return -1;
    }

    if (setuid(target_uid) != 0) {
        perror("Error: Failed to set UID");
        return -1;
    }
    
    printf("Successfully dropped privileges.\n");
    return 0;
}

/**
 * Attempts to read a root-only file to verify privilege drop.
 */
void attemptReadShadow() {
    printf("\nAttempting to read /etc/shadow as non-privileged user...\n");
    FILE *shadow_file = fopen("/etc/shadow", "r");
    if (shadow_file) {
        printf("Success: Was able to read /etc/shadow. Privileges were NOT dropped correctly.\n");
        fclose(shadow_file);
    } else {
        printf("Success: Permission denied as expected. Privileges dropped correctly.\n");
    }
}

int main() {
    if (geteuid() != 0) {
        fprintf(stderr, "This program must be run as root (use sudo).\n");
        return 1;
    }
    
    printf("Script started with UID=%u GID=%u\n", geteuid(), getegid());
    
    printf("\n--- Running 5 Privileged Test Cases ---\n");
    changeUserPassword("testuser1", "Password123!");
    changeUserPassword("app_service", "SecurePass!@#");
    changeUserPassword("invalid;user", "password"); // Should fail validation
    changeUserPassword("testuser2", ""); // Should fail validation
    changeUserPassword("another_user", "GoodPa$$w0rd");

    printf("\n--- All privileged operations complete ---\n");

    if (dropPrivileges() == 0) {
        printf("Script now running with UID=%u GID=%u\n", geteuid(), getegid());
        attemptReadShadow();
    } else {
        fprintf(stderr, "Exiting due to failure in dropping privileges.\n");
        return 1;
    }
    
    return 0;
}
#include <stdio.h>
#include <stdlib.h>    // for getenv, strtol
#include <unistd.h>    // for geteuid, setuid, setgid
#include <sys/types.h> // for uid_t, gid_t

/**
 * @brief Simulates a privileged operation.
 * Checks the effective user ID to determine if it can proceed.
 * @param username The user to update.
 * @return 1 if successful (is root), 0 otherwise.
 */
int simulatePasswordChange(const char* username) {
    if (geteuid() == 0) {
        printf("PRIVILEGED: Successfully changed password for user '%s'.\n", username);
        return 1;
    } else {
        printf("FAILED: Insufficient privileges to change password for user '%s'.\n", username);
        return 0;
    }
}

/**
 * @brief Performs a privileged action, drops privileges, and then attempts the action again.
 * @param username The user whose password we are pretending to change.
 * @param newPassword The new password (unused in this simulation).
 */
void dropPrivilegesAndRun(const char* username, const char* newPassword) {
    printf("--- Starting process for user '%s' ---\n", username);
    
    uid_t initial_euid = geteuid();
    printf("Initial Effective User ID: %u\n", initial_euid);

    if (initial_euid != 0) {
        fprintf(stderr, "Error: This program must be run as root (or with sudo).\n");
        fprintf(stderr, "Cannot perform privileged operations or drop privileges.\n");
        printf("--- Process finished ---\n\n");
        return;
    }

    // 1. Perform privileged operation
    printf("\nStep 1: Performing action with root privileges.\n");
    simulatePasswordChange(username);

    // 2. Drop privileges
    printf("\nStep 2: Dropping root privileges.\n");
    const char* sudo_uid_str = getenv("SUDO_UID");
    const char* sudo_gid_str = getenv("SUDO_GID");

    if (sudo_uid_str == NULL || sudo_gid_str == NULL) {
        fprintf(stderr, "Error: SUDO_UID or SUDO_GID not found. Cannot drop privileges.\n");
        printf("--- Process finished ---\n\n");
        return;
    }

    uid_t target_uid = (uid_t)strtol(sudo_uid_str, NULL, 10);
    gid_t target_gid = (gid_t)strtol(sudo_gid_str, NULL, 10);
    
    printf("Dropping to UID=%u, GID=%u\n", target_uid, target_gid);

    // IMPORTANT: Set group first, then user.
    if (setgid(target_gid) != 0) {
        perror("setgid failed");
        return;
    }
    if (setuid(target_uid) != 0) {
        perror("setuid failed");
        return;
    }

    uid_t final_euid = geteuid();
    printf("Privileges dropped. New Effective User ID: %u\n", final_euid);
    if (final_euid == 0) {
        printf("WARNING: Failed to drop privileges.\n");
    } else {
        printf("SUCCESS: Privileges dropped successfully.\n");
    }

    // 3. Attempt privileged operation again
    printf("\nStep 3: Attempting the same action without root privileges.\n");
    simulatePasswordChange(username);

    printf("--- Process finished ---\n\n");
}

int main() {
    printf("Running Privilege Drop Demonstration in C\n");
    printf("NOTE: This program must be executed with 'sudo'.\n\n");

    const char* users_to_update[] = {"alice", "bob", "charlie", "david", "eve"};
    int num_users = sizeof(users_to_update) / sizeof(users_to_update[0]);

    for (int i = 0; i < num_users; ++i) {
        printf("--- Test Case %d ---\n", i + 1);
        dropPrivilegesAndRun(users_to_update[i], "new_secure_password_123");
    }

    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <ctype.h>

/**
 * Validates if a username is safe (alphanumeric, dash, underscore).
 * @param username The username string to validate.
 * @return 1 if valid, 0 otherwise.
 */
int is_valid_username(const char* username) {
    if (username == NULL || strlen(username) == 0 || strlen(username) > 32) {
        return 0;
    }
    for (size_t i = 0; i < strlen(username); ++i) {
        if (!isalnum(username[i]) && username[i] != '_' && username[i] != '-') {
            return 0;
        }
    }
    return 1;
}


/**
 * Simulates changing a user's password. This is a privileged operation.
 * @param username The OS user to modify.
 * @param new_password The new password (not used in simulation).
 * @return 0 on success, -1 on failure.
 */
int change_user_password(const char* username, const char* new_password) {
    if (geteuid() != 0) {
        fprintf(stderr, "Error: Must be root to change user passwords.\n");
        return -1;
    }

    if (!is_valid_username(username)) {
        fprintf(stderr, "Error: Invalid username format.\n");
        return -1;
    }

    printf("[PRIVILEGED] Simulating password change for user '%s'.\n", username);
    // In a real implementation, you would securely call a utility like 'chpasswd'.
    printf("[PRIVILEGED] Privileged operation complete.\n");
    return 0;
}

/**
 * Drops root privileges to a less-privileged user.
 * @param username The user to switch to.
 * @return 0 on success, -1 on failure.
 */
int drop_privileges(const char* username) {
    if (geteuid() != 0) {
        printf("Not running as root, no privileges to drop.\n");
        return 0; // Not an error in this context
    }
    
    if (!is_valid_username(username)) {
        fprintf(stderr, "Error: Invalid username format for privilege drop.\n");
        return -1;
    }

    struct passwd *pw = getpwnam(username);
    if (pw == NULL) {
        fprintf(stderr, "Error: User '%s' not found.\n", username);
        return -1;
    }

    uid_t target_uid = pw->pw_uid;
    gid_t target_gid = pw->pw_gid;

    printf("--- Dropping privileges to user '%s' (uid=%d, gid=%d) ---\n", username, target_uid, target_gid);

    // Initialize supplementary groups. This is a critical security step.
    if (initgroups(username, target_gid) != 0) {
        perror("initgroups() failed");
        return -1;
    }

    // Set the Group ID first.
    if (setgid(target_gid) != 0) {
        perror("setgid() failed");
        return -1;
    }

    // Finally, set the User ID. This is an irreversible operation for this process.
    if (setuid(target_uid) != 0) {
        perror("setuid() failed");
        return -1;
    }
    
    printf("Privileges dropped. New effective UID: %d\n", geteuid());
    return 0;
}

void run_test_case(int case_num, const char* user, const char* pass, const char* drop_user) {
    printf("--- Test Case %d: Change pass for '%s', drop to '%s' ---\n", case_num, user, drop_user);
    
    if (geteuid() != 0) {
        printf("Cannot run test, not running as root.\n");
        return;
    }

    printf("Initial effective UID: %d\n", geteuid());

    // 1. Perform privileged action
    if (change_user_password(user, pass) != 0) {
        fprintf(stderr, "Privileged operation failed for user '%s'.\n", user);
        goto end_case;
    }

    // 2. Drop privileges
    if (drop_privileges(drop_user) != 0) {
        fprintf(stderr, "Failed to drop privileges to '%s'.\n", drop_user);
        goto end_case;
    }

    // 3. Attempt privileged action again (should fail)
    printf("\nAttempting privileged action after dropping privileges...\n");
    if (change_user_password("anotheruser", "somepass") != 0) {
        printf("Operation failed as expected after dropping privileges.\n");
    } else {
        printf("Error: Operation should have failed but it succeeded.\n");
    }

end_case:
    if (geteuid() != 0) {
        printf("\nExiting because privileges have been dropped successfully.\n");
        exit(0);
    }
    printf("--------------------------------------------------------\n\n");
}


int main() {
    printf("--- Privilege Drop Demonstration (C) ---\n");
    printf("NOTE: This program must be run with 'sudo' to demonstrate the full effect.\n\n");
    
    const char* drop_to_user = "nobody";

    run_test_case(1, "testuser1", "Pass1!", drop_to_user);
    run_test_case(2, "app_user", "SecurePass_2", drop_to_user);
    run_test_case(3, ";reboot", "bad", drop_to_user); // Invalid user
    run_test_case(4, "testuser3", "Pass3", "nonexistentuser"); // Invalid drop user
    run_test_case(5, "another_user", "Pass4", drop_to_user);

    return 0;
}
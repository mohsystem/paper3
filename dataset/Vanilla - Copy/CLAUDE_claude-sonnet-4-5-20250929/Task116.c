
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
    #include <windows.h>
    #include <lmaccess.h>
#else
    #include <unistd.h>
    #include <sys/types.h>
#endif

int hasElevatedPrivileges() {
    #ifdef _WIN32
        // Windows: Check if running as administrator
        BOOL isAdmin = FALSE;
        SID_IDENTIFIER_AUTHORITY NtAuthority = {SECURITY_NT_AUTHORITY};
        PSID AdministratorsGroup;
        
        if (AllocateAndInitializeSid(&NtAuthority, 2,
            SECURITY_BUILTIN_DOMAIN_RID,
            DOMAIN_ALIAS_RID_ADMINS,
            0, 0, 0, 0, 0, 0,
            &AdministratorsGroup)) {
            CheckTokenMembership(NULL, AdministratorsGroup, &isAdmin);
            FreeSid(AdministratorsGroup);
        }
        return isAdmin != 0;
    #else
        // Linux/Unix: Check if running as root
        return geteuid() == 0;
    #endif
}

void dropPrivileges() {
    printf("Dropping privileges...\\n");
    
    #ifdef _WIN32
        // Windows: Cannot easily drop privileges
        printf("Note: Privilege dropping on Windows requires process termination\\n");
        printf("Simulating privilege drop by restricting further operations\\n");
    #else
        // Linux/Unix: Drop to regular user
        if (geteuid() == 0) {
            char* sudo_uid_str = getenv("SUDO_UID");
            char* sudo_gid_str = getenv("SUDO_GID");
            
            if (sudo_uid_str && sudo_gid_str) {
                uid_t sudo_uid = atoi(sudo_uid_str);
                gid_t sudo_gid = atoi(sudo_gid_str);
                
                if (setgid(sudo_gid) == 0 && setuid(sudo_uid) == 0) {
                    printf("Privileges dropped to UID: %d, GID: %d\\n", sudo_uid, sudo_gid);
                } else {
                    printf("Failed to drop privileges\\n");
                }
            } else {
                printf("Running as root but no SUDO_UID available\\n");
            }
        } else {
            printf("Not running with elevated privileges\\n");
        }
    #endif
}

int changeUserPassword(const char* username, const char* newPassword) {
    // Check if running with elevated privileges
    if (!hasElevatedPrivileges()) {
        printf("Error: Insufficient privileges to change password\\n");
        return 0;
    }
    
    // Perform privileged operation
    printf("Performing privileged operation: Changing password for user: %s\\n", username);
    
    char cmd[512];
    int result;
    
    #ifdef _WIN32
        // Windows implementation
        snprintf(cmd, sizeof(cmd), "net user %s %s", username, newPassword);
        result = system(cmd);
    #else
        // Linux/Unix implementation
        snprintf(cmd, sizeof(cmd), "echo '%s:%s' | chpasswd", username, newPassword);
        result = system(cmd);
    #endif
    
    if (result == 0) {
        printf("Password changed successfully for user: %s\\n", username);
        
        // Drop privileges after privileged operation
        dropPrivileges();
        
        return 1;
    } else {
        printf("Failed to change password. Exit code: %d\\n", result);
        return 0;
    }
}

int main() {
    printf("=== Privilege Management Test Cases ===\\n\\n");
    
    // Test Case 1: Valid user password change
    printf("Test Case 1: Attempting to change password for testuser1\\n");
    changeUserPassword("testuser1", "NewPass123!");
    printf("\\n");
    
    // Test Case 2: Another user password change
    printf("Test Case 2: Attempting to change password for testuser2\\n");
    changeUserPassword("testuser2", "SecurePass456!");
    printf("\\n");
    
    // Test Case 3: Different user
    printf("Test Case 3: Attempting to change password for adminuser\\n");
    changeUserPassword("adminuser", "AdminPass789!");
    printf("\\n");
    
    // Test Case 4: Special characters in password
    printf("Test Case 4: Attempting to change password with special characters\\n");
    changeUserPassword("testuser3", "P@ssw0rd#Special!");
    printf("\\n");
    
    // Test Case 5: Empty username
    printf("Test Case 5: Attempting to change password with empty username\\n");
    changeUserPassword("", "Password123!");
    printf("\\n");
    
    return 0;
}

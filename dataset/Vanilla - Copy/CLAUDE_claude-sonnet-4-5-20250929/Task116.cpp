
#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>

#ifdef _WIN32
    #include <windows.h>
    #include <lmaccess.h>
    #pragma comment(lib, "netapi32.lib")
#else
    #include <unistd.h>
    #include <sys/types.h>
    #include <pwd.h>
    #include <shadow.h>
#endif

class Task116 {
public:
    static bool changeUserPassword(const std::string& username, const std::string& newPassword) {
        try {
            // Check if running with elevated privileges
            if (!hasElevatedPrivileges()) {
                std::cout << "Error: Insufficient privileges to change password" << std::endl;
                return false;
            }
            
            // Perform privileged operation
            std::cout << "Performing privileged operation: Changing password for user: " << username << std::endl;
            
            #ifdef _WIN32
                // Windows implementation
                std::string cmd = "net user " + username + " " + newPassword;
                int result = system(cmd.c_str());
            #else
                // Linux/Unix implementation
                std::string cmd = "echo '" + username + ":" + newPassword + "' | chpasswd";
                int result = system(cmd.c_str());
            #endif
            
            if (result == 0) {
                std::cout << "Password changed successfully for user: " << username << std::endl;
                
                // Drop privileges after privileged operation
                dropPrivileges();
                
                return true;
            } else {
                std::cout << "Failed to change password. Exit code: " << result << std::endl;
                return false;
            }
            
        } catch (const std::exception& e) {
            std::cout << "Error changing password: " << e.what() << std::endl;
            return false;
        }
    }
    
private:
    static bool hasElevatedPrivileges() {
        #ifdef _WIN32
            // Windows: Check if running as administrator
            BOOL isAdmin = FALSE;
            SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
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
    
    static void dropPrivileges() {
        std::cout << "Dropping privileges..." << std::endl;
        
        #ifdef _WIN32
            // Windows: Cannot easily drop privileges
            std::cout << "Note: Privilege dropping on Windows requires process termination" << std::endl;
            std::cout << "Simulating privilege drop by restricting further operations" << std::endl;
        #else
            // Linux/Unix: Drop to regular user
            if (geteuid() == 0) {
                const char* sudo_uid_str = getenv("SUDO_UID");
                const char* sudo_gid_str = getenv("SUDO_GID");
                
                if (sudo_uid_str && sudo_gid_str) {
                    uid_t sudo_uid = atoi(sudo_uid_str);
                    gid_t sudo_gid = atoi(sudo_gid_str);
                    
                    if (setgid(sudo_gid) == 0 && setuid(sudo_uid) == 0) {
                        std::cout << "Privileges dropped to UID: " << sudo_uid << ", GID: " << sudo_gid << std::endl;
                    } else {
                        std::cout << "Failed to drop privileges" << std::endl;
                    }
                } else {
                    std::cout << "Running as root but no SUDO_UID available" << std::endl;
                }
            } else {
                std::cout << "Not running with elevated privileges" << std::endl;
            }
        #endif
    }
};

int main() {
    std::cout << "=== Privilege Management Test Cases ===" << std::endl << std::endl;
    
    // Test Case 1: Valid user password change
    std::cout << "Test Case 1: Attempting to change password for testuser1" << std::endl;
    Task116::changeUserPassword("testuser1", "NewPass123!");
    std::cout << std::endl;
    
    // Test Case 2: Another user password change
    std::cout << "Test Case 2: Attempting to change password for testuser2" << std::endl;
    Task116::changeUserPassword("testuser2", "SecurePass456!");
    std::cout << std::endl;
    
    // Test Case 3: Different user
    std::cout << "Test Case 3: Attempting to change password for adminuser" << std::endl;
    Task116::changeUserPassword("adminuser", "AdminPass789!");
    std::cout << std::endl;
    
    // Test Case 4: Special characters in password
    std::cout << "Test Case 4: Attempting to change password with special characters" << std::endl;
    Task116::changeUserPassword("testuser3", "P@ssw0rd#Special!");
    std::cout << std::endl;
    
    // Test Case 5: Empty username
    std::cout << "Test Case 5: Attempting to change password with empty username" << std::endl;
    Task116::changeUserPassword("", "Password123!");
    std::cout << std::endl;
    
    return 0;
}

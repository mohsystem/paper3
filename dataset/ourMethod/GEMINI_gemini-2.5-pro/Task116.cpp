#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <regex>

// POSIX/Linux headers
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>

class PrivilegeManager {
public:
    /**
     * Simulates changing a user's password. This is a privileged operation.
     * @param username The OS user to modify.
     * @param newPassword The new password (not used in simulation).
     */
    void changeUserPassword(const std::string& username, const std::string& newPassword) {
        if (geteuid() != 0) {
            throw std::runtime_error("Must be root to change user passwords.");
        }
        
        // Basic validation to prevent command injection if this were calling a shell.
        if (!std::regex_match(username, std::regex("^[a-zA-Z0-9_-]{1,32}$"))) {
            throw std::invalid_argument("Invalid username format.");
        }

        std::cout << "[PRIVILEGED] Simulating password change for user '" << username << "'." << std::endl;
        // In a real implementation, you would securely call a utility like 'chpasswd'.
        std::cout << "[PRIVILEGED] Privileged operation complete." << std::endl;
    }

    /**
     * Drops root privileges to a less-privileged user.
     * @param username The user to switch to.
     */
    void dropPrivileges(const std::string& username) {
        if (geteuid() != 0) {
            std::cout << "Not running as root, no privileges to drop." << std::endl;
            return;
        }

        if (!std::regex_match(username, std::regex("^[a-zA-Z0-9_-]{1,32}$"))) {
            throw std::invalid_argument("Invalid username format for privilege drop.");
        }

        struct passwd *pw = getpwnam(username.c_str());
        if (!pw) {
            throw std::runtime_error("User '" + username + "' not found.");
        }

        uid_t target_uid = pw->pw_uid;
        gid_t target_gid = pw->pw_gid;

        std::cout << "--- Dropping privileges to user '" << username << "' (uid=" << target_uid << ", gid=" << target_gid << ") ---" << std::endl;

        // Initialize supplementary groups. This is a critical security step.
        if (initgroups(username.c_str(), target_gid) != 0) {
            throw std::runtime_error("initgroups() failed.");
        }

        // Set the Group ID first.
        if (setgid(target_gid) != 0) {
            throw std::runtime_error("setgid() failed.");
        }

        // Finally, set the User ID. This is an irreversible operation for this process.
        if (setuid(target_uid) != 0) {
            throw std::runtime_error("setuid() failed.");
        }
        
        std::cout << "Privileges dropped. New effective UID: " << geteuid() << std::endl;
    }
};

void run_test_case(int case_num, const std::string& user, const std::string& pass, const std::string& drop_user) {
    std::cout << "--- Test Case " << case_num << ": Change pass for '" << user << "', drop to '" << drop_user << "' ---" << std::endl;
    PrivilegeManager pm;
    
    try {
        if (geteuid() != 0) {
            std::cout << "Cannot run test, not running as root." << std::endl;
            return;
        }
        
        std::cout << "Initial effective UID: " << geteuid() << std::endl;

        // 1. Perform privileged action
        pm.changeUserPassword(user, pass);
        
        // 2. Drop privileges
        pm.dropPrivileges(drop_user);

        // 3. Attempt privileged action again (should fail)
        std::cout << "\nAttempting privileged action after dropping privileges..." << std::endl;
        pm.changeUserPassword("anotheruser", "somepass");

    } catch (const std::exception& e) {
        std::cerr << "Operation failed as expected: " << e.what() << std::endl;
    }
    
    // Check if we are still root. If not, we can't continue to the next test.
    if (geteuid() != 0) {
        std::cout << "\nExiting because privileges have been dropped successfully." << std::endl;
        exit(0);
    }
    
    std::cout << "--------------------------------------------------------\n" << std::endl;
}

int main() {
    std::cout << "--- Privilege Drop Demonstration (C++) ---" << std::endl;
    std::cout << "NOTE: This program must be run with 'sudo' to demonstrate the full effect.\n" << std::endl;

    const std::string drop_to_user = "nobody";

    run_test_case(1, "testuser1", "Pass1!", drop_to_user);
    run_test_case(2, "app_user", "SecurePass_2", drop_to_user);
    run_test_case(3, ";reboot", "bad", drop_to_user); // Invalid user
    run_test_case(4, "testuser3", "Pass3", "nonexistentuser"); // Invalid drop user
    run_test_case(5, "another_user", "Pass4", drop_to_user);

    return 0;
}
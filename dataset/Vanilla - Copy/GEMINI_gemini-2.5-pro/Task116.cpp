#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>    // for geteuid, setuid, setgid
#include <sys/types.h> // for uid_t, gid_t
#include <cstdlib>     // for getenv, strtol

/**
 * @brief Simulates a privileged operation.
 * Checks the effective user ID to determine if it can proceed.
 * @param username The user to update.
 * @return True if successful (is root), false otherwise.
 */
bool simulatePasswordChange(const std::string& username) {
    if (geteuid() == 0) {
        std::cout << "PRIVILEGED: Successfully changed password for user '" << username << "'." << std::endl;
        return true;
    } else {
        std::cout << "FAILED: Insufficient privileges to change password for user '" << username << "'." << std::endl;
        return false;
    }
}

/**
 * @brief Performs a privileged action, drops privileges, and then attempts the action again.
 * @param username The user whose password we are pretending to change.
 * @param newPassword The new password (unused in this simulation).
 */
void dropPrivilegesAndRun(const std::string& username, const std::string& newPassword) {
    std::cout << "--- Starting process for user '" << username << "' ---" << std::endl;
    
    uid_t initial_euid = geteuid();
    std::cout << "Initial Effective User ID: " << initial_euid << std::endl;

    if (initial_euid != 0) {
        std::cerr << "Error: This program must be run as root (or with sudo)." << std::endl;
        std::cerr << "Cannot perform privileged operations or drop privileges." << std::endl;
        std::cout << "--- Process finished ---\n" << std::endl;
        return;
    }

    // 1. Perform privileged operation
    std::cout << "\nStep 1: Performing action with root privileges." << std::endl;
    simulatePasswordChange(username);

    // 2. Drop privileges
    std::cout << "\nStep 2: Dropping root privileges." << std::endl;
    const char* sudo_uid_str = getenv("SUDO_UID");
    const char* sudo_gid_str = getenv("SUDO_GID");

    if (sudo_uid_str == nullptr || sudo_gid_str == nullptr) {
        std::cerr << "Error: SUDO_UID or SUDO_GID not found. Cannot drop privileges." << std::endl;
        std::cout << "--- Process finished ---\n" << std::endl;
        return;
    }

    uid_t target_uid = static_cast<uid_t>(strtol(sudo_uid_str, nullptr, 10));
    gid_t target_gid = static_cast<gid_t>(strtol(sudo_gid_str, nullptr, 10));

    std::cout << "Dropping to UID=" << target_uid << ", GID=" << target_gid << std::endl;

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
    std::cout << "Privileges dropped. New Effective User ID: " << final_euid << std::endl;
    if (final_euid == 0) {
        std::cout << "WARNING: Failed to drop privileges." << std::endl;
    } else {
        std::cout << "SUCCESS: Privileges dropped successfully." << std::endl;
    }

    // 3. Attempt privileged operation again
    std::cout << "\nStep 3: Attempting the same action without root privileges." << std::endl;
    simulatePasswordChange(username);
    
    std::cout << "--- Process finished ---\n" << std::endl;
}

class Task116 {
public:
    static void main() {
        std::cout << "Running Privilege Drop Demonstration in C++" << std::endl;
        std::cout << "NOTE: This program must be executed with 'sudo'.\n" << std::endl;
        
        std::vector<std::string> users_to_update = {"alice", "bob", "charlie", "david", "eve"};
        
        for (size_t i = 0; i < users_to_update.size(); ++i) {
            std::cout << "--- Test Case " << i + 1 << " ---" << std::endl;
            dropPrivilegesAndRun(users_to_update[i], "new_secure_password_123");
        }
    }
};

int main() {
    Task116::main();
    return 0;
}
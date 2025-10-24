#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/types.h>
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <fstream>
#include <regex>

// This program is designed for POSIX-compliant systems (like Linux, macOS)
// and must be run with root privileges (e.g., `sudo ./your_executable`).
// It will not work on Windows.

/**
 * Validates a username against a safe pattern.
 * @param username The username to validate.
 * @return True if valid, false otherwise.
 */
bool isUsernameValid(const std::string& username) {
    // Usernames should be alphanumeric, may contain _ and -
    static const std::regex pattern("^[a-zA-Z0-9_][a-zA-Z0-9_-]{0,30}$");
    return std::regex_match(username, pattern);
}

/**
 * Simulates changing a user's password. Requires root.
 * @param username The user whose password will be changed.
 * @param newPassword The new password.
 * @return True on success, false on failure.
 */
bool changeUserPassword(const std::string& username, const std::string& newPassword) {
    if (geteuid() != 0) {
        std::cerr << "Error: This operation requires root privileges." << std::endl;
        return false;
    }

    if (!isUsernameValid(username)) {
        std::cerr << "Error: Invalid username format for '" << username << "'." << std::endl;
        return false;
    }
    if (newPassword.empty()) {
        std::cerr << "Error: Password cannot be empty." << std::endl;
        return false;
    }

    std::cout << "[PRIVILEGED] Simulating password change for user '" << username << "'..." << std::endl;
    // In a real scenario, use execvp or similar to call `passwd` securely.
    // Avoid system() to prevent shell injection.
    std::cout << "[PRIVILEGED] Password for '" << username << "' successfully changed." << std::endl;
    return true;
}

/**
 * Drops root privileges to the original user who invoked sudo.
 * @return True on success, false on failure.
 */
bool dropPrivileges() {
    if (geteuid() != 0) {
        std::cout << "Not running as root. No privileges to drop." << std::endl;
        return true; // Not a failure case if not root to begin with
    }

    uid_t target_uid = -1;
    gid_t target_gid = -1;

    const char* sudo_uid_str = getenv("SUDO_UID");
    const char* sudo_gid_str = getenv("SUDO_GID");

    if (sudo_uid_str == nullptr || sudo_gid_str == nullptr) {
        std::cerr << "Error: SUDO_UID or SUDO_GID not set. Cannot determine which user to drop to." << std::endl;
        return false;
    }

    target_uid = static_cast<uid_t>(std::stoul(sudo_uid_str));
    target_gid = static_cast<gid_t>(std::stoul(sudo_gid_str));

    std::cout << "\nDropping privileges to UID=" << target_uid << ", GID=" << target_gid << "..." << std::endl;

    // The order is important: drop group privileges first.
    if (setgid(target_gid) != 0) {
        std::cerr << "Error: Failed to set GID: " << strerror(errno) << std::endl;
        return false;
    }

    if (setuid(target_uid) != 0) {
        std::cerr << "Error: Failed to set UID: " << strerror(errno) << std::endl;
        return false;
    }
    
    std::cout << "Successfully dropped privileges." << std::endl;
    return true;
}

/**
 * Attempts to read a root-only file to verify privilege drop.
 */
void attemptReadShadow() {
    std::cout << "\nAttempting to read /etc/shadow as non-privileged user..." << std::endl;
    std::ifstream shadow_file("/etc/shadow");
    if (shadow_file.is_open()) {
        std::cout << "Success: Was able to read /etc/shadow. Privileges were NOT dropped correctly." << std::endl;
        shadow_file.close();
    } else {
        std::cout << "Success: Permission denied as expected. Privileges dropped correctly." << std::endl;
    }
}

int main() {
    if (geteuid() != 0) {
        std::cerr << "This program must be run as root (use sudo)." << std::endl;
        return 1;
    }

    std::cout << "Script started with UID=" << geteuid() << " GID=" << getegid() << std::endl;

    std::cout << "\n--- Running 5 Privileged Test Cases ---" << std::endl;
    changeUserPassword("testuser1", "Password123!");
    changeUserPassword("app_service", "SecurePass!@#");
    changeUserPassword("invalid;user", "password"); // Should fail validation
    changeUserPassword("testuser2", ""); // Should fail validation
    changeUserPassword("another_user", "GoodPa$$w0rd");

    std::cout << "\n--- All privileged operations complete ---" << std::endl;

    if (dropPrivileges()) {
        std::cout << "Script now running with UID=" << geteuid() << " GID=" << getegid() << std::endl;
        attemptReadShadow();
    } else {
        std::cerr << "Exiting due to failure in dropping privileges." << std::endl;
        return 1;
    }

    return 0;
}
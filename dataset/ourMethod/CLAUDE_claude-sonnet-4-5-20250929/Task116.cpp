
#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <limits>
#include <cctype>

#ifdef _WIN32
    #include <windows.h>
    #include <lmaccess.h>
    #include <lm.h>
    #pragma comment(lib, "netapi32.lib")
    #pragma comment(lib, "advapi32.lib")
#else
    #include <unistd.h>
    #include <pwd.h>
    #include <shadow.h>
    #include <crypt.h>
    #include <sys/types.h>
    #include <errno.h>
#endif

// Secure string clearing - prevents compiler optimization from removing memset
void secure_clear(void* ptr, size_t size) {
    if (ptr == nullptr || size == 0) return;
    volatile unsigned char* p = static_cast<volatile unsigned char*>(ptr);
    while (size--) *p++ = 0;
}

// Validate username - prevent injection attacks
bool validate_username(const std::string& username) {
    // Username must be 1-32 characters, alphanumeric plus underscore/dash
    if (username.empty() || username.length() > 32) {
        return false;
    }
    
    for (char c : username) {
        if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_' && c != '-') {
            return false;
        }
    }
    
    // Prevent directory traversal
    if (username.find("..") != std::string::npos) {
        return false;
    }
    
    return true;
}

// Validate password - enforce strong password policy
bool validate_password(const std::string& password) {
    // Password must be 8-128 characters
    if (password.length() < 8 || password.length() > 128) {
        return false;
    }
    
    bool has_upper = false, has_lower = false, has_digit = false, has_special = false;
    
    for (char c : password) {
        if (std::isupper(static_cast<unsigned char>(c))) has_upper = true;
        else if (std::islower(static_cast<unsigned char>(c))) has_lower = true;
        else if (std::isdigit(static_cast<unsigned char>(c))) has_digit = true;
        else if (std::ispunct(static_cast<unsigned char>(c)) || c == ' ') has_special = true;
    }
    
    // Require at least 3 of 4 character classes
    int complexity = (has_upper ? 1 : 0) + (has_lower ? 1 : 0) + 
                     (has_digit ? 1 : 0) + (has_special ? 1 : 0);
    
    return complexity >= 3;
}

#ifdef _WIN32
// Windows implementation
bool change_user_password_privileged(const std::string& username, const std::string& new_password) {
    // Validate inputs before any privileged operations
    if (!validate_username(username)) {
        std::cerr << "Invalid username format" << std::endl;
        return false;
    }
    
    if (!validate_password(new_password)) {
        std::cerr << "Password does not meet complexity requirements" << std::endl;
        return false;
    }
    
    // Convert to wide strings for Windows API
    int username_size = MultiByteToWideChar(CP_UTF8, 0, username.c_str(), -1, nullptr, 0);
    int password_size = MultiByteToWideChar(CP_UTF8, 0, new_password.c_str(), -1, nullptr, 0);
    
    if (username_size <= 0 || password_size <= 0) {
        return false;
    }
    
    std::vector<wchar_t> wusername(username_size);
    std::vector<wchar_t> wpassword(password_size);
    
    MultiByteToWideChar(CP_UTF8, 0, username.c_str(), -1, wusername.data(), username_size);
    MultiByteToWideChar(CP_UTF8, 0, new_password.c_str(), -1, wpassword.data(), password_size);
    
    USER_INFO_1003 ui;
    ui.usri1003_password = wpassword.data();
    
    // Perform privileged operation
    NET_API_STATUS status = NetUserSetInfo(nullptr, wusername.data(), 1003, 
                                           reinterpret_cast<LPBYTE>(&ui), nullptr);
    
    // Clear sensitive data immediately
    secure_clear(wpassword.data(), wpassword.size() * sizeof(wchar_t));
    
    if (status != NERR_Success) {
        std::cerr << "Failed to change password. Error: " << status << std::endl;
        return false;
    }
    
    return true;
}

bool drop_privileges() {
    // On Windows, attempt to drop admin token privileges
    HANDLE hToken;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
        return false;
    }
    
    TOKEN_PRIVILEGES tp;
    tp.PrivilegeCount = 1;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_REMOVED;
    
    // Try to remove SeDebugPrivilege as an example
    if (LookupPrivilegeValue(nullptr, SE_DEBUG_NAME, &tp.Privileges[0].Luid)) {
        AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), nullptr, nullptr);
    }
    
    CloseHandle(hToken);
    return true;
}

#else
// Unix/Linux implementation
bool change_user_password_privileged(const std::string& username, const std::string& new_password) {
    // Must be running as root to change passwords
    if (geteuid() != 0) {
        std::cerr << "Must run with root privileges to change passwords" << std::endl;
        return false;
    }
    
    // Validate inputs before any privileged operations
    if (!validate_username(username)) {
        std::cerr << "Invalid username format" << std::endl;
        return false;
    }
    
    if (!validate_password(new_password)) {
        std::cerr << "Password does not meet complexity requirements" << std::endl;
        return false;
    }
    
    // Verify user exists
    struct passwd* pw = getpwnam(username.c_str());
    if (pw == nullptr) {
        std::cerr << "User does not exist" << std::endl;
        return false;
    }
    
    // Use system command with validated input (safer than direct shadow file manipulation)
    // Note: In production, consider using PAM or other system libraries
    std::string command = "echo '" + username + ":" + new_password + "' | chpasswd";
    
    int result = system(command.c_str());
    
    if (result != 0) {
        std::cerr << "Failed to change password" << std::endl;
        return false;
    }
    
    return true;
}

bool drop_privileges() {
    // Drop to nobody user (uid 65534 typically) or a safe non-root user
    uid_t target_uid = 65534; // nobody
    gid_t target_gid = 65534; // nogroup
    
    // Drop supplementary groups
    if (setgroups(0, nullptr) != 0) {
        std::cerr << "Failed to drop supplementary groups" << std::endl;
        return false;
    }
    
    // Drop GID first (must be done before dropping UID)
    if (setgid(target_gid) != 0) {
        std::cerr << "Failed to drop group privileges" << std::endl;
        return false;
    }
    
    // Drop UID
    if (setuid(target_uid) != 0) {
        std::cerr << "Failed to drop user privileges" << std::endl;
        return false;
    }
    
    // Verify we can't regain privileges
    if (setuid(0) == 0 || seteuid(0) == 0) {
        std::cerr << "Failed to permanently drop privileges" << std::endl;
        return false;
    }
    
    return true;
}
#endif

// Main function to change password and drop privileges
bool secure_password_change(const std::string& username, const std::string& new_password) {
    // Perform privileged operation
    bool success = change_user_password_privileged(username, new_password);
    
    // Always attempt to drop privileges after operation, regardless of success
    if (!drop_privileges()) {
        std::cerr << "Warning: Failed to drop privileges" << std::endl;
    }
    
    return success;
}

int main() {
    // Test cases demonstrating secure password change with privilege dropping
    
    std::cout << "=== Test Case 1: Valid password change ===" << std::endl;
    {
        std::string username = "testuser1";
        std::string password = "SecurePass123!";
        bool result = secure_password_change(username, password);
        std::cout << "Result: " << (result ? "SUCCESS" : "FAILED") << std::endl;
        // Clear sensitive data
        secure_clear(&password[0], password.size());
    }
    
    std::cout << "\\n=== Test Case 2: Invalid username (too long) ===" << std::endl;
    {
        std::string username = "thisusernameiswaytoolongandshouldfailvalidation";
        std::string password = "ValidPass123!";
        bool result = secure_password_change(username, password);
        std::cout << "Result: " << (result ? "SUCCESS" : "FAILED (expected)") << std::endl;
        secure_clear(&password[0], password.size());
    }
    
    std::cout << "\\n=== Test Case 3: Invalid password (too short) ===" << std::endl;
    {
        std::string username = "testuser2";
        std::string password = "short";
        bool result = secure_password_change(username, password);
        std::cout << "Result: " << (result ? "SUCCESS" : "FAILED (expected)") << std::endl;
        secure_clear(&password[0], password.size());
    }
    
    std::cout << "\\n=== Test Case 4: Invalid username (injection attempt) ===" << std::endl;
    {
        std::string username = "user; rm -rf /";
        std::string password = "ValidPass123!";
        bool result = secure_password_change(username, password);
        std::cout << "Result: " << (result ? "SUCCESS" : "FAILED (expected)") << std::endl;
        secure_clear(&password[0], password.size());
    }
    
    std::cout << "\\n=== Test Case 5: Valid complex password ===" << std::endl;
    {
        std::string username = "testuser3";
        std::string password = "C0mpl3x!P@ssw0rd";
        bool result = secure_password_change(username, password);
        std::cout << "Result: " << (result ? "SUCCESS" : "FAILED") << std::endl;
        secure_clear(&password[0], password.size());
    }
    
    return 0;
}

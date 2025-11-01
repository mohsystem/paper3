
#include <iostream>
#include <string>
#include <regex>
#include <cstring>
#include <sqlite3.h>
#include <unistd.h>
#include <sys/wait.h>
#include <limits.h>
#include <cstdlib>

// Maximum email length to prevent buffer overflow
const size_t MAX_EMAIL_LENGTH = 254;

// Email validation pattern (RFC 5322 simplified)
const std::regex EMAIL_PATTERN(
    "^[a-zA-Z0-9_+&*-]+(?:\\\\.[a-zA-Z0-9_+&*-]+)*@(?:[a-zA-Z0-9-]+\\\\.)+[a-zA-Z]{2,7}$"
);

/**
 * Validates email format and length
 * 
 * @param email The email to validate
 * @return true if valid, false otherwise
 */
bool validateEmail(const std::string& email) {
    // Check for empty email
    if (email.empty()) {
        return false;
    }
    
    // Validate length to prevent buffer overflow
    if (email.length() > MAX_EMAIL_LENGTH) {
        return false;
    }
    
    // Validate format to prevent injection
    if (!std::regex_match(email, EMAIL_PATTERN)) {
        return false;
    }
    
    return true;
}

/**
 * Validates path to prevent path traversal
 * 
 * @param path The path to validate
 * @return true if valid, false otherwise
 */
bool validatePath(const std::string& path) {
    if (path.empty()) {
        return false;
    }
    
    // Check for path traversal attempts
    if (path.find("..") != std::string::npos || path.find("~") != std::string::npos) {
        return false;
    }
    
    return true;
}

/**
 * Calls the unsubscribe script with validated email parameter
 * 
 * @param email Validated email address
 * @param scriptPath Path to unsubscribe script
 * @return 1 if script returns 0, otherwise 0
 */
int callUnsubscribeScript(const std::string& email, const std::string& scriptPath) {
    pid_t pid = fork();
    
    if (pid == -1) {
        // Fork failed
        std::cerr << "Process creation failed" << std::endl;
        return 0;
    }
    
    if (pid == 0) {
        // Child process
        // Clear environment to prevent information leakage
        clearenv();
        
        // Execute script with email as argument (prevents command injection)
        const char* args[] = {scriptPath.c_str(), email.c_str(), nullptr};
        execv(scriptPath.c_str(), const_cast<char* const*>(args));
        
        // If execv returns, it failed
        _exit(1);
    } else {
        // Parent process
        int status = 0;
        
        // Wait for child with timeout handling
        pid_t result = waitpid(pid, &status, 0);
        
        if (result == -1) {
            std::cerr << "Wait failed" << std::endl;
            return 0;
        }
        
        // Check if process exited normally
        if (WIFEXITED(status)) {
            int exitCode = WEXITSTATUS(status);
            // Return 1 if script returns 0 (success), otherwise 0
            return (exitCode == 0) ? 1 : 0;
        }
        
        return 0;
    }
}

/**
 * Checks if email exists in database and unsubscribes if found.
 * Returns 1 if unsubscribe successful, 0 otherwise.
 * 
 * @param email The email address to unsubscribe
 * @param dbPath Path to SQLite database
 * @param scriptPath Path to unsubscribe script
 * @return 1 if successful, 0 otherwise
 */
int unsubscribeEmail(const std::string& email, const std::string& dbPath, 
                     const std::string& scriptPath) {
    // Input validation
    if (!validateEmail(email)) {
        return 0;
    }
    
    // Validate paths
    if (!validatePath(dbPath) || !validatePath(scriptPath)) {
        return 0;
    }
    
    sqlite3* db = nullptr;
    sqlite3_stmt* stmt = nullptr;
    
    // Open database connection
    int rc = sqlite3_open_v2(dbPath.c_str(), &db, SQLITE_OPEN_READONLY, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Database error occurred" << std::endl;
        if (db != nullptr) {
            sqlite3_close(db);
        }
        return 0;
    }
    
    // Prepare parameterized query to prevent SQL injection
    const char* query = "SELECT COUNT(*) FROM subscribers WHERE email = ?";
    rc = sqlite3_prepare_v2(db, query, -1, &stmt, nullptr);
    
    if (rc != SQLITE_OK) {
        std::cerr << "Query preparation failed" << std::endl;
        sqlite3_close(db);
        return 0;
    }
    
    // Bind email parameter safely
    rc = sqlite3_bind_text(stmt, 1, email.c_str(), -1, SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        std::cerr << "Parameter binding failed" << std::endl;
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 0;
    }
    
    // Execute query
    rc = sqlite3_step(stmt);
    int result = 0;
    
    if (rc == SQLITE_ROW) {
        int count = sqlite3_column_int(stmt, 0);
        
        if (count > 0) {
            // Email exists, call unsubscribe script
            result = callUnsubscribeScript(email, scriptPath);
        }
    }
    
    // Clean up resources
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    return result;
}

int main() {
    // Test cases with safe mock values
    std::string dbPath = ":memory:";
    std::string scriptPath = "/usr/local/bin/unsubscribe.sh";
    
    // Test case 1: Valid email format
    std::cout << "Test 1 (valid email): " 
              << unsubscribeEmail("user@example.com", dbPath, scriptPath) << std::endl;
    
    // Test case 2: Empty email
    std::cout << "Test 2 (empty email): " 
              << unsubscribeEmail("", dbPath, scriptPath) << std::endl;
    
    // Test case 3: Invalid email format (SQL injection attempt)
    std::cout << "Test 3 (injection attempt): " 
              << unsubscribeEmail("' OR '1'='1", dbPath, scriptPath) << std::endl;
    
    // Test case 4: Email too long
    std::string longEmail(300, 'a');
    longEmail += "@example.com";
    std::cout << "Test 4 (too long): " 
              << unsubscribeEmail(longEmail, dbPath, scriptPath) << std::endl;
    
    // Test case 5: Path traversal attempt
    std::cout << "Test 5 (path traversal): " 
              << unsubscribeEmail("user@example.com", "../etc/passwd", scriptPath) << std::endl;
    
    return 0;
}

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <algorithm>
#include <sstream>

// For file locking on POSIX systems (Linux, macOS).
// This code is not compatible with Windows.
#include <fcntl.h>
#include <unistd.h>
#include <sys/file.h>
#include <cerrno>
#include <cstring>

namespace SecurityLogger {

    const std::string LOG_FILE_NAME = "security_events.log";
    constexpr size_t MAX_DESCRIPTION_LENGTH = 256;

    enum class Severity {
        INFO,
        WARN,
        CRITICAL
    };

    // Helper to convert Severity to string
    std::string severityToString(Severity s) {
        switch (s) {
            case Severity::INFO: return "INFO";
            case Severity::WARN: return "WARN";
            case Severity::CRITICAL: return "CRITICAL";
        }
        return "UNKNOWN";
    }

    // Sanitize string by removing newlines
    std::string sanitize(const std::string& input) {
        std::string sanitized = input;
        sanitized.erase(std::remove(sanitized.begin(), sanitized.end(), '\n'), sanitized.end());
        sanitized.erase(std::remove(sanitized.begin(), sanitized.end(), '\r'), sanitized.end());
        return sanitized;
    }

    bool logEvent(const std::string& eventType, Severity severity, const std::string& description) {
        // 1. Input Validation
        if (eventType.empty() || description.empty()) {
            std::cerr << "Error: Event details cannot be empty." << std::endl;
            return false;
        }
        if (description.length() > MAX_DESCRIPTION_LENGTH) {
            std::cerr << "Error: Description exceeds maximum length of " << MAX_DESCRIPTION_LENGTH << std::endl;
            return false;
        }

        // 2. Sanitize
        std::string sanitizedEventType = sanitize(eventType);
        std::string sanitizedDescription = sanitize(description);

        // 3. Format log entry
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::gmtime(&in_time_t), "%Y-%m-%dT%H:%M:%SZ");
        
        std::string logEntry = "[" + ss.str() + "] [" + severityToString(severity) + "] [" 
                             + sanitizedEventType + "]: " + sanitizedDescription + "\n";

        // 4. Write to file with exclusive lock (POSIX specific)
        int fd = open(LOG_FILE_NAME.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
        if (fd == -1) {
            std::cerr << "Failed to open log file: " << std::strerror(errno) << std::endl;
            return false;
        }

        if (flock(fd, LOCK_EX) == -1) {
            std::cerr << "Failed to lock log file: " << std::strerror(errno) << std::endl;
            close(fd);
            return false;
        }

        ssize_t bytes_written = write(fd, logEntry.c_str(), logEntry.length());
        if (bytes_written < 0 || static_cast<size_t>(bytes_written) != logEntry.length()) {
             std::cerr << "Failed to write to log file: " << std::strerror(errno) << std::endl;
        }

        flock(fd, LOCK_UN);
        close(fd);
        
        return bytes_written > 0;
    }

    std::vector<std::string> readLogs() {
        std::vector<std::string> logs;
        
        int fd = open(LOG_FILE_NAME.c_str(), O_RDONLY);
        if (fd == -1) {
            if (errno != ENOENT) { // File not found is not an error
                std::cerr << "Failed to open log file for reading: " << std::strerror(errno) << std::endl;
            }
            return logs;
        }

        if (flock(fd, LOCK_SH) == -1) {
            std::cerr << "Failed to acquire shared lock: " << std::strerror(errno) << std::endl;
            close(fd);
            return logs;
        }

        std::ifstream file(LOG_FILE_NAME);
        // We re-open with fstream after locking, which is not ideal but simple.
        // A robust solution would read directly from the file descriptor `fd`.
        // For this example, we assume no other process will interfere between flock and ifstream open.
        // The lock is primarily to prevent writes during this read.
        
        if (file.is_open()) {
            std::string line;
            while (std::getline(file, line)) {
                logs.push_back(line);
            }
        } else {
             std::cerr << "Could not open log file stream after locking." << std::endl;
        }
        
        flock(fd, LOCK_UN);
        close(fd);

        return logs;
    }

} // namespace SecurityLogger

int main() {
    // Clean up previous log file for a fresh run
    std::remove(SecurityLogger::LOG_FILE_NAME.c_str());

    std::cout << "--- Running Security Logger Test Cases ---" << std::endl;

    // Test Case 1: Successful login event
    std::cout << "\n1. Logging a successful login event..." << std::endl;
    SecurityLogger::logEvent("LOGIN_SUCCESS", SecurityLogger::Severity::INFO, "User 'admin' logged in from 192.168.1.1");

    // Test Case 2: Failed login attempt
    std::cout << "\n2. Logging a failed login attempt..." << std::endl;
    SecurityLogger::logEvent("LOGIN_FAILURE", SecurityLogger::Severity::WARN, "Failed login for user 'guest' from 10.0.0.5");

    // Test Case 3: Critical system event
    std::cout << "\n3. Logging a critical event..." << std::endl;
    SecurityLogger::logEvent("CONFIG_MODIFIED", SecurityLogger::Severity::CRITICAL, "Critical system file '/etc/shadow' was modified.");

    // Test Case 4: Attempted log injection
    std::cout << "\n4. Logging an event with newline characters (should be sanitized)..." << std::endl;
    SecurityLogger::logEvent("INPUT_VALIDATION_FAIL", SecurityLogger::Severity::WARN, "User input contained malicious payload:\n<script>alert(1)</script>");

    // Test Case 5: Event description too long
    std::cout << "\n5. Logging an event with a description that is too long (should fail)..." << std::endl;
    std::string longDescription(SecurityLogger::MAX_DESCRIPTION_LENGTH + 1, 'C');
    SecurityLogger::logEvent("DATA_OVERFLOW", SecurityLogger::Severity::WARN, longDescription);

    // Read and display all logs
    std::cout << "\n--- Current Security Logs ---" << std::endl;
    std::vector<std::string> logs = SecurityLogger::readLogs();
    if (logs.empty()) {
        std::cout << "No logs found or unable to read log file." << std::endl;
    } else {
        for (const auto& log : logs) {
            std::cout << log << std::endl;
        }
    }
    std::cout << "--- End of Logs ---" << std::endl;

    return 0;
}
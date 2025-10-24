
#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <vector>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <stdexcept>
#include <memory>
#include <sys/stat.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

// Security event severity levels
enum class Severity {
    INFO,
    WARNING,
    CRITICAL
};

// Security event types
enum class EventType {
    LOGIN_SUCCESS,
    LOGIN_FAILURE,
    UNAUTHORIZED_ACCESS,
    FILE_ACCESS,
    CONFIGURATION_CHANGE
};

// Structure to hold security event data
struct SecurityEvent {
    std::time_t timestamp;
    Severity severity;
    EventType type;
    std::string username;
    std::string ipAddress;
    std::string description;
};

class SecurityLogger {
private:
    std::string logFilePath;
    const size_t MAX_USERNAME_LEN = 256;
    const size_t MAX_IP_LEN = 45; // IPv6 max length
    const size_t MAX_DESC_LEN = 1024;
    
    // Validate and sanitize input strings to prevent injection attacks
    bool validateString(const std::string& input, size_t maxLen) const {
        if (input.empty() || input.length() > maxLen) {
            return false;
        }
        // Check for null bytes that could truncate strings
        if (input.find('\\0') != std::string::npos) {
            return false;
        }
        return true;
    }
    
    // Sanitize string by removing or escaping potentially dangerous characters
    std::string sanitizeString(const std::string& input) const {
        std::string sanitized;
        sanitized.reserve(input.length());
        
        for (char c : input) {
            // Remove control characters except newline which we'll convert\n            if (c == '\
' || c == '\\r') {\n                sanitized += ' ';\n            } else if (c >= 32 && c <= 126) {\n                // Only allow printable ASCII characters\n                sanitized += c;\n            }\n        }\n        return sanitized;\n    }\n    \n    // Convert severity enum to string\n    std::string severityToString(Severity sev) const {\n        switch (sev) {\n            case Severity::INFO: return "INFO";\n            case Severity::WARNING: return "WARNING";\n            case Severity::CRITICAL: return "CRITICAL";\n            default: return "UNKNOWN";\n        }\n    }\n    \n    // Convert event type enum to string\n    std::string eventTypeToString(EventType type) const {\n        switch (type) {\n            case EventType::LOGIN_SUCCESS: return "LOGIN_SUCCESS";\n            case EventType::LOGIN_FAILURE: return "LOGIN_FAILURE";\n            case EventType::UNAUTHORIZED_ACCESS: return "UNAUTHORIZED_ACCESS";\n            case EventType::FILE_ACCESS: return "FILE_ACCESS";\n            case EventType::CONFIGURATION_CHANGE: return "CONFIGURATION_CHANGE";\n            default: return "UNKNOWN";\n        }\n    }\n    \n    // Format timestamp in ISO 8601 format\n    std::string formatTimestamp(std::time_t timestamp) const {\n        std::tm* tm_info = std::gmtime(&timestamp);\n        if (!tm_info) {\n            return "INVALID_TIME";\n        }\n        \n        char buffer[32];\n        if (std::strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%SZ", tm_info) == 0) {\n            return "INVALID_TIME";\n        }\n        return std::string(buffer);\n    }\n    \n    // Validate IP address format (basic validation)\n    bool validateIpAddress(const std::string& ip) const {\n        if (!validateString(ip, MAX_IP_LEN)) {\n            return false;\n        }\n        \n        // Check for valid IPv4 or IPv6 characters\n        for (char c : ip) {\n            if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || \n                  (c >= 'A' && c <= 'F') || c == '.' || c == ':')) {\n                return false;\n            }\n        }\n        return true;\n    }\n\npublic:\n    explicit SecurityLogger(const std::string& logPath) : logFilePath(logPath) {\n        // Validate log path to prevent directory traversal\n        if (logPath.empty() || logPath.length() > 4096) {\n            throw std::invalid_argument("Invalid log file path length");\n        }\n        \n        // Check for path traversal attempts\n        if (logPath.find("..") != std::string::npos) {\n            throw std::invalid_argument("Path traversal detected in log path");\n        }\n    }\n    \n    // Log a security event with full validation\n    bool logEvent(const SecurityEvent& event) {\n        // Validate all input fields\n        if (!validateString(event.username, MAX_USERNAME_LEN)) {\n            std::cerr << "Invalid username format" << std::endl;\n            return false;\n        }\n        \n        if (!validateIpAddress(event.ipAddress)) {\n            std::cerr << "Invalid IP address format" << std::endl;\n            return false;\n        }\n        \n        if (!validateString(event.description, MAX_DESC_LEN)) {\n            std::cerr << "Invalid description format" << std::endl;\n            return false;\n        }\n        \n        // Sanitize all string inputs to prevent log injection\n        std::string safeUsername = sanitizeString(event.username);\n        std::string safeIpAddress = sanitizeString(event.ipAddress);\n        std::string safeDescription = sanitizeString(event.description);\n        \n        // Format the log entry\n        std::ostringstream logEntry;\n        logEntry << formatTimestamp(event.timestamp) << " | "\n                 << severityToString(event.severity) << " | "\n                 << eventTypeToString(event.type) << " | "\n                 << "User: " << safeUsername << " | "\n                 << "IP: " << safeIpAddress << " | "\n                 << "Description: " << safeDescription << std::endl;\n        \n        // Use RAII for file handling\n        std::ofstream logFile(logFilePath, std::ios::app);\n        if (!logFile.is_open()) {\n            std::cerr << "Failed to open log file" << std::endl;\n            return false;\n        }\n        \n        logFile << logEntry.str();\n        \n        if (!logFile.good()) {\n            std::cerr << "Failed to write to log file" << std::endl;\n            return false;\n        }\n        \n        return true;\n    }\n    \n    // Monitor and analyze recent events for suspicious patterns\n    std::vector<std::string> analyzeEvents(int failureThreshold) {\n        std::vector<std::string> alerts;\n        std::ifstream logFile(logFilePath);\n        \n        if (!logFile.is_open()) {\n            alerts.push_back("Unable to analyze events: log file not accessible");\n            return alerts;\n        }\n        \n        // Count failed login attempts per user\n        std::map<std::string, int> failedLogins;\n        std::string line;\n        \n        while (std::getline(logFile, line)) {\n            if (line.find("LOGIN_FAILURE") != std::string::npos) {\n                size_t userPos = line.find("User: ");\n                if (userPos != std::string::npos) {\n                    size_t userStart = userPos + 6;\n                    size_t userEnd = line.find(" |", userStart);\n                    if (userEnd != std::string::npos) {\n                        std::string user = line.substr(userStart, userEnd - userStart);\n                        failedLogins[user]++;\n                    }\n                }\n            }\n        }\n        \n        // Generate alerts for users exceeding threshold\n        for (const auto& entry : failedLogins) {\n            if (entry.second >= failureThreshold) {\n                std::ostringstream alert;\n                alert << "ALERT: User '" << entry.first \n                      << "' has " << entry.second \n                      << " failed login attempts";\n                alerts.push_back(alert.str());\n            }\n        }\n        \n        return alerts;\n    }\n};\n\nint main() {\n    try {\n        // Initialize logger with validated path\n        SecurityLogger logger("security_events.log");\n        \n        // Test case 1: Successful login event\n        SecurityEvent event1{\n            std::time(nullptr),\n            Severity::INFO,\n            EventType::LOGIN_SUCCESS,\n            "admin",\n            "192.168.1.100",\n            "User logged in successfully from web interface"\n        };\n        \n        if (logger.logEvent(event1)) {\n            std::cout << "Test 1 passed: Login success logged" << std::endl;\n        }\n        \n        // Test case 2: Failed login attempt\n        SecurityEvent event2{\n            std::time(nullptr),\n            Severity::WARNING,\n            EventType::LOGIN_FAILURE,\n            "admin",\n            "192.168.1.100",\n            "Invalid password provided"\n        };\n        \n        if (logger.logEvent(event2)) {\n            std::cout << "Test 2 passed: Login failure logged" << std::endl;\n        }\n        \n        // Test case 3: Multiple failed attempts (simulate attack)\n        for (int i = 0; i < 5; i++) {\n            SecurityEvent event{\n                std::time(nullptr),\n                Severity::WARNING,\n                EventType::LOGIN_FAILURE,\n                "testuser",\n                "10.0.0.50",\n                "Brute force attack detected"\n            };\n            logger.logEvent(event);\n        }\n        std::cout << "Test 3 passed: Multiple login failures logged" << std::endl;\n        \n        // Test case 4: Unauthorized access attempt\n        SecurityEvent event4{\n            std::time(nullptr),\n            Severity::CRITICAL,\n            EventType::UNAUTHORIZED_ACCESS,\n            "guest",\n            "203.0.113.45",\n            "Attempted access to restricted resource /admin/config"\n        };\n        \n        if (logger.logEvent(event4)) {\n            std::cout << "Test 4 passed: Unauthorized access logged" << std::endl;\n        }\n        \n        // Test case 5: Analyze events and generate alerts\n        std::vector<std::string> alerts = logger.analyzeEvents(3);\n        std::cout << "Test 5: Event analysis completed" << std::endl;\n        \n        if (!alerts.empty()) {\n            std::cout << "Security Alerts Generated:" << std::endl;\n            for (const auto& alert : alerts) {\n                std::cout << "  " << alert << std::endl;\n            }\n        } else {\n            std::cout << "No security alerts at this time" << std::endl;\n        }\n        \n    } catch (const std::exception& e) {\n        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}

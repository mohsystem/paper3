#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <mutex>
#include <sstream>

// A global mutex to protect file access, making the logger thread-safe.
std::mutex log_mutex;

enum class Severity {
    INFO,
    WARNING,
    CRITICAL
};

enum class EventType {
    LOGIN_SUCCESS,
    LOGIN_FAILURE,
    FILE_ACCESS,
    CONFIG_CHANGE,
    UNAUTHORIZED_ATTEMPT
};

// Helper function to convert Severity enum to string
std::string severityToString(Severity s) {
    switch (s) {
        case Severity::INFO: return "INFO";
        case Severity::WARNING: return "WARNING";
        case Severity::CRITICAL: return "CRITICAL";
        default: return "UNKNOWN";
    }
}

// Helper function to convert EventType enum to string
std::string eventTypeToString(EventType et) {
    switch (et) {
        case EventType::LOGIN_SUCCESS: return "LOGIN_SUCCESS";
        case EventType::LOGIN_FAILURE: return "LOGIN_FAILURE";
        case EventType::FILE_ACCESS: return "FILE_ACCESS";
        case EventType::CONFIG_CHANGE: return "CONFIG_CHANGE";
        case EventType::UNAUTHORIZED_ATTEMPT: return "UNAUTHORIZED_ATTEMPT";
        default: return "UNKNOWN";
    }
}

/**
 * Simulates sending an alert for critical events to standard output.
 * In a real system, this would integrate with a monitoring tool.
 * @param alertMessage The message for the alert.
 */
void sendAlert(const std::string& alertMessage) {
    // This function can be called from a locked section, so we keep it simple.
    std::cout << "ALERT! Critical security event detected:" << std::endl;
    std::cout << ">> " << alertMessage << std::endl;
}

/**
 * Logs a security event to a file in a thread-safe manner.
 * Note: The log file 'security.log' should have its permissions set to be
 * readable/writable only by the application user (e.g., chmod 600).
 *
 * @param severity The severity level of the event.
 * @param eventType The type of the event.
 * @param sourceIp The source IP address.
 * @param description A detailed description of the event.
 */
void logEvent(Severity severity, EventType eventType, const std::string& sourceIp, const std::string& description) {
    // Sanitize description to prevent log injection via newlines.
    std::string sanitizedDescription = description;
    size_t pos;
    while ((pos = sanitizedDescription.find_first_of("\r\n")) != std::string::npos) {
        sanitizedDescription.replace(pos, 1, "_");
    }

    // Get current time
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm buf;
    #ifdef _WIN32
        localtime_s(&buf, &in_time_t);
    #else
        localtime_r(&in_time_t, &buf);
    #endif

    std::stringstream ss;
    ss << std::put_time(&buf, "%Y-%m-%d %H:%M:%S");
    std::string timestamp = ss.str();

    std::stringstream logStream;
    logStream << "[" << timestamp << "] [" << severityToString(severity) << "] ["
              << eventTypeToString(eventType) << "] [SRC_IP: " << sourceIp << "] - "
              << sanitizedDescription;
    std::string logMessage = logStream.str();

    {
        // Use a lock_guard for RAII-style mutex locking to ensure thread safety
        const std::lock_guard<std::mutex> lock(log_mutex);

        // Open file in append mode. The destructor of std::ofstream will close it.
        std::ofstream logFile("security.log", std::ios_base::app);
        if (logFile.is_open()) {
            logFile << logMessage << std::endl;
        } else {
            // Writing to cerr is thread-safe in C++11 and later.
            std::cerr << "Error: Failed to open log file." << std::endl;
        }
    }
    
    // Monitor for critical events and send alerts (outside the lock).
    if (severity == Severity::CRITICAL) {
        sendAlert(logMessage);
    }
}

int main() {
    std::cout << "Logging security events. Check 'security.log' for details." << std::endl;
    std::cout << "---------------------------------------------------------" << std::endl;

    // Test Case 1: Successful login
    logEvent(Severity::INFO, EventType::LOGIN_SUCCESS, "192.168.1.10", "User 'admin' logged in successfully.");

    // Test Case 2: Failed login attempt
    logEvent(Severity::WARNING, EventType::LOGIN_FAILURE, "10.0.0.5", "Failed login attempt for user 'guest'.");

    // Test Case 3: Critical - Multiple failed attempts (Simulated)
    logEvent(Severity::CRITICAL, EventType::LOGIN_FAILURE, "203.0.113.45", "Multiple (5) failed login attempts for user 'root'. IP blocked.");

    // Test Case 4: Configuration change
    logEvent(Severity::INFO, EventType::CONFIG_CHANGE, "127.0.0.1", "User 'admin' updated 'firewall.conf'.");

    // Test Case 5: Unauthorized access attempt
    logEvent(Severity::CRITICAL, EventType::UNAUTHORIZED_ATTEMPT, "198.51.100.88", "Attempt to access '/api/admin' endpoint without authentication.");
    
    std::cout << "---------------------------------------------------------" << std::endl;
    std::cout << "Test cases executed." << std::endl;

    return 0;
}
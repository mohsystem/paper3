#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

// Represents a single security event
struct SecurityEvent {
    std::string timestamp;
    std::string eventType;
    std::string severity;
    std::string description;
};

// Manages logging and monitoring of security events
class SecurityLogger {
private:
    std::vector<SecurityEvent> eventLog;

    // Helper to get current formatted timestamp
    std::string getCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        #ifdef _MSC_VER
            struct tm timeinfo;
            localtime_s(&timeinfo, &in_time_t);
            ss << std::put_time(&timeinfo, "%Y-%m-%d %H:%M:%S");
        #else
            ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S");
        #endif
        return ss.str();
    }

public:
    /**
     * @brief Creates and logs a new security event.
     * @param eventType The type of the event (e.g., "LOGIN").
     * @param severity The severity level (e.g., "INFO", "CRITICAL").
     * @param description A detailed description of the event.
     */
    void logEvent(const std::string& eventType, const std::string& severity, const std::string& description) {
        SecurityEvent newEvent;
        newEvent.timestamp = getCurrentTimestamp();
        newEvent.eventType = eventType;
        newEvent.severity = severity;
        newEvent.description = description;
        
        eventLog.push_back(newEvent);
        // In a real system, this might also write to a file or a remote logging service.
        std::cout << "Logged: [" << newEvent.timestamp << "] [" << newEvent.severity << "] [" << newEvent.eventType << "] " << newEvent.description << std::endl;
    }

    /**
     * @brief Displays all the logged events to the console.
     */
    void displayLogs() const {
        std::cout << "\n--- Security Event Log ---" << std::endl;
        if (eventLog.empty()) {
            std::cout << "No events logged." << std::endl;
        } else {
            for (const auto& event : eventLog) {
                std::cout << "[" << event.timestamp << "] [" << event.severity << "] [" << event.eventType << "] " << event.description << std::endl;
            }
        }
        std::cout << "--------------------------" << std::endl << std::endl;
    }
};


int main() {
    SecurityLogger logger;

    std::cout << "--- Running 5 Test Cases ---" << std::endl;

    // Test Case 1: Successful login
    logger.logEvent("LOGIN_SUCCESS", "INFO", "User 'admin' logged in successfully from IP 192.168.1.10");

    // Test Case 2: Failed login attempt
    logger.logEvent("LOGIN_FAILURE", "WARNING", "Failed login attempt for user 'root' from IP 10.0.0.5");

    // Test Case 3: Unauthorized access attempt
    logger.logEvent("UNAUTHORIZED_ACCESS", "CRITICAL", "Attempt to access '/etc/shadow' by user 'guest'");

    // Test Case 4: File modification
    logger.logEvent("FILE_MODIFICATION", "INFO", "User 'dev' modified '/var/www/index.html'");

    // Test Case 5: Firewall rule change
    logger.logEvent("FIREWALL_UPDATE", "WARNING", "Firewall rule added to block port 8080");

    // Display all logged events at the end
    logger.displayLogs();

    return 0;
}
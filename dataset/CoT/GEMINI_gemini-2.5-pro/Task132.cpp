#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <map>
#include <algorithm>
#include <thread> // For std::this_thread::sleep_for

// Represents a single security event.
struct SecurityEvent {
    long long timestamp; // Unix timestamp in seconds
    std::string eventType;
    std::string userId;
    std::string sourceIp;
    std::string description;
};

// Handles the logging of security events.
// Note: This implementation is not thread-safe. Access from multiple threads
// would require synchronization (e.g., std::mutex).
class SecurityLogger {
private:
    std::vector<SecurityEvent> eventLog;

    // Sanitizes input strings to prevent log injection by removing newlines.
    std::string sanitize(std::string input) {
        // Replace newline and carriage return characters.
        input.erase(std::remove(input.begin(), input.end(), '\n'), input.end());
        input.erase(std::remove(input.begin(), input.end(), '\r'), input.end());
        return input;
    }

public:
    // Logs a new security event.
    void logEvent(const std::string& eventType, const std::string& userId, const std::string& sourceIp, const std::string& description) {
        auto now = std::chrono::system_clock::now();
        long long timestamp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
        
        eventLog.push_back({
            timestamp,
            sanitize(eventType),
            sanitize(userId),
            sanitize(sourceIp),
            sanitize(description)
        });
    }

    const std::vector<SecurityEvent>& getEventLog() const {
        return eventLog;
    }

    void printLogs() const {
        std::cout << "--- Security Event Log ---" << std::endl;
        if (eventLog.empty()) {
            std::cout << "No events logged." << std::endl;
        } else {
            for (const auto& event : eventLog) {
                std::cout << "Event(time=" << event.timestamp
                          << ", type='" << event.eventType
                          << "', user='" << event.userId
                          << "', ip='" << event.sourceIp
                          << "', desc='" << event.description << "')" << std::endl;
            }
        }
        std::cout << "--------------------------\n" << std::endl;
    }
};

// Monitors logs for suspicious patterns.
class SecurityMonitor {
private:
    static const int FAILED_LOGIN_THRESHOLD = 3;
    static const long long TIME_WINDOW_SECONDS = 60;

public:
    void analyzeLogs(const std::vector<SecurityEvent>& events) const {
        std::cout << "--- Security Analysis Report ---" << std::endl;
        std::map<std::string, std::vector<long long>> failedLoginAttempts;

        for (const auto& event : events) {
            if (event.eventType == "LOGIN_FAILURE") {
                failedLoginAttempts[event.sourceIp].push_back(event.timestamp);
            }
        }

        bool alertsFound = false;
        for (auto const& [ip, timestamps] : failedLoginAttempts) {
            if (timestamps.size() >= FAILED_LOGIN_THRESHOLD) {
                std::vector<long long> sorted_ts = timestamps;
                std::sort(sorted_ts.begin(), sorted_ts.end());

                for (size_t i = 0; i <= sorted_ts.size() - FAILED_LOGIN_THRESHOLD; ++i) {
                    if (sorted_ts[i + FAILED_LOGIN_THRESHOLD - 1] - sorted_ts[i] <= TIME_WINDOW_SECONDS) {
                        std::cout << "ALERT: Potential brute-force attack detected from IP: " << ip
                                  << ". " << FAILED_LOGIN_THRESHOLD << " failed attempts within "
                                  << TIME_WINDOW_SECONDS << " seconds." << std::endl;
                        alertsFound = true;
                        break; // One alert per IP is sufficient
                    }
                }
            }
        }

        if (!alertsFound) {
            std::cout << "No suspicious activity detected." << std::endl;
        }
        std::cout << "------------------------------\n" << std::endl;
    }
};

void run_test_cases() {
    SecurityLogger logger;
    SecurityMonitor monitor;

    // --- Test Case 1: Normal Operations ---
    std::cout << ">>> Running Test Case 1: Normal Operations" << std::endl;
    logger.logEvent("LOGIN_SUCCESS", "alice", "192.168.1.10", "User alice logged in successfully.");
    std::this_thread::sleep_for(std::chrono::seconds(1));
    logger.logEvent("FILE_ACCESS", "alice", "192.168.1.10", "User alice accessed /data/report.docx.");
    std::this_thread::sleep_for(std::chrono::seconds(1));
    logger.logEvent("LOGOUT", "alice", "192.168.1.10", "User alice logged out.");
    logger.printLogs();
    monitor.analyzeLogs(logger.getEventLog());

    // --- Test Case 2: Brute-Force Attack ---
    std::cout << ">>> Running Test Case 2: Brute-Force Attack" << std::endl;
    logger.logEvent("LOGIN_FAILURE", "admin", "203.0.113.55", "Failed login attempt for user admin.");
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    logger.logEvent("LOGIN_FAILURE", "root", "203.0.113.55", "Failed login attempt for user root.");
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    logger.logEvent("LOGIN_FAILURE", "admin", "203.0.113.55", "Failed login attempt for user admin.");
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    logger.logEvent("LOGIN_FAILURE", "guest", "203.0.113.55", "Failed login attempt for user guest.");
    logger.printLogs();
    monitor.analyzeLogs(logger.getEventLog());
    
    // --- Test Case 3: Distributed Attack (should not trigger alert) ---
    std::cout << ">>> Running Test Case 3: Distributed Low-and-Slow Attack" << std::endl;
    logger.logEvent("LOGIN_FAILURE", "user", "198.51.100.1", "Failed login.");
    std::this_thread::sleep_for(std::chrono::seconds(2));
    logger.logEvent("LOGIN_FAILURE", "user", "198.51.100.2", "Failed login.");
    std::this_thread::sleep_for(std::chrono::seconds(2));
    logger.logEvent("LOGIN_FAILURE", "user", "198.51.100.3", "Failed login.");
    logger.printLogs();
    monitor.analyzeLogs(logger.getEventLog());
    
    // --- Test Case 4: Mixed Activity & Successful Intrusion ---
    std::cout << ">>> Running Test Case 4: Mixed Activity & Eventual Success" << std::endl;
    logger.logEvent("LOGIN_FAILURE", "bob", "10.0.0.42", "Failed login: invalid password.");
    std::this_thread::sleep_for(std::chrono::seconds(1));
    logger.logEvent("LOGIN_FAILURE", "bob", "10.0.0.42", "Failed login: invalid password.");
    std::this_thread::sleep_for(std::chrono::seconds(1));
    logger.logEvent("LOGIN_SUCCESS", "bob", "10.0.0.42", "User bob logged in successfully.");
    logger.printLogs();
    monitor.analyzeLogs(logger.getEventLog());
    
    // --- Test Case 5: Permission Escalation Attempt ---
    std::cout << ">>> Running Test Case 5: Permission Escalation Attempt" << std::endl;
    logger.logEvent("LOGIN_SUCCESS", "charlie", "172.16.0.12", "User charlie logged in successfully.");
    std::this_thread::sleep_for(std::chrono::seconds(1));
    logger.logEvent("PERMISSION_DENIED", "charlie", "172.16.0.12", "Access denied for /etc/shadow.");
    logger.printLogs();
    monitor.analyzeLogs(logger.getEventLog());
}

int main() {
    run_test_cases();
    return 0;
}
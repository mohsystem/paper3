
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <chrono>
#include <regex>
#include <mutex>
#include <algorithm>
#include <iomanip>
#include <sstream>

class Task132 {
private:
    static const int MAX_LOG_SIZE = 10000;
    static const int MAX_EVENT_LENGTH = 1000;
    static const std::string LOG_FILE;
    
    struct SecurityEvent {
        std::string timestamp;
        std::string eventType;
        std::string description;
        std::string severity;
        std::string sourceIP;
        
        std::string toString() const {
            return "[" + timestamp + "] [" + severity + "] [" + eventType + "] " + 
                   description + " - Source: " + (sourceIP.empty() ? "N/A" : sourceIP);
        }
    };
    
    std::vector<SecurityEvent> events;
    std::map<std::string, int> eventCounters;
    mutable std::mutex mutex;
    
    std::string sanitizeInput(const std::string& input) {
        if (input.empty()) {
            return "";
        }
        
        std::string result = input;
        
        // Limit length
        if (result.length() > MAX_EVENT_LENGTH) {
            result = result.substr(0, MAX_EVENT_LENGTH);
        }
        
        // Remove control characters and newlines
        result.erase(std::remove_if(result.begin(), result.end(),
            [](char c) { return c == '\\n' || c == '\\r' || c == '\\t' || c < 32 || c > 126; }),
            result.end());
        
        // Trim whitespace
        result.erase(0, result.find_first_not_of(" \\t"));
        result.erase(result.find_last_not_of(" \\t") + 1);
        
        return result;
    }
    
    bool isValidSeverity(const std::string& severity) {
        std::string upper = severity;
        std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
        return upper == "LOW" || upper == "MEDIUM" || upper == "HIGH" || upper == "CRITICAL";
    }
    
    bool isValidIP(const std::string& ip) {
        if (ip.empty()) {
            return false;
        }
        
        std::regex ipPattern(
            "^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$"
        );
        
        return std::regex_match(ip, ipPattern);
    }
    
    std::string getCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }
    
    void writeToFile(const SecurityEvent& event) {
        std::ofstream file(LOG_FILE, std::ios::app);
        if (file.is_open()) {
            file << event.toString() << std::endl;
            file.close();
        }
    }
    
public:
    Task132() {}
    
    bool logEvent(const std::string& eventType, const std::string& description,
                  const std::string& severity, const std::string& sourceIP) {
        std::lock_guard<std::mutex> lock(mutex);
        
        try {
            // Validate inputs
            if (eventType.empty() || description.empty() || severity.empty()) {
                return false;
            }
            
            // Sanitize all inputs
            std::string cleanEventType = sanitizeInput(eventType);
            std::string cleanDescription = sanitizeInput(description);
            std::string cleanSeverity = sanitizeInput(severity);
            std::string cleanSourceIP = sanitizeInput(sourceIP);
            
            // Validate severity level
            if (!isValidSeverity(cleanSeverity)) {
                return false;
            }
            
            // Validate IP format if provided
            if (!cleanSourceIP.empty() && !isValidIP(cleanSourceIP)) {
                return false;
            }
            
            // Convert severity to uppercase
            std::transform(cleanSeverity.begin(), cleanSeverity.end(), 
                         cleanSeverity.begin(), ::toupper);
            
            SecurityEvent event;
            event.timestamp = getCurrentTimestamp();
            event.eventType = cleanEventType;
            event.description = cleanDescription;
            event.severity = cleanSeverity;
            event.sourceIP = cleanSourceIP;
            
            // Prevent log overflow
            if (events.size() >= MAX_LOG_SIZE) {
                events.erase(events.begin());
            }
            
            events.push_back(event);
            eventCounters[cleanEventType]++;
            
            // Write to file securely
            writeToFile(event);
            
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error logging event: " << e.what() << std::endl;
            return false;
        }
    }
    
    std::vector<SecurityEvent> getEventsBySeverity(const std::string& severity) {
        std::lock_guard<std::mutex> lock(mutex);
        std::string cleanSeverity = sanitizeInput(severity);
        std::transform(cleanSeverity.begin(), cleanSeverity.end(), 
                      cleanSeverity.begin(), ::toupper);
        
        std::vector<SecurityEvent> result;
        for (const auto& event : events) {
            if (event.severity == cleanSeverity) {
                result.push_back(event);
            }
        }
        return result;
    }
    
    int getEventCount(const std::string& eventType) {
        std::lock_guard<std::mutex> lock(mutex);
        std::string cleanEventType = sanitizeInput(eventType);
        auto it = eventCounters.find(cleanEventType);
        return (it != eventCounters.end()) ? it->second : 0;
    }
    
    std::vector<SecurityEvent> getAllEvents() {
        std::lock_guard<std::mutex> lock(mutex);
        return events;
    }
};

const std::string Task132::LOG_FILE = "security_events.log";

int main() {
    Task132 monitor;
    
    std::cout << "=== Security Event Logging System ===\\n" << std::endl;
    
    // Test Case 1: Valid login failure event
    std::cout << "Test Case 1: Login Failure" << std::endl;
    bool result1 = monitor.logEvent("LOGIN_FAILURE", "Failed login attempt for user admin", 
                                    "HIGH", "192.168.1.100");
    std::cout << "Event logged: " << (result1 ? "true" : "false") << std::endl;
    
    // Test Case 2: Valid unauthorized access event
    std::cout << "\\nTest Case 2: Unauthorized Access" << std::endl;
    bool result2 = monitor.logEvent("UNAUTHORIZED_ACCESS", 
                                    "Attempted access to restricted resource", 
                                    "CRITICAL", "10.0.0.50");
    std::cout << "Event logged: " << (result2 ? "true" : "false") << std::endl;
    
    // Test Case 3: Invalid severity level (should fail)
    std::cout << "\\nTest Case 3: Invalid Severity" << std::endl;
    bool result3 = monitor.logEvent("DATA_BREACH", "Sensitive data accessed", 
                                    "INVALID_LEVEL", "172.16.0.1");
    std::cout << "Event logged: " << (result3 ? "true" : "false") << std::endl;
    
    // Test Case 4: Log injection attempt (should be sanitized)
    std::cout << "\\nTest Case 4: Log Injection Prevention" << std::endl;
    bool result4 = monitor.logEvent("MALICIOUS_INPUT", 
                                    "Test\\nFake Log Entry\\r\\nAnother Line", 
                                    "MEDIUM", "192.168.1.50");
    std::cout << "Event logged: " << (result4 ? "true" : "false") << std::endl;
    
    // Test Case 5: Valid firewall block event
    std::cout << "\\nTest Case 5: Firewall Block" << std::endl;
    bool result5 = monitor.logEvent("FIREWALL_BLOCK", 
                                    "Blocked suspicious traffic", 
                                    "LOW", "203.0.113.10");
    std::cout << "Event logged: " << (result5 ? "true" : "false") << std::endl;
    
    // Display all logged events
    std::cout << "\\n=== All Logged Security Events ===" << std::endl;
    auto allEvents = monitor.getAllEvents();
    for (const auto& event : allEvents) {
        std::cout << event.toString() << std::endl;
    }
    
    // Display high severity events
    std::cout << "\\n=== High Severity Events ===" << std::endl;
    auto highEvents = monitor.getEventsBySeverity("HIGH");
    for (const auto& event : highEvents) {
        std::cout << event.toString() << std::endl;
    }
    
    // Display event counts
    std::cout << "\\n=== Event Counts ===" << std::endl;
    std::cout << "LOGIN_FAILURE: " << monitor.getEventCount("LOGIN_FAILURE") << std::endl;
    std::cout << "UNAUTHORIZED_ACCESS: " << monitor.getEventCount("UNAUTHORIZED_ACCESS") << std::endl;
    
    return 0;
}

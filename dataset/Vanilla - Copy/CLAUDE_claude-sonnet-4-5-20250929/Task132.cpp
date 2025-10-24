
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <ctime>
#include <sstream>
#include <iomanip>

using namespace std;

class SecurityEvent {
private:
    string eventType;
    string severity;
    string source;
    string description;
    time_t timestamp;

public:
    SecurityEvent(string eventType, string severity, string source, string description)
        : eventType(eventType), severity(severity), source(source), description(description) {
        timestamp = time(nullptr);
    }

    string getEventType() const { return eventType; }
    string getSeverity() const { return severity; }
    string getSource() const { return source; }
    string getDescription() const { return description; }
    time_t getTimestamp() const { return timestamp; }

    string toString() const {
        char timeStr[100];
        struct tm* timeinfo = localtime(&timestamp);
        strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", timeinfo);
        
        ostringstream oss;
        oss << "[" << timeStr << "] [" << severity << "] [" << eventType << "] " 
            << source << " - " << description;
        return oss.str();
    }
};

class SecurityMonitor {
private:
    vector<SecurityEvent> eventLog;
    map<string, int> eventCounts;
    map<string, int> severityCounts;

public:
    SecurityMonitor() {}

    void logEvent(string eventType, string severity, string source, string description) {
        SecurityEvent event(eventType, severity, source, description);
        eventLog.push_back(event);
        
        eventCounts[eventType]++;
        severityCounts[severity]++;
        
        cout << "Logged: " << event.toString() << endl;
    }

    vector<SecurityEvent> getEventsBySeverity(string severity) {
        vector<SecurityEvent> result;
        for (const auto& event : eventLog) {
            if (event.getSeverity() == severity) {
                result.push_back(event);
            }
        }
        return result;
    }

    vector<SecurityEvent> getEventsByType(string eventType) {
        vector<SecurityEvent> result;
        for (const auto& event : eventLog) {
            if (event.getEventType() == eventType) {
                result.push_back(event);
            }
        }
        return result;
    }

    map<string, int> getEventStatistics() {
        return eventCounts;
    }

    map<string, int> getSeverityStatistics() {
        return severityCounts;
    }

    vector<SecurityEvent> getAllEvents() {
        return eventLog;
    }

    int getTotalEvents() {
        return eventLog.size();
    }

    void generateReport() {
        cout << "\\n=== Security Event Report ===" << endl;
        cout << "Total Events: " << getTotalEvents() << endl;
        cout << "\\nEvents by Type:" << endl;
        for (const auto& pair : eventCounts) {
            cout << "  " << pair.first << ": " << pair.second << endl;
        }
        cout << "\\nEvents by Severity:" << endl;
        for (const auto& pair : severityCounts) {
            cout << "  " << pair.first << ": " << pair.second << endl;
        }
        cout << "============================\\n" << endl;
    }
};

int main() {
    SecurityMonitor monitor;
    
    // Test Case 1: Log authentication failure
    cout << "Test Case 1: Authentication Failure" << endl;
    monitor.logEvent("AUTH_FAILURE", "HIGH", "192.168.1.100", "Failed login attempt for user 'admin'");
    
    // Test Case 2: Log successful login
    cout << "\\nTest Case 2: Successful Login" << endl;
    monitor.logEvent("AUTH_SUCCESS", "INFO", "192.168.1.101", "User 'john.doe' logged in successfully");
    
    // Test Case 3: Log unauthorized access attempt
    cout << "\\nTest Case 3: Unauthorized Access" << endl;
    monitor.logEvent("UNAUTHORIZED_ACCESS", "CRITICAL", "192.168.1.105", "Attempt to access restricted resource");
    
    // Test Case 4: Log multiple events and generate report
    cout << "\\nTest Case 4: Multiple Events" << endl;
    monitor.logEvent("AUTH_FAILURE", "HIGH", "192.168.1.102", "Brute force attack detected");
    monitor.logEvent("DATA_ACCESS", "MEDIUM", "192.168.1.103", "Sensitive data accessed by user 'jane.smith'");
    monitor.logEvent("AUTH_FAILURE", "HIGH", "192.168.1.100", "Another failed login for 'admin'");
    
    monitor.generateReport();
    
    // Test Case 5: Query events by severity
    cout << "Test Case 5: Query High Severity Events" << endl;
    vector<SecurityEvent> highSeverityEvents = monitor.getEventsBySeverity("HIGH");
    cout << "High Severity Events: " << highSeverityEvents.size() << endl;
    for (const auto& event : highSeverityEvents) {
        cout << "  " << event.toString() << endl;
    }
    
    cout << "\\nQuery Events by Type (AUTH_FAILURE):" << endl;
    vector<SecurityEvent> authFailures = monitor.getEventsByType("AUTH_FAILURE");
    cout << "Authentication Failures: " << authFailures.size() << endl;
    
    return 0;
}

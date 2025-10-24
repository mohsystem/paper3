
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <memory>

using namespace std;

enum class EventType {
    LOGIN_SUCCESS,
    LOGIN_FAILURE,
    UNAUTHORIZED_ACCESS,
    DATA_MODIFICATION,
    PRIVILEGE_ESCALATION,
    SYSTEM_ERROR
};

enum class Severity {
    LOW,
    MEDIUM,
    HIGH,
    CRITICAL
};

string eventTypeToString(EventType type) {
    switch(type) {
        case EventType::LOGIN_SUCCESS: return "LOGIN_SUCCESS";
        case EventType::LOGIN_FAILURE: return "LOGIN_FAILURE";
        case EventType::UNAUTHORIZED_ACCESS: return "UNAUTHORIZED_ACCESS";
        case EventType::DATA_MODIFICATION: return "DATA_MODIFICATION";
        case EventType::PRIVILEGE_ESCALATION: return "PRIVILEGE_ESCALATION";
        case EventType::SYSTEM_ERROR: return "SYSTEM_ERROR";
        default: return "UNKNOWN";
    }
}

string severityToString(Severity sev) {
    switch(sev) {
        case Severity::LOW: return "LOW";
        case Severity::MEDIUM: return "MEDIUM";
        case Severity::HIGH: return "HIGH";
        case Severity::CRITICAL: return "CRITICAL";
        default: return "UNKNOWN";
    }
}

class SecurityEvent {
private:
    string eventId;
    time_t timestamp;
    EventType eventType;
    Severity severity;
    string userId;
    string ipAddress;
    string description;
    string resourceAccessed;
    
    string generateEventId() {
        stringstream ss;
        ss << "EVT-" << hex << uppercase << (rand() % 0xFFFFFFFF);
        return ss.str().substr(0, 12);
    }
    
    string sanitizeInput(const string& input) {
        if (input.empty()) return "N/A";
        string sanitized = input;
        replace(sanitized.begin(), sanitized.end(), '\\r', ' ');
        replace(sanitized.begin(), sanitized.end(), '\\n', ' ');
        return sanitized;
    }
    
public:
    SecurityEvent(EventType type, Severity sev, const string& user,
                  const string& ip, const string& desc, const string& resource)
        : eventType(type), severity(sev) {
        eventId = generateEventId();
        timestamp = time(nullptr);
        userId = sanitizeInput(user);
        ipAddress = sanitizeInput(ip);
        description = sanitizeInput(desc);
        resourceAccessed = sanitizeInput(resource);
    }
    
    string toString() const {
        stringstream ss;
        char timeBuffer[100];
        strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%d %H:%M:%S", localtime(&timestamp));
        
        ss << "[" << timeBuffer << "] "
           << "[" << eventId << "] "
           << "[" << severityToString(severity) << "] "
           << "[" << eventTypeToString(eventType) << "] "
           << "User: " << userId << " | "
           << "IP: " << ipAddress << " | "
           << "Resource: " << resourceAccessed << " | "
           << "Description: " << description;
        
        return ss.str();
    }
    
    Severity getSeverity() const { return severity; }
    EventType getEventType() const { return eventType; }
};

class SecurityLogger {
private:
    vector<shared_ptr<SecurityEvent>> eventBuffer;
    int maxBufferSize;
    string logFile;
    
    void writeToFile(const SecurityEvent& event) {
        ofstream file(logFile, ios::app);
        if (file.is_open()) {
            file << event.toString() << endl;
            file.close();
        } else {
            cerr << "Error writing to log file" << endl;
        }
    }
    
    void flushBuffer() {
        eventBuffer.clear();
    }
    
public:
    SecurityLogger(int bufferSize, const string& filename = "security_events.log")
        : maxBufferSize(bufferSize), logFile(filename) {}
    
    void logEvent(const SecurityEvent& event) {
        eventBuffer.push_back(make_shared<SecurityEvent>(event));
        writeToFile(event);
        
        if (event.getSeverity() == Severity::CRITICAL || event.getSeverity() == Severity::HIGH) {
            cout << "ALERT: " << event.toString() << endl;
        }
        
        if (eventBuffer.size() >= static_cast<size_t>(maxBufferSize)) {
            flushBuffer();
        }
    }
    
    vector<shared_ptr<SecurityEvent>> getRecentEvents(int count) {
        int size = eventBuffer.size();
        int start = max(0, size - count);
        return vector<shared_ptr<SecurityEvent>>(eventBuffer.begin() + start, eventBuffer.end());
    }
    
    void generateReport() {
        map<EventType, int> eventCounts;
        map<Severity, int> severityCounts;
        
        for (const auto& event : eventBuffer) {
            eventCounts[event->getEventType()]++;
            severityCounts[event->getSeverity()]++;
        }
        
        cout << "\\n=== Security Event Report ===" << endl;
        cout << "Total Events: " << eventBuffer.size() << endl;
        cout << "\\nEvents by Type:" << endl;
        for (const auto& pair : eventCounts) {
            cout << "  " << eventTypeToString(pair.first) << ": " << pair.second << endl;
        }
        cout << "\\nEvents by Severity:" << endl;
        for (const auto& pair : severityCounts) {
            cout << "  " << severityToString(pair.first) << ": " << pair.second << endl;
        }
        cout << "============================\\n" << endl;
    }
};

int main() {
    srand(time(nullptr));
    SecurityLogger logger(100);
    
    // Test Case 1: Login Success
    SecurityEvent event1(EventType::LOGIN_SUCCESS, Severity::LOW, "user123",
                        "192.168.1.100", "User logged in successfully", "/login");
    logger.logEvent(event1);
    
    // Test Case 2: Failed Login Attempt
    SecurityEvent event2(EventType::LOGIN_FAILURE, Severity::MEDIUM, "admin",
                        "10.0.0.50", "Failed login attempt - invalid password", "/login");
    logger.logEvent(event2);
    
    // Test Case 3: Unauthorized Access Attempt
    SecurityEvent event3(EventType::UNAUTHORIZED_ACCESS, Severity::HIGH, "user456",
                        "172.16.0.200", "Attempted to access restricted resource", "/admin/config");
    logger.logEvent(event3);
    
    // Test Case 4: Critical Privilege Escalation
    SecurityEvent event4(EventType::PRIVILEGE_ESCALATION, Severity::CRITICAL, "user789",
                        "203.0.113.42", "Attempted privilege escalation detected", "/admin/users");
    logger.logEvent(event4);
    
    // Test Case 5: Data Modification
    SecurityEvent event5(EventType::DATA_MODIFICATION, Severity::MEDIUM, "user123",
                        "192.168.1.100", "Modified user profile settings", "/profile/edit");
    logger.logEvent(event5);
    
    // Generate and display report
    logger.generateReport();
    
    cout << "Recent Events:" << endl;
    auto recent = logger.getRecentEvents(3);
    for (const auto& event : recent) {
        cout << event->toString() << endl;
    }
    
    return 0;
}

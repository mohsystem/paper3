
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_STRING 256
#define MAX_EVENTS 1000
#define MAX_TYPES 50

typedef struct {
    char eventType[MAX_STRING];
    char severity[MAX_STRING];
    char source[MAX_STRING];
    char description[MAX_STRING];
    time_t timestamp;
} SecurityEvent;

typedef struct {
    char type[MAX_STRING];
    int count;
} EventCount;

typedef struct {
    SecurityEvent events[MAX_EVENTS];
    int eventCount;
    EventCount eventTypes[MAX_TYPES];
    int typeCount;
    EventCount severityTypes[MAX_TYPES];
    int severityCount;
} SecurityMonitor;

void initMonitor(SecurityMonitor* monitor) {
    monitor->eventCount = 0;
    monitor->typeCount = 0;
    monitor->severityCount = 0;
}

void logEvent(SecurityMonitor* monitor, const char* eventType, const char* severity, 
              const char* source, const char* description) {
    if (monitor->eventCount >= MAX_EVENTS) {
        printf("Event log is full!\\n");
        return;
    }
    
    SecurityEvent* event = &monitor->events[monitor->eventCount];
    strncpy(event->eventType, eventType, MAX_STRING - 1);
    strncpy(event->severity, severity, MAX_STRING - 1);
    strncpy(event->source, source, MAX_STRING - 1);
    strncpy(event->description, description, MAX_STRING - 1);
    event->timestamp = time(NULL);
    
    // Update event type count
    int found = 0;
    for (int i = 0; i < monitor->typeCount; i++) {
        if (strcmp(monitor->eventTypes[i].type, eventType) == 0) {
            monitor->eventTypes[i].count++;
            found = 1;
            break;
        }
    }
    if (!found && monitor->typeCount < MAX_TYPES) {
        strncpy(monitor->eventTypes[monitor->typeCount].type, eventType, MAX_STRING - 1);
        monitor->eventTypes[monitor->typeCount].count = 1;
        monitor->typeCount++;
    }
    
    // Update severity count
    found = 0;
    for (int i = 0; i < monitor->severityCount; i++) {
        if (strcmp(monitor->severityTypes[i].type, severity) == 0) {
            monitor->severityTypes[i].count++;
            found = 1;
            break;
        }
    }
    if (!found && monitor->severityCount < MAX_TYPES) {
        strncpy(monitor->severityTypes[monitor->severityCount].type, severity, MAX_STRING - 1);
        monitor->severityTypes[monitor->severityCount].count = 1;
        monitor->severityCount++;
    }
    
    char timeStr[100];
    struct tm* timeinfo = localtime(&event->timestamp);
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", timeinfo);
    
    printf("Logged: [%s] [%s] [%s] %s - %s\\n", 
           timeStr, severity, eventType, source, description);
    
    monitor->eventCount++;
}

int getEventsBySeverity(SecurityMonitor* monitor, const char* severity, 
                        SecurityEvent* results, int maxResults) {
    int count = 0;
    for (int i = 0; i < monitor->eventCount && count < maxResults; i++) {
        if (strcmp(monitor->events[i].severity, severity) == 0) {
            results[count++] = monitor->events[i];
        }
    }
    return count;
}

int getEventsByType(SecurityMonitor* monitor, const char* eventType, 
                    SecurityEvent* results, int maxResults) {
    int count = 0;
    for (int i = 0; i < monitor->eventCount && count < maxResults; i++) {
        if (strcmp(monitor->events[i].eventType, eventType) == 0) {
            results[count++] = monitor->events[i];
        }
    }
    return count;
}

void generateReport(SecurityMonitor* monitor) {
    printf("\\n=== Security Event Report ===\\n");
    printf("Total Events: %d\\n", monitor->eventCount);
    printf("\\nEvents by Type:\\n");
    for (int i = 0; i < monitor->typeCount; i++) {
        printf("  %s: %d\\n", monitor->eventTypes[i].type, monitor->eventTypes[i].count);
    }
    printf("\\nEvents by Severity:\\n");
    for (int i = 0; i < monitor->severityCount; i++) {
        printf("  %s: %d\\n", monitor->severityTypes[i].type, monitor->severityTypes[i].count);
    }
    printf("============================\\n\\n");
}

int main() {
    SecurityMonitor monitor;
    initMonitor(&monitor);
    
    // Test Case 1: Log authentication failure
    printf("Test Case 1: Authentication Failure\\n");
    logEvent(&monitor, "AUTH_FAILURE", "HIGH", "192.168.1.100", "Failed login attempt for user 'admin'");
    
    // Test Case 2: Log successful login
    printf("\\nTest Case 2: Successful Login\\n");
    logEvent(&monitor, "AUTH_SUCCESS", "INFO", "192.168.1.101", "User 'john.doe' logged in successfully");
    
    // Test Case 3: Log unauthorized access attempt
    printf("\\nTest Case 3: Unauthorized Access\\n");
    logEvent(&monitor, "UNAUTHORIZED_ACCESS", "CRITICAL", "192.168.1.105", "Attempt to access restricted resource");
    
    // Test Case 4: Log multiple events and generate report
    printf("\\nTest Case 4: Multiple Events\\n");
    logEvent(&monitor, "AUTH_FAILURE", "HIGH", "192.168.1.102", "Brute force attack detected");
    logEvent(&monitor, "DATA_ACCESS", "MEDIUM", "192.168.1.103", "Sensitive data accessed by user 'jane.smith'");
    logEvent(&monitor, "AUTH_FAILURE", "HIGH", "192.168.1.100", "Another failed login for 'admin'");
    
    generateReport(&monitor);
    
    // Test Case 5: Query events by severity
    printf("Test Case 5: Query High Severity Events\\n");
    SecurityEvent highSeverityEvents[MAX_EVENTS];
    int highCount = getEventsBySeverity(&monitor, "HIGH", highSeverityEvents, MAX_EVENTS);
    printf("High Severity Events: %d\\n", highCount);
    for (int i = 0; i < highCount; i++) {
        char timeStr[100];
        struct tm* timeinfo = localtime(&highSeverityEvents[i].timestamp);
        strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", timeinfo);
        printf("  [%s] [%s] [%s] %s - %s\\n", 
               timeStr, highSeverityEvents[i].severity, highSeverityEvents[i].eventType, 
               highSeverityEvents[i].source, highSeverityEvents[i].description);
    }
    
    printf("\\nQuery Events by Type (AUTH_FAILURE):\\n");
    SecurityEvent authFailures[MAX_EVENTS];
    int authCount = getEventsByType(&monitor, "AUTH_FAILURE", authFailures, MAX_EVENTS);
    printf("Authentication Failures: %d\\n", authCount);
    
    return 0;
}

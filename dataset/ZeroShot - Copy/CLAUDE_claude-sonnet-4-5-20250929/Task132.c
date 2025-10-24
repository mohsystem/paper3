
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_STRING_LEN 256
#define MAX_EVENTS 100
#define LOG_FILE "security_events.log"

typedef enum {
    LOGIN_SUCCESS,
    LOGIN_FAILURE,
    UNAUTHORIZED_ACCESS,
    DATA_MODIFICATION,
    PRIVILEGE_ESCALATION,
    SYSTEM_ERROR
} EventType;

typedef enum {
    LOW,
    MEDIUM,
    HIGH,
    CRITICAL
} Severity;

typedef struct {
    char eventId[20];
    time_t timestamp;
    EventType eventType;
    Severity severity;
    char userId[MAX_STRING_LEN];
    char ipAddress[50];
    char description[MAX_STRING_LEN];
    char resourceAccessed[MAX_STRING_LEN];
} SecurityEvent;

typedef struct {
    SecurityEvent events[MAX_EVENTS];
    int count;
    int maxBufferSize;
} SecurityLogger;

const char* eventTypeToString(EventType type) {
    switch(type) {
        case LOGIN_SUCCESS: return "LOGIN_SUCCESS";
        case LOGIN_FAILURE: return "LOGIN_FAILURE";
        case UNAUTHORIZED_ACCESS: return "UNAUTHORIZED_ACCESS";
        case DATA_MODIFICATION: return "DATA_MODIFICATION";
        case PRIVILEGE_ESCALATION: return "PRIVILEGE_ESCALATION";
        case SYSTEM_ERROR: return "SYSTEM_ERROR";
        default: return "UNKNOWN";
    }
}

const char* severityToString(Severity sev) {
    switch(sev) {
        case LOW: return "LOW";
        case MEDIUM: return "MEDIUM";
        case HIGH: return "HIGH";
        case CRITICAL: return "CRITICAL";
        default: return "UNKNOWN";
    }
}

void generateEventId(char* eventId) {
    sprintf(eventId, "EVT-%08X", rand() & 0xFFFFFFFF);
}

void sanitizeInput(char* output, const char* input, size_t maxLen) {
    if (input == NULL || strlen(input) == 0) {
        strncpy(output, "N/A", maxLen - 1);
        output[maxLen - 1] = '\\0';
        return;
    }
    
    size_t len = strlen(input);
    size_t copyLen = len < maxLen - 1 ? len : maxLen - 1;
    
    for (size_t i = 0; i < copyLen; i++) {
        if (input[i] == '\\r' || input[i] == '\\n') {
            output[i] = ' ';
        } else {
            output[i] = input[i];
        }
    }
    output[copyLen] = '\\0';
}

void createSecurityEvent(SecurityEvent* event, EventType type, Severity sev,
                        const char* userId, const char* ipAddress,
                        const char* description, const char* resourceAccessed) {
    generateEventId(event->eventId);
    event->timestamp = time(NULL);
    event->eventType = type;
    event->severity = sev;
    
    sanitizeInput(event->userId, userId, MAX_STRING_LEN);
    sanitizeInput(event->ipAddress, ipAddress, 50);
    sanitizeInput(event->description, description, MAX_STRING_LEN);
    sanitizeInput(event->resourceAccessed, resourceAccessed, MAX_STRING_LEN);
}

void eventToString(const SecurityEvent* event, char* buffer, size_t bufferSize) {
    char timeBuffer[100];
    strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%d %H:%M:%S", 
             localtime(&event->timestamp));
    
    snprintf(buffer, bufferSize,
             "[%s] [%s] [%s] [%s] User: %s | IP: %s | Resource: %s | Description: %s",
             timeBuffer, event->eventId, severityToString(event->severity),
             eventTypeToString(event->eventType), event->userId, event->ipAddress,
             event->resourceAccessed, event->description);
}

void initLogger(SecurityLogger* logger, int maxBufferSize) {
    logger->count = 0;
    logger->maxBufferSize = maxBufferSize;
}

void writeToFile(const SecurityEvent* event) {
    FILE* file = fopen(LOG_FILE, "a");
    if (file != NULL) {
        char buffer[1024];
        eventToString(event, buffer, sizeof(buffer));
        fprintf(file, "%s\\n", buffer);
        fclose(file);
    } else {
        fprintf(stderr, "Error writing to log file\\n");
    }
}

void logEvent(SecurityLogger* logger, const SecurityEvent* event) {
    if (logger->count < MAX_EVENTS) {
        logger->events[logger->count] = *event;
        logger->count++;
    }
    
    writeToFile(event);
    
    if (event->severity == HIGH || event->severity == CRITICAL) {
        char buffer[1024];
        eventToString(event, buffer, sizeof(buffer));
        printf("ALERT: %s\\n", buffer);
    }
    
    if (logger->count >= logger->maxBufferSize) {
        logger->count = 0;
    }
}

void generateReport(const SecurityLogger* logger) {
    int eventCounts[6] = {0};
    int severityCounts[4] = {0};
    
    for (int i = 0; i < logger->count; i++) {
        eventCounts[logger->events[i].eventType]++;
        severityCounts[logger->events[i].severity]++;
    }
    
    printf("\\n=== Security Event Report ===\\n");
    printf("Total Events: %d\\n", logger->count);
    printf("\\nEvents by Type:\\n");
    for (int i = 0; i < 6; i++) {
        if (eventCounts[i] > 0) {
            printf("  %s: %d\\n", eventTypeToString((EventType)i), eventCounts[i]);
        }
    }
    printf("\\nEvents by Severity:\\n");
    for (int i = 0; i < 4; i++) {
        if (severityCounts[i] > 0) {
            printf("  %s: %d\\n", severityToString((Severity)i), severityCounts[i]);
        }
    }
    printf("============================\\n\\n");
}

int main() {
    srand(time(NULL));
    SecurityLogger logger;
    initLogger(&logger, 100);
    
    SecurityEvent event1, event2, event3, event4, event5;
    
    // Test Case 1: Login Success
    createSecurityEvent(&event1, LOGIN_SUCCESS, LOW, "user123",
                       "192.168.1.100", "User logged in successfully", "/login");
    logEvent(&logger, &event1);
    
    // Test Case 2: Failed Login Attempt
    createSecurityEvent(&event2, LOGIN_FAILURE, MEDIUM, "admin",
                       "10.0.0.50", "Failed login attempt - invalid password", "/login");
    logEvent(&logger, &event2);
    
    // Test Case 3: Unauthorized Access Attempt
    createSecurityEvent(&event3, UNAUTHORIZED_ACCESS, HIGH, "user456",
                       "172.16.0.200", "Attempted to access restricted resource", "/admin/config");
    logEvent(&logger, &event3);
    
    // Test Case 4: Critical Privilege Escalation
    createSecurityEvent(&event4, PRIVILEGE_ESCALATION, CRITICAL, "user789",
                       "203.0.113.42", "Attempted privilege escalation detected", "/admin/users");
    logEvent(&logger, &event4);
    
    // Test Case 5: Data Modification
    createSecurityEvent(&event5, DATA_MODIFICATION, MEDIUM, "user123",
                       "192.168.1.100", "Modified user profile settings", "/profile/edit");
    logEvent(&logger, &event5);
    
    // Generate and display report
    generateReport(&logger);
    
    printf("Recent Events:\\n");
    int start = logger.count > 3 ? logger.count - 3 : 0;
    for (int i = start; i < logger.count; i++) {
        char buffer[1024];
        eventToString(&logger.events[i], buffer, sizeof(buffer));
        printf("%s\\n", buffer);
    }
    
    return 0;
}

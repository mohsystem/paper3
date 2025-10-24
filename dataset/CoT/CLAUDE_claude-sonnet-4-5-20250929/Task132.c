
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <regex.h>

#define MAX_LOG_SIZE 10000
#define MAX_EVENT_LENGTH 1000
#define MAX_EVENTS 10000
#define LOG_FILE "security_events.log"
#define TIMESTAMP_SIZE 20
#define IP_SIZE 16

typedef struct {
    char timestamp[TIMESTAMP_SIZE];
    char eventType[MAX_EVENT_LENGTH];
    char description[MAX_EVENT_LENGTH];
    char severity[20];
    char sourceIP[IP_SIZE];
} SecurityEvent;

typedef struct {
    SecurityEvent events[MAX_EVENTS];
    int eventCount;
    int eventCounters[100];
    char eventTypes[100][MAX_EVENT_LENGTH];
    int typeCount;
} Task132;

void sanitizeInput(const char* input, char* output, int maxLen) {
    if (input == NULL || output == NULL) {
        output[0] = '\\0';
        return;
    }
    
    int len = strlen(input);
    if (len > maxLen) {
        len = maxLen;
    }
    
    int j = 0;
    for (int i = 0; i < len && j < maxLen - 1; i++) {
        char c = input[i];
        if (c != '\\n' && c != '\\r' && c != '\\t' && c >= 32 && c <= 126) {
            output[j++] = c;
        } else {
            output[j++] = ' ';
        }
    }
    output[j] = '\\0';
    
    // Trim whitespace
    while (j > 0 && output[j-1] == ' ') {
        output[--j] = '\\0';
    }
}

int isValidSeverity(const char* severity) {
    if (severity == NULL) return 0;
    
    char upper[20];
    int i = 0;
    while (severity[i] && i < 19) {
        upper[i] = toupper(severity[i]);
        i++;
    }
    upper[i] = '\\0';
    
    return strcmp(upper, "LOW") == 0 || strcmp(upper, "MEDIUM") == 0 ||
           strcmp(upper, "HIGH") == 0 || strcmp(upper, "CRITICAL") == 0;
}

int isValidIP(const char* ip) {
    if (ip == NULL || strlen(ip) == 0) {
        return 0;
    }
    
    regex_t regex;
    int ret = regcomp(&regex, 
        "^([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])\\\\.([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])\\\\.([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])\\\\.([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])$",
        REG_EXTENDED);
    
    if (ret != 0) {
        return 0;
    }
    
    ret = regexec(&regex, ip, 0, NULL, 0);
    regfree(&regex);
    
    return ret == 0;
}

void getCurrentTimestamp(char* buffer, int size) {
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", t);
}

void toUpperCase(char* str) {
    for (int i = 0; str[i]; i++) {
        str[i] = toupper(str[i]);
    }
}

void writeToFile(const SecurityEvent* event) {
    FILE* file = fopen(LOG_FILE, "a");
    if (file != NULL) {
        fprintf(file, "[%s] [%s] [%s] %s - Source: %s\\n",
                event->timestamp, event->severity, event->eventType,
                event->description, 
                strlen(event->sourceIP) > 0 ? event->sourceIP : "N/A");
        fclose(file);
    }
}

void initTask132(Task132* monitor) {
    monitor->eventCount = 0;
    monitor->typeCount = 0;
    memset(monitor->eventCounters, 0, sizeof(monitor->eventCounters));
}

int logEvent(Task132* monitor, const char* eventType, const char* description,
             const char* severity, const char* sourceIP) {
    // Validate inputs
    if (eventType == NULL || strlen(eventType) == 0 ||
        description == NULL || strlen(description) == 0 ||
        severity == NULL || strlen(severity) == 0) {
        return 0;
    }
    
    char cleanEventType[MAX_EVENT_LENGTH];
    char cleanDescription[MAX_EVENT_LENGTH];
    char cleanSeverity[20];
    char cleanSourceIP[IP_SIZE];
    
    // Sanitize all inputs
    sanitizeInput(eventType, cleanEventType, MAX_EVENT_LENGTH);
    sanitizeInput(description, cleanDescription, MAX_EVENT_LENGTH);
    sanitizeInput(severity, cleanSeverity, 20);
    sanitizeInput(sourceIP, cleanSourceIP, IP_SIZE);
    
    // Validate severity level
    if (!isValidSeverity(cleanSeverity)) {
        return 0;
    }
    
    // Validate IP format if provided
    if (strlen(cleanSourceIP) > 0 && !isValidIP(cleanSourceIP)) {
        return 0;
    }
    
    // Convert severity to uppercase
    toUpperCase(cleanSeverity);
    
    // Prevent overflow
    if (monitor->eventCount >= MAX_EVENTS) {
        return 0;
    }
    
    SecurityEvent* event = &monitor->events[monitor->eventCount];
    
    getCurrentTimestamp(event->timestamp, TIMESTAMP_SIZE);
    strncpy(event->eventType, cleanEventType, MAX_EVENT_LENGTH - 1);
    event->eventType[MAX_EVENT_LENGTH - 1] = '\\0';
    strncpy(event->description, cleanDescription, MAX_EVENT_LENGTH - 1);
    event->description[MAX_EVENT_LENGTH - 1] = '\\0';
    strncpy(event->severity, cleanSeverity, 19);
    event->severity[19] = '\\0';
    strncpy(event->sourceIP, cleanSourceIP, IP_SIZE - 1);
    event->sourceIP[IP_SIZE - 1] = '\\0';
    
    monitor->eventCount++;
    
    // Update counters
    int found = 0;
    for (int i = 0; i < monitor->typeCount; i++) {
        if (strcmp(monitor->eventTypes[i], cleanEventType) == 0) {
            monitor->eventCounters[i]++;
            found = 1;
            break;
        }
    }
    
    if (!found && monitor->typeCount < 100) {
        strncpy(monitor->eventTypes[monitor->typeCount], cleanEventType, MAX_EVENT_LENGTH - 1);
        monitor->eventTypes[monitor->typeCount][MAX_EVENT_LENGTH - 1] = '\\0';
        monitor->eventCounters[monitor->typeCount] = 1;
        monitor->typeCount++;
    }
    
    // Write to file
    writeToFile(event);
    
    return 1;
}

int getEventCount(Task132* monitor, const char* eventType) {
    char cleanEventType[MAX_EVENT_LENGTH];
    sanitizeInput(eventType, cleanEventType, MAX_EVENT_LENGTH);
    
    for (int i = 0; i < monitor->typeCount; i++) {
        if (strcmp(monitor->eventTypes[i], cleanEventType) == 0) {
            return monitor->eventCounters[i];
        }
    }
    return 0;
}

void printEvent(const SecurityEvent* event) {
    printf("[%s] [%s] [%s] %s - Source: %s\\n",
           event->timestamp, event->severity, event->eventType,
           event->description,
           strlen(event->sourceIP) > 0 ? event->sourceIP : "N/A");
}

int main() {
    Task132 monitor;
    initTask132(&monitor);
    
    printf("=== Security Event Logging System ===\\n\\n");
    
    // Test Case 1: Valid login failure event
    printf("Test Case 1: Login Failure\\n");
    int result1 = logEvent(&monitor, "LOGIN_FAILURE", "Failed login attempt for user admin",
                          "HIGH", "192.168.1.100");
    printf("Event logged: %s\\n", result1 ? "true" : "false");
    
    // Test Case 2: Valid unauthorized access event
    printf("\\nTest Case 2: Unauthorized Access\\n");
    int result2 = logEvent(&monitor, "UNAUTHORIZED_ACCESS",
                          "Attempted access to restricted resource",
                          "CRITICAL", "10.0.0.50");
    printf("Event logged: %s\\n", result2 ? "true" : "false");
    
    // Test Case 3: Invalid severity level (should fail)
    printf("\\nTest Case 3: Invalid Severity\\n");
    int result3 = logEvent(&monitor, "DATA_BREACH", "Sensitive data accessed",
                          "INVALID_LEVEL", "172.16.0.1");
    printf("Event logged: %s\\n", result3 ? "true" : "false");
    
    // Test Case 4: Log injection attempt (should be sanitized)
    printf("\\nTest Case 4: Log Injection Prevention\\n");
    int result4 = logEvent(&monitor, "MALICIOUS_INPUT",
                          "Test\\nFake Log Entry\\r\\nAnother Line",
                          "MEDIUM", "192.168.1.50");
    printf("Event logged: %s\\n", result4 ? "true" : "false");
    
    // Test Case 5: Valid firewall block event
    printf("\\nTest Case 5: Firewall Block\\n");
    int result5 = logEvent(&monitor, "FIREWALL_BLOCK",
                          "Blocked suspicious traffic",
                          "LOW", "203.0.113.10");
    printf("Event logged: %s\\n", result5 ? "true" : "false");
    
    // Display all logged events
    printf("\\n=== All Logged Security Events ===\\n");
    for (int i = 0; i < monitor.eventCount; i++) {
        printEvent(&monitor.events[i]);
    }
    
    // Display event counts
    printf("\\n=== Event Counts ===\\n");
    printf("LOGIN_FAILURE: %d\\n", getEventCount(&monitor, "LOGIN_FAILURE"));
    printf("UNAUTHORIZED_ACCESS: %d\\n", getEventCount(&monitor, "UNAUTHORIZED_ACCESS"));
    
    return 0;
}

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

#define MAX_LOGS 50
#define MAX_TIMESTAMP_LEN 20
#define MAX_TYPE_LEN 25
#define MAX_SEVERITY_LEN 10
#define MAX_DESC_LEN 150

// Represents a single security event
typedef struct {
    char timestamp[MAX_TIMESTAMP_LEN];
    char eventType[MAX_TYPE_LEN];
    char severity[MAX_SEVERITY_LEN];
    char description[MAX_DESC_LEN];
} SecurityEvent;

// Manages the collection of security events
typedef struct {
    SecurityEvent events[MAX_LOGS];
    int count;
} SecurityLogger;

/**
 * @brief Initializes the security logger.
 * @param logger Pointer to the SecurityLogger instance.
 */
void init_logger(SecurityLogger* logger) {
    logger->count = 0;
}

/**
 * @brief Creates and logs a new security event.
 * @param logger Pointer to the SecurityLogger instance.
 * @param eventType The type of the event.
 * @param severity The severity level of the event.
 * @param description A detailed description of the event.
 */
void log_event(SecurityLogger* logger, const char* eventType, const char* severity, const char* description) {
    if (logger->count >= MAX_LOGS) {
        printf("Log is full. Cannot add more events.\n");
        return;
    }

    SecurityEvent* newEvent = &logger->events[logger->count];

    // Get current time
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(newEvent->timestamp, sizeof(newEvent->timestamp), "%Y-%m-%d %H:%M:%S", t);

    // Copy event data
    strncpy(newEvent->eventType, eventType, MAX_TYPE_LEN - 1);
    newEvent->eventType[MAX_TYPE_LEN - 1] = '\0';

    strncpy(newEvent->severity, severity, MAX_SEVERITY_LEN - 1);
    newEvent->severity[MAX_SEVERITY_LEN - 1] = '\0';

    strncpy(newEvent->description, description, MAX_DESC_LEN - 1);
    newEvent->description[MAX_DESC_LEN - 1] = '\0';

    logger->count++;

    // In a real system, this might also write to a file or a remote logging service.
    printf("Logged: [%s] [%s] [%s] %s\n", newEvent->timestamp, newEvent->severity, newEvent->eventType, newEvent->description);
}

/**
 * @brief Displays all the logged events to the console.
 * @param logger Pointer to the SecurityLogger instance.
 */
void display_logs(const SecurityLogger* logger) {
    printf("\n--- Security Event Log ---\n");
    if (logger->count == 0) {
        printf("No events logged.\n");
    } else {
        for (int i = 0; i < logger->count; i++) {
            const SecurityEvent* event = &logger->events[i];
            printf("[%s] [%s] [%s] %s\n", event->timestamp, event->severity, event->eventType, event->description);
        }
    }
    printf("--------------------------\n\n");
}


int main() {
    SecurityLogger logger;
    init_logger(&logger);

    printf("--- Running 5 Test Cases ---\n");

    // Test Case 1: Successful login
    log_event(&logger, "LOGIN_SUCCESS", "INFO", "User 'admin' logged in successfully from IP 192.168.1.10");

    // Test Case 2: Failed login attempt
    log_event(&logger, "LOGIN_FAILURE", "WARNING", "Failed login attempt for user 'root' from IP 10.0.0.5");

    // Test Case 3: Unauthorized access attempt
    log_event(&logger, "UNAUTHORIZED_ACCESS", "CRITICAL", "Attempt to access '/etc/shadow' by user 'guest'");

    // Test Case 4: File modification
    log_event(&logger, "FILE_MODIFICATION", "INFO", "User 'dev' modified '/var/www/index.html'");

    // Test Case 5: Firewall rule change
    log_event(&logger, "FIREWALL_UPDATE", "WARNING", "Firewall rule added to block port 8080");

    // Display all logged events at the end
    display_logs(&logger);

    return 0;
}
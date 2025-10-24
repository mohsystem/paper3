#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Note: For a multi-threaded C application, you must use a mutex (e.g., from pthreads)
// to lock around the file I/O operations in logEvent to ensure thread safety.
// This example is single-threaded and does not include mutexes for simplicity.

#define LOG_FILE_NAME "security.log"
#define MAX_LOG_MESSAGE_SIZE 1024
#define MAX_DESCRIPTION_SIZE 512

typedef enum {
    INFO,
    WARNING,
    CRITICAL
} Severity;

typedef enum {
    LOGIN_SUCCESS,
    LOGIN_FAILURE,
    FILE_ACCESS,
    CONFIG_CHANGE,
    UNAUTHORIZED_ATTEMPT
} EventType;

// Helper function to convert Severity enum to string
const char* severityToString(Severity s) {
    switch (s) {
        case INFO: return "INFO";
        case WARNING: return "WARNING";
        case CRITICAL: return "CRITICAL";
        default: return "UNKNOWN";
    }
}

// Helper function to convert EventType enum to string
const char* eventTypeToString(EventType et) {
    switch (et) {
        case LOGIN_SUCCESS: return "LOGIN_SUCCESS";
        case LOGIN_FAILURE: return "LOGIN_FAILURE";
        case FILE_ACCESS: return "FILE_ACCESS";
        case CONFIG_CHANGE: return "CONFIG_CHANGE";
        case UNAUTHORIZED_ATTEMPT: return "UNAUTHORIZED_ATTEMPT";
        default: return "UNKNOWN";
    }
}

/**
 * Simulates sending an alert for critical events to standard output.
 * @param alertMessage The message for the alert.
 */
void sendAlert(const char* alertMessage) {
    printf("ALERT! Critical security event detected:\n");
    printf(">> %s\n", alertMessage);
}

/**
 * Sanitizes a string by replacing newline and carriage return characters with '_'.
 * The string is modified in place.
 * @param str The string to sanitize.
 */
void sanitize_string(char* str) {
    if (str == NULL) return;
    for (char* p = str; *p; ++p) {
        if (*p == '\n' || *p == '\r') {
            *p = '_';
        }
    }
}

/**
 * Logs a security event to a file.
 * Note: The log file 'security.log' should have its permissions set to be
 * readable/writable only by the application user (e.g., chmod 600).
 *
 * @param severity The severity level of the event.
 * @param eventType The type of the event.
 * @param sourceIp The source IP address.
 * @param description A detailed description of the event.
 */
void logEvent(Severity severity, EventType eventType, const char* sourceIp, const char* description) {
    FILE *logFile = fopen(LOG_FILE_NAME, "a");
    if (logFile == NULL) {
        perror("Error: Failed to open log file");
        return;
    }

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    if (t == NULL) {
        fprintf(stderr, "Error: localtime failed.\n");
        fclose(logFile);
        return;
    }
    char timestamp[20];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", t);

    // Sanitize description to prevent log injection.
    char sanitizedDescription[MAX_DESCRIPTION_SIZE];
    strncpy(sanitizedDescription, description, MAX_DESCRIPTION_SIZE - 1);
    sanitizedDescription[MAX_DESCRIPTION_SIZE - 1] = '\0'; // Ensure null termination
    sanitize_string(sanitizedDescription);
    
    char logMessage[MAX_LOG_MESSAGE_SIZE];
    // Use snprintf for safe string formatting to prevent buffer overflows.
    int written = snprintf(logMessage, MAX_LOG_MESSAGE_SIZE, "[%s] [%s] [%s] [SRC_IP: %s] - %s",
             timestamp,
             severityToString(severity),
             eventTypeToString(eventType),
             sourceIp,
             sanitizedDescription);
    
    if (written > 0 && written < MAX_LOG_MESSAGE_SIZE) {
        fprintf(logFile, "%s\n", logMessage);
    } else {
        fprintf(stderr, "Error: Failed to format log message or buffer was too small.\n");
    }

    fclose(logFile);

    // Monitor for critical events and send alerts.
    if (severity == CRITICAL) {
        sendAlert(logMessage);
    }
}

int main(void) {
    printf("Logging security events. Check 'security.log' for details.\n");
    printf("---------------------------------------------------------\n");

    // Test Case 1: Successful login
    logEvent(INFO, LOGIN_SUCCESS, "192.168.1.10", "User 'admin' logged in successfully.");

    // Test Case 2: Failed login attempt
    logEvent(WARNING, LOGIN_FAILURE, "10.0.0.5", "Failed login attempt for user 'guest'.");

    // Test Case 3: Critical - Multiple failed attempts (Simulated)
    logEvent(CRITICAL, LOGIN_FAILURE, "203.0.113.45", "Multiple (5) failed login attempts for user 'root'. IP blocked.");

    // Test Case 4: Configuration change
    logEvent(INFO, CONFIG_CHANGE, "127.0.0.1", "User 'admin' updated 'firewall.conf'.");

    // Test Case 5: Unauthorized access attempt
    logEvent(CRITICAL, UNAUTHORIZED_ATTEMPT, "198.51.100.88", "Attempt to access '/api/admin' endpoint without authentication.");

    printf("---------------------------------------------------------\n");
    printf("Test cases executed.\n");

    return 0;
}
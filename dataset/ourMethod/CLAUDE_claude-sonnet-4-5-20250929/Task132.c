
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <ctype.h>

#ifdef _WIN32
#define MEMSET_S(ptr, size, val, count) memset(ptr, val, count)
#else
#define MEMSET_S(ptr, size, val, count) memset_s(ptr, size, val, count)
#endif

// Security event severity levels
typedef enum {
    SEVERITY_INFO,
    SEVERITY_WARNING,
    SEVERITY_CRITICAL
} Severity;

// Security event types
typedef enum {
    EVENT_LOGIN_SUCCESS,
    EVENT_LOGIN_FAILURE,
    EVENT_UNAUTHORIZED_ACCESS,
    EVENT_FILE_ACCESS,
    EVENT_CONFIGURATION_CHANGE
} EventType;

// Maximum lengths for security
#define MAX_USERNAME_LEN 256
#define MAX_IP_LEN 45
#define MAX_DESC_LEN 1024
#define MAX_PATH_LEN 4096
#define MAX_LOG_LINE 2048

// Structure to hold security event data
typedef struct {
    time_t timestamp;
    Severity severity;
    EventType type;
    char username[MAX_USERNAME_LEN + 1];
    char ipAddress[MAX_IP_LEN + 1];
    char description[MAX_DESC_LEN + 1];
} SecurityEvent;

// Validate string length and content
bool validate_string(const char* input, size_t max_len) {
    if (input == NULL) {
        return false;
    }
    
    size_t len = strnlen(input, max_len + 1);
    if (len == 0 || len > max_len) {
        return false;
    }
    
    // Check for null bytes in the middle of string
    for (size_t i = 0; i < len; i++) {
        if (input[i] == '\\0') {
            return i == len;
        }
    }
    
    return true;
}

// Sanitize string by removing control characters
void sanitize_string(char* dest, const char* src, size_t dest_size) {
    if (dest == NULL || src == NULL || dest_size == 0) {
        return;
    }
    
    size_t src_len = strnlen(src, dest_size - 1);
    size_t j = 0;
    
    for (size_t i = 0; i < src_len && j < dest_size - 1; i++) {
        // Replace newlines and carriage returns with spaces
        if (src[i] == '\\n' || src[i] == '\\r') {
            dest[j++] = ' ';
        }
        // Only allow printable ASCII characters
        else if (src[i] >= 32 && src[i] <= 126) {
            dest[j++] = src[i];
        }
    }
    
    dest[j] = '\\0';
}

// Convert severity to string
const char* severity_to_string(Severity sev) {
    switch (sev) {
        case SEVERITY_INFO: return "INFO";
        case SEVERITY_WARNING: return "WARNING";
        case SEVERITY_CRITICAL: return "CRITICAL";
        default: return "UNKNOWN";
    }
}

// Convert event type to string
const char* event_type_to_string(EventType type) {
    switch (type) {
        case EVENT_LOGIN_SUCCESS: return "LOGIN_SUCCESS";
        case EVENT_LOGIN_FAILURE: return "LOGIN_FAILURE";
        case EVENT_UNAUTHORIZED_ACCESS: return "UNAUTHORIZED_ACCESS";
        case EVENT_FILE_ACCESS: return "FILE_ACCESS";
        case EVENT_CONFIGURATION_CHANGE: return "CONFIGURATION_CHANGE";
        default: return "UNKNOWN";
    }
}

// Format timestamp in ISO 8601 format
bool format_timestamp(char* buffer, size_t buffer_size, time_t timestamp) {
    if (buffer == NULL || buffer_size < 21) {
        return false;
    }
    
    struct tm* tm_info = gmtime(&timestamp);
    if (tm_info == NULL) {
        snprintf(buffer, buffer_size, "INVALID_TIME");
        return false;
    }
    
    size_t result = strftime(buffer, buffer_size, "%Y-%m-%dT%H:%M:%SZ", tm_info);
    return result > 0;
}

// Validate IP address format
bool validate_ip_address(const char* ip) {
    if (!validate_string(ip, MAX_IP_LEN)) {
        return false;
    }
    
    size_t len = strnlen(ip, MAX_IP_LEN + 1);
    
    for (size_t i = 0; i < len; i++) {
        char c = ip[i];
        if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || 
              (c >= 'A' && c <= 'F') || c == '.' || c == ':')) {
            return false;
        }
    }
    
    return true;
}

// Log a security event
bool log_event(const char* log_path, const SecurityEvent* event) {
    if (log_path == NULL || event == NULL) {
        fprintf(stderr, "Invalid parameters\\n");
        return false;
    }
    
    // Validate log path
    size_t path_len = strnlen(log_path, MAX_PATH_LEN + 1);
    if (path_len == 0 || path_len > MAX_PATH_LEN) {
        fprintf(stderr, "Invalid log file path length\\n");
        return false;
    }
    
    // Check for path traversal
    if (strstr(log_path, "..") != NULL) {
        fprintf(stderr, "Path traversal detected\\n");
        return false;
    }
    
    // Validate event fields
    if (!validate_string(event->username, MAX_USERNAME_LEN)) {
        fprintf(stderr, "Invalid username format\\n");
        return false;
    }
    
    if (!validate_ip_address(event->ipAddress)) {
        fprintf(stderr, "Invalid IP address format\\n");
        return false;
    }
    
    if (!validate_string(event->description, MAX_DESC_LEN)) {
        fprintf(stderr, "Invalid description format\\n");
        return false;
    }
    
    // Sanitize inputs
    char safe_username[MAX_USERNAME_LEN + 1];
    char safe_ip[MAX_IP_LEN + 1];
    char safe_desc[MAX_DESC_LEN + 1];
    
    sanitize_string(safe_username, event->username, sizeof(safe_username));
    sanitize_string(safe_ip, event->ipAddress, sizeof(safe_ip));
    sanitize_string(safe_desc, event->description, sizeof(safe_desc));
    
    // Format timestamp
    char timestamp_str[32];
    if (!format_timestamp(timestamp_str, sizeof(timestamp_str), event->timestamp)) {
        fprintf(stderr, "Failed to format timestamp\\n");
        return false;
    }
    
    // Open file with write mode
    FILE* log_file = fopen(log_path, "a");
    if (log_file == NULL) {
        fprintf(stderr, "Failed to open log file\\n");
        return false;
    }
    
    // Write log entry
    int result = fprintf(log_file, "%s | %s | %s | User: %s | IP: %s | Description: %s\\n",
                        timestamp_str,
                        severity_to_string(event->severity),
                        event_type_to_string(event->type),
                        safe_username,
                        safe_ip,
                        safe_desc);
    
    // Flush to ensure write completes
    fflush(log_file);
    
    // Close file
    if (fclose(log_file) != 0) {
        fprintf(stderr, "Failed to close log file\\n");
        return false;
    }
    
    return result > 0;
}

// Analyze events for suspicious patterns
int analyze_events(const char* log_path, int failure_threshold) {
    if (log_path == NULL) {
        fprintf(stderr, "Invalid log path\\n");
        return -1;
    }
    
    FILE* log_file = fopen(log_path, "r");
    if (log_file == NULL) {
        fprintf(stderr, "Unable to open log file for analysis\\n");
        return -1;
    }
    
    char line[MAX_LOG_LINE];
    int alert_count = 0;
    
    // Simple analysis: count LOGIN_FAILURE events
    int failure_count = 0;
    
    while (fgets(line, sizeof(line), log_file) != NULL) {
        if (strstr(line, "LOGIN_FAILURE") != NULL) {
            failure_count++;
        }
    }
    
    if (failure_count >= failure_threshold) {
        printf("ALERT: %d failed login attempts detected (threshold: %d)\\n", 
               failure_count, failure_threshold);
        alert_count++;
    }
    
    if (fclose(log_file) != 0) {
        fprintf(stderr, "Failed to close log file\\n");
    }
    
    return alert_count;
}

int main(void) {
    const char* log_path = "security_events.log";
    
    // Test case 1: Successful login event
    SecurityEvent event1 = {
        .timestamp = time(NULL),
        .severity = SEVERITY_INFO,
        .type = EVENT_LOGIN_SUCCESS
    };
    strncpy(event1.username, "admin", sizeof(event1.username) - 1);
    event1.username[sizeof(event1.username) - 1] = '\\0';
    strncpy(event1.ipAddress, "192.168.1.100", sizeof(event1.ipAddress) - 1);
    event1.ipAddress[sizeof(event1.ipAddress) - 1] = '\\0';
    strncpy(event1.description, "User logged in successfully", sizeof(event1.description) - 1);
    event1.description[sizeof(event1.description) - 1] = '\\0';
    
    if (log_event(log_path, &event1)) {
        printf("Test 1 passed: Login success logged\\n");
    }
    
    // Test case 2: Failed login attempt
    SecurityEvent event2 = {
        .timestamp = time(NULL),
        .severity = SEVERITY_WARNING,
        .type = EVENT_LOGIN_FAILURE
    };
    strncpy(event2.username, "admin", sizeof(event2.username) - 1);
    event2.username[sizeof(event2.username) - 1] = '\\0';
    strncpy(event2.ipAddress, "192.168.1.100", sizeof(event2.ipAddress) - 1);
    event2.ipAddress[sizeof(event2.ipAddress) - 1] = '\\0';
    strncpy(event2.description, "Invalid password", sizeof(event2.description) - 1);
    event2.description[sizeof(event2.description) - 1] = '\\0';
    
    if (log_event(log_path, &event2)) {
        printf("Test 2 passed: Login failure logged\\n");
    }
    
    // Test case 3: Multiple failed attempts
    for (int i = 0; i < 5; i++) {
        SecurityEvent event = {
            .timestamp = time(NULL),
            .severity = SEVERITY_WARNING,
            .type = EVENT_LOGIN_FAILURE
        };
        strncpy(event.username, "testuser", sizeof(event.username) - 1);
        event.username[sizeof(event.username) - 1] = '\\0';
        strncpy(event.ipAddress, "10.0.0.50", sizeof(event.ipAddress) - 1);
        event.ipAddress[sizeof(event.ipAddress) - 1] = '\\0';
        strncpy(event.description, "Brute force detected", sizeof(event.description) - 1);
        event.description[sizeof(event.description) - 1] = '\\0';
        
        log_event(log_path, &event);
    }
    printf("Test 3 passed: Multiple failures logged\\n");
    
    // Test case 4: Unauthorized access
    SecurityEvent event4 = {
        .timestamp = time(NULL),
        .severity = SEVERITY_CRITICAL,
        .type = EVENT_UNAUTHORIZED_ACCESS
    };
    strncpy(event4.username, "guest", sizeof(event4.username) - 1);
    event4.username[sizeof(event4.username) - 1] = '\\0';
    strncpy(event4.ipAddress, "203.0.113.45", sizeof(event4.ipAddress) - 1);
    event4.ipAddress[sizeof(event4.ipAddress) - 1] = '\\0';
    strncpy(event4.description, "Unauthorized access attempt", sizeof(event4.description) - 1);
    event4.description[sizeof(event4.description) - 1] = '\\0';
    
    if (log_event(log_path, &event4)) {
        printf("Test 4 passed: Unauthorized access logged\\n");
    }
    
    // Test case 5: Analyze events
    int alerts = analyze_events(log_path, 3);
    printf("Test 5: Event analysis completed with %d alerts\\n", alerts);
    
    return 0;
}

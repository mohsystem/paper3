#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

// For file locking on POSIX systems (Linux, macOS).
// This code is not compatible with Windows.
#include <fcntl.h>
#include <unistd.h>
#include <sys/file.h>
#include <errno.h>

#define LOG_FILE_NAME "security_events.log"
#define MAX_DESCRIPTION_LENGTH 256
#define MAX_LOG_LINE_LENGTH 512
#define TIME_BUFFER_SIZE 30

typedef enum {
    INFO,
    WARN,
    CRITICAL
} Severity;

const char* severity_to_string(Severity s) {
    switch (s) {
        case INFO: return "INFO";
        case WARN: return "WARN";
        case CRITICAL: return "CRITICAL";
    }
    return "UNKNOWN";
}

// Sanitizes a string in-place by replacing newline characters.
void sanitize_input(char* input) {
    if (input == NULL) return;
    for (size_t i = 0; i < strlen(input); ++i) {
        if (input[i] == '\n' || input[i] == '\r') {
            input[i] = '_';
        }
    }
}

bool log_event(const char* event_type, Severity severity, const char* description) {
    // 1. Input Validation
    if (event_type == NULL || description == NULL) {
        fprintf(stderr, "Error: Event details cannot be NULL.\n");
        return false;
    }
    if (strlen(description) > MAX_DESCRIPTION_LENGTH) {
        fprintf(stderr, "Error: Description exceeds maximum length of %d\n", MAX_DESCRIPTION_LENGTH);
        return false;
    }

    // 2. Sanitize (on copies to avoid modifying caller's data)
    char sanitized_event_type[128];
    char sanitized_description[MAX_DESCRIPTION_LENGTH + 1];
    
    strncpy(sanitized_event_type, event_type, sizeof(sanitized_event_type) - 1);
    sanitized_event_type[sizeof(sanitized_event_type) - 1] = '\0';
    
    strncpy(sanitized_description, description, sizeof(sanitized_description) - 1);
    sanitized_description[sizeof(sanitized_description) - 1] = '\0';

    sanitize_input(sanitized_event_type);
    sanitize_input(sanitized_description);

    // 3. Format log entry
    time_t now = time(NULL);
    struct tm* tm_info = gmtime(&now);
    char time_buf[TIME_BUFFER_SIZE];
    strftime(time_buf, sizeof(time_buf), "%Y-%m-%dT%H:%M:%SZ", tm_info);

    char log_entry[MAX_LOG_LINE_LENGTH];
    int written_len = snprintf(log_entry, sizeof(log_entry), "[%s] [%s] [%s]: %s\n",
             time_buf, severity_to_string(severity), sanitized_event_type, sanitized_description);

    if (written_len < 0 || (size_t)written_len >= sizeof(log_entry)) {
        fprintf(stderr, "Error: Failed to format log entry or entry too long.\n");
        return false;
    }

    // 4. Write to file with exclusive lock
    int fd = open(LOG_FILE_NAME, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd == -1) {
        perror("Failed to open log file");
        return false;
    }

    if (flock(fd, LOCK_EX) == -1) {
        perror("Failed to lock log file");
        close(fd);
        return false;
    }

    ssize_t bytes_written = write(fd, log_entry, strlen(log_entry));
    if (bytes_written < 0 || (size_t)bytes_written != strlen(log_entry)) {
        perror("Failed to write to log file");
    }

    flock(fd, LOCK_UN);
    close(fd);

    return bytes_written > 0;
}

void read_logs() {
    FILE* file = fopen(LOG_FILE_NAME, "r");
    if (file == NULL) {
        if (errno != ENOENT) { // File not found is not an error for a new log
            perror("Could not open log file for reading");
        }
        return;
    }

    int fd = fileno(file);
    if (flock(fd, LOCK_SH) == -1) {
        perror("Failed to acquire shared lock");
        fclose(file);
        return;
    }

    char line[MAX_LOG_LINE_LENGTH];
    while (fgets(line, sizeof(line), file) != NULL) {
        printf("%s", line);
    }

    flock(fd, LOCK_UN);
    fclose(file);
}


int main() {
    // Clean up previous log file for a fresh run
    remove(LOG_FILE_NAME);

    printf("--- Running Security Logger Test Cases ---\n");
    
    // Test Case 1: Successful login event
    printf("\n1. Logging a successful login event...\n");
    log_event("LOGIN_SUCCESS", INFO, "User 'admin' logged in from 192.168.1.1");

    // Test Case 2: Failed login attempt
    printf("\n2. Logging a failed login attempt...\n");
    log_event("LOGIN_FAILURE", WARN, "Failed login for user 'guest' from 10.0.0.5");

    // Test Case 3: Critical system event
    printf("\n3. Logging a critical event...\n");
    log_event("CONFIG_MODIFIED", CRITICAL, "Critical system file '/etc/shadow' was modified.");

    // Test Case 4: Attempted log injection
    printf("\n4. Logging an event with newline characters (should be sanitized)...\n");
    log_event("INPUT_VALIDATION_FAIL", WARN, "User input contained malicious payload:\n<script>alert(1)</script>");

    // Test Case 5: Event description too long
    printf("\n5. Logging an event with a description that is too long (should fail)...\n");
    char long_description[MAX_DESCRIPTION_LENGTH + 2];
    memset(long_description, 'D', MAX_DESCRIPTION_LENGTH + 1);
    long_description[MAX_DESCRIPTION_LENGTH + 1] = '\0';
    log_event("DATA_OVERFLOW", WARN, long_description);

    // Read and display all logs
    printf("\n--- Current Security Logs ---\n");
    read_logs();
    printf("--- End of Logs ---\n");
    
    return 0;
}
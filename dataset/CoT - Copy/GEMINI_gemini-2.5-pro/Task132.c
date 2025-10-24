#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

// Define fixed sizes for char arrays to prevent buffer overflows
#define MAX_LOGS 100
#define STR_LEN 64
#define DESC_LEN 128
#define FAILED_LOGIN_THRESHOLD 3
#define TIME_WINDOW_SECONDS 60

// Structure for a security event
typedef struct {
    long long timestamp; // Unix timestamp in seconds
    char eventType[STR_LEN];
    char userId[STR_LEN];
    char sourceIp[STR_LEN];
    char description[DESC_LEN];
} SecurityEvent;

// Global log storage. In a real application, this would not be global.
// A pointer to a struct containing the logs and count would be passed around.
SecurityEvent eventLog[MAX_LOGS];
int logCount = 0;

// Utility to sleep for a given number of milliseconds
void sleep_ms(int milliseconds) {
#ifdef _WIN32
    Sleep(milliseconds);
#else
    usleep(milliseconds * 1000);
#endif
}

/**
 * @brief Sanitizes a string by removing newline characters.
 * This is a simplified sanitizer. A robust one would handle more cases.
 * @param str The string to sanitize.
 */
void sanitize(char *str) {
    if (str == NULL) return;
    char *p = str;
    while (*p) {
        if (*p == '\n' || *p == '\r') {
            *p = '_';
        }
        p++;
    }
}

/**
 * @brief Safely copies a string, ensuring null termination and preventing overflow.
 * @param dest The destination buffer.
 * @param src The source string.
 * @param size The size of the destination buffer.
 */
void safe_strncpy(char* dest, const char* src, size_t size) {
    if (src == NULL) {
        dest[0] = '\0';
        return;
    }
    strncpy(dest, src, size - 1);
    dest[size - 1] = '\0'; // Ensure null-termination
}

/**
 * @brief Logs a security event.
 * @return true on success, false if the log is full.
 */
bool logEvent(const char* eventType, const char* userId, const char* sourceIp, const char* description) {
    if (logCount >= MAX_LOGS) {
        printf("Error: Log is full. Cannot add new event.\n");
        return false;
    }

    SecurityEvent* newEvent = &eventLog[logCount];
    newEvent->timestamp = time(NULL);

    // Sanitize and copy data securely
    char tempEventType[STR_LEN], tempUserId[STR_LEN], tempSourceIp[STR_LEN], tempDesc[DESC_LEN];
    safe_strncpy(tempEventType, eventType, STR_LEN);
    safe_strncpy(tempUserId, userId, STR_LEN);
    safe_strncpy(tempSourceIp, sourceIp, STR_LEN);
    safe_strncpy(tempDesc, description, DESC_LEN);

    sanitize(tempEventType);
    sanitize(tempUserId);
    sanitize(tempSourceIp);
    sanitize(tempDesc);
    
    strcpy(newEvent->eventType, tempEventType);
    strcpy(newEvent->userId, tempUserId);
    strcpy(newEvent->sourceIp, tempSourceIp);
    strcpy(newEvent->description, tempDesc);

    logCount++;
    return true;
}

void printLogs() {
    printf("--- Security Event Log ---\n");
    if (logCount == 0) {
        printf("No events logged.\n");
    } else {
        for (int i = 0; i < logCount; i++) {
            printf("Event(time=%lld, type='%s', user='%s', ip='%s', desc='%s')\n",
                   eventLog[i].timestamp, eventLog[i].eventType, eventLog[i].userId,
                   eventLog[i].sourceIp, eventLog[i].description);
        }
    }
    printf("--------------------------\n\n");
}

/**
 * @brief Simple comparison function for qsort.
 */
int compare_long(const void* a, const void* b) {
    long long la = *(const long long*)a;
    long long lb = *(const long long*)b;
    if (la < lb) return -1;
    if (la > lb) return 1;
    return 0;
}

void analyzeLogs() {
    printf("--- Security Analysis Report ---\n");
    
    // This is a simplified analysis due to C's lack of built-in hash maps.
    // We iterate to find unique IPs with failed logins first.
    char uniqueIps[MAX_LOGS][STR_LEN];
    int uniqueIpCount = 0;
    bool alertsFound = false;

    for (int i = 0; i < logCount; i++) {
        if (strcmp(eventLog[i].eventType, "LOGIN_FAILURE") == 0) {
            bool found = false;
            for (int j = 0; j < uniqueIpCount; j++) {
                if (strcmp(uniqueIps[j], eventLog[i].sourceIp) == 0) {
                    found = true;
                    break;
                }
            }
            if (!found && uniqueIpCount < MAX_LOGS) {
                safe_strncpy(uniqueIps[uniqueIpCount++], eventLog[i].sourceIp, STR_LEN);
            }
        }
    }

    // Now, for each unique IP, collect timestamps and check for alerts.
    for (int i = 0; i < uniqueIpCount; i++) {
        long long timestamps[MAX_LOGS];
        int tsCount = 0;
        for (int j = 0; j < logCount; j++) {
            if (strcmp(eventLog[j].eventType, "LOGIN_FAILURE") == 0 &&
                strcmp(eventLog[j].sourceIp, uniqueIps[i]) == 0) {
                timestamps[tsCount++] = eventLog[j].timestamp;
            }
        }

        if (tsCount >= FAILED_LOGIN_THRESHOLD) {
            qsort(timestamps, tsCount, sizeof(long long), compare_long);
            for (int k = 0; k <= tsCount - FAILED_LOGIN_THRESHOLD; k++) {
                if (timestamps[k + FAILED_LOGIN_THRESHOLD - 1] - timestamps[k] <= TIME_WINDOW_SECONDS) {
                    printf("ALERT: Potential brute-force attack detected from IP: %s. %d failed attempts within %d seconds.\n",
                           uniqueIps[i], FAILED_LOGIN_THRESHOLD, TIME_WINDOW_SECONDS);
                    alertsFound = true;
                    break; // One alert per IP
                }
            }
        }
    }

    if (!alertsFound) {
        printf("No suspicious activity detected.\n");
    }
    printf("------------------------------\n\n");
}

int main() {
    // --- Test Case 1: Normal Operations ---
    printf(">>> Running Test Case 1: Normal Operations\n");
    logEvent("LOGIN_SUCCESS", "alice", "192.168.1.10", "User alice logged in successfully.");
    sleep_ms(1000);
    logEvent("FILE_ACCESS", "alice", "192.168.1.10", "User alice accessed /data/report.docx.");
    sleep_ms(1000);
    logEvent("LOGOUT", "alice", "192.168.1.10", "User alice logged out.");
    printLogs();
    analyzeLogs();

    // --- Test Case 2: Brute-Force Attack ---
    printf(">>> Running Test Case 2: Brute-Force Attack\n");
    logEvent("LOGIN_FAILURE", "admin", "203.0.113.55", "Failed login attempt for user admin.");
    sleep_ms(500);
    logEvent("LOGIN_FAILURE", "root", "203.0.113.55", "Failed login attempt for user root.");
    sleep_ms(500);
    logEvent("LOGIN_FAILURE", "admin", "203.0.113.55", "Failed login attempt for user admin.");
    sleep_ms(500);
    logEvent("LOGIN_FAILURE", "guest", "203.0.113.55", "Failed login attempt for user guest.");
    printLogs();
    analyzeLogs();

    // --- Test Case 3: Distributed Attack (should not trigger alert) ---
    printf(">>> Running Test Case 3: Distributed Low-and-Slow Attack\n");
    logEvent("LOGIN_FAILURE", "user", "198.51.100.1", "Failed login.");
    sleep_ms(2000);
    logEvent("LOGIN_FAILURE", "user", "198.51.100.2", "Failed login.");
    sleep_ms(2000);
    logEvent("LOGIN_FAILURE", "user", "198.51.100.3", "Failed login.");
    printLogs();
    analyzeLogs();
    
    // --- Test Case 4: Mixed Activity & Successful Intrusion ---
    printf(">>> Running Test Case 4: Mixed Activity & Eventual Success\n");
    logEvent("LOGIN_FAILURE", "bob", "10.0.0.42", "Failed login: invalid password.");
    sleep_ms(1000);
    logEvent("LOGIN_FAILURE", "bob", "10.0.0.42", "Failed login: invalid password.");
    sleep_ms(1000);
    logEvent("LOGIN_SUCCESS", "bob", "10.0.0.42", "User bob logged in successfully.");
    printLogs();
    analyzeLogs();

    // --- Test Case 5: Permission Escalation Attempt ---
    printf(">>> Running Test Case 5: Permission Escalation Attempt\n");
    logEvent("LOGIN_SUCCESS", "charlie", "172.16.0.12", "User charlie logged in successfully.");
    sleep_ms(1000);
    logEvent("PERMISSION_DENIED", "charlie", "172.16.0.12", "Access denied for /etc/shadow.");
    printLogs();
    analyzeLogs();

    return 0;
}
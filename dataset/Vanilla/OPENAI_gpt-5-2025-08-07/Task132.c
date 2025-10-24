#include <stdio.h>
#include <string.h>

#define MAX_EVENTS 1000
#define TYPE_LEN 64
#define SOURCE_LEN 64
#define MESSAGE_LEN 256

typedef struct {
    long long timestamp;
    char type[TYPE_LEN];
    int severity;
    char source[SOURCE_LEN];
    char message[MESSAGE_LEN];
} Event;

typedef struct {
    Event events[MAX_EVENTS];
    int count;
} SecurityLogger;

// Initialize logger
void initLogger(SecurityLogger* logger) {
    logger->count = 0;
}

// Log an event. Returns index of stored event, or -1 if full.
int logEvent(SecurityLogger* logger, long long timestamp, const char* type, int severity, const char* source, const char* message) {
    if (logger->count >= MAX_EVENTS) return -1;
    Event* e = &logger->events[logger->count];
    e->timestamp = timestamp;
    // Copy with truncation and ensure null-termination
    strncpy(e->type, type, TYPE_LEN - 1); e->type[TYPE_LEN - 1] = '\0';
    e->severity = severity;
    strncpy(e->source, source, SOURCE_LEN - 1); e->source[SOURCE_LEN - 1] = '\0';
    strncpy(e->message, message, MESSAGE_LEN - 1); e->message[MESSAGE_LEN - 1] = '\0';
    logger->count += 1;
    return logger->count - 1;
}

int totalEvents(const SecurityLogger* logger) {
    return logger->count;
}

int countByType(const SecurityLogger* logger, const char* type) {
    int c = 0;
    for (int i = 0; i < logger->count; ++i) {
        if (strcmp(logger->events[i].type, type) == 0) c++;
    }
    return c;
}

int countBySeverityAtLeast(const SecurityLogger* logger, int minSeverity) {
    int c = 0;
    for (int i = 0; i < logger->count; ++i) {
        if (logger->events[i].severity >= minSeverity) c++;
    }
    return c;
}

int countFromSource(const SecurityLogger* logger, const char* source) {
    int c = 0;
    for (int i = 0; i < logger->count; ++i) {
        if (strcmp(logger->events[i].source, source) == 0) c++;
    }
    return c;
}

// Count events in [start, end] inclusive
int countInRange(const SecurityLogger* logger, long long start, long long end) {
    int c = 0;
    for (int i = 0; i < logger->count; ++i) {
        long long ts = logger->events[i].timestamp;
        if (ts >= start && ts <= end) c++;
    }
    return c;
}

// Count events in (endTime - windowSeconds, endTime]
int countInWindow(const SecurityLogger* logger, long long endTime, long long windowSeconds) {
    long long startExclusive = endTime - windowSeconds;
    int c = 0;
    for (int i = 0; i < logger->count; ++i) {
        long long ts = logger->events[i].timestamp;
        if (ts > startExclusive && ts <= endTime) c++;
    }
    return c;
}

int isAnomalous(const SecurityLogger* logger, long long endTime, long long windowSeconds, int threshold) {
    return countInWindow(logger, endTime, windowSeconds) > threshold;
}

int main() {
    SecurityLogger logger;
    initLogger(&logger);
    long long t0 = 1700000000LL;

    logEvent(&logger, t0 + 10, "LOGIN_FAILURE", 3, "web", "Invalid password");
    logEvent(&logger, t0 + 20, "ACCESS_DENIED", 4, "db", "Unauthorized table access");
    logEvent(&logger, t0 + 30, "MALWARE_DETECTED", 5, "av", "Trojan found");
    logEvent(&logger, t0 + 40, "LOGIN_SUCCESS", 1, "web", "User login");
    logEvent(&logger, t0 + 50, "PORT_SCAN", 4, "ids", "Scan detected");
    logEvent(&logger, t0 + 55, "LOGIN_FAILURE", 3, "web", "Invalid password");
    logEvent(&logger, t0 + 58, "LOGIN_FAILURE", 3, "vpn", "Failed login");
    logEvent(&logger, t0 + 120, "DLP_ALERT", 4, "dlp", "Sensitive data transfer");
    logEvent(&logger, t0 + 125, "MALWARE_DETECTED", 5, "av", "Worm found");
    logEvent(&logger, t0 + 180, "ACCESS_DENIED", 2, "db", "Restricted view");

    printf("Test1 totalEvents: %d\n", totalEvents(&logger));
    printf("Test2 countByType(LOGIN_FAILURE): %d\n", countByType(&logger, "LOGIN_FAILURE"));
    printf("Test3 countBySeverityAtLeast(4): %d\n", countBySeverityAtLeast(&logger, 4));
    printf("Test4 countInRange[t0, t0+60]: %d\n", countInRange(&logger, t0, t0 + 60));
    printf("Test5 isAnomalous(window=60, end=t0+60, threshold=6): %s\n", isAnomalous(&logger, t0 + 60, 60, 6) ? "true" : "false");

    return 0;
}
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdbool.h>

#define MAX_EVENTS 1000
#define MAX_SOURCE_LEN 64
#define MAX_MESSAGE_LEN 256
#define MAX_TYPE_LEN 20
#define MAX_SEVERITY_LEN 10

static const int64_t MIN_TS = 946684800;   // 2000-01-01
static const int64_t MAX_TS = 4102444800;  // 2100-01-01

static const char* ALLOWED_TYPES[] = {
    "LOGIN_SUCCESS", "LOGIN_FAILURE", "ACCESS_DENIED", "DATA_EXPORT", "MALWARE_ALERT", "CONFIG_CHANGE"
};
static const size_t ALLOWED_TYPES_COUNT = 6;

static const char* ALLOWED_SEVERITIES[] = {
    "INFO", "LOW", "MEDIUM", "HIGH", "CRITICAL"
};
static const size_t ALLOWED_SEVERITIES_COUNT = 5;

typedef struct {
    uint64_t id;
    int64_t timestampSec;
    char type[MAX_TYPE_LEN + 1];
    char severity[MAX_SEVERITY_LEN + 1];
    char source[MAX_SOURCE_LEN + 1];
    char message[MAX_MESSAGE_LEN + 1];
} Event;

typedef struct {
    Event events[MAX_EVENTS];
    size_t capacity;
    size_t count;
    uint64_t nextId;
} EventLogger;

static bool is_printable_ascii(char c) {
    return (c >= 32 && c <= 126);
}

static bool is_alnum(char c) {
    return ((c >= 'A' && c <= 'Z') ||
            (c >= 'a' && c <= 'z') ||
            (c >= '0' && c <= '9'));
}

static bool is_allowed_type(const char* t) {
    if (!t) return false;
    for (size_t i = 0; i < ALLOWED_TYPES_COUNT; i++) {
        if (strcmp(t, ALLOWED_TYPES[i]) == 0) return true;
    }
    return false;
}

static bool is_allowed_severity(const char* s) {
    if (!s) return false;
    for (size_t i = 0; i < ALLOWED_SEVERITIES_COUNT; i++) {
        if (strcmp(s, ALLOWED_SEVERITIES[i]) == 0) return true;
    }
    return false;
}

static bool is_valid_timestamp(int64_t ts) {
    return ts >= MIN_TS && ts <= MAX_TS;
}

static bool is_valid_source(const char* s, size_t minLen, size_t maxLen) {
    if (!s) return false;
    size_t len = strnlen(s, maxLen + 1);
    if (len < minLen || len > maxLen) return false;
    for (size_t i = 0; i < len; i++) {
        char c = s[i];
        if (!(is_alnum(c) || c == '.' || c == '_' || c == '-')) return false;
    }
    return true;
}

static bool is_valid_message(const char* s, size_t minLen, size_t maxLen) {
    if (!s) return false;
    size_t len = strnlen(s, maxLen + 1);
    if (len < minLen || len > maxLen) return false;
    for (size_t i = 0; i < len; i++) {
        if (!is_printable_ascii(s[i])) return false;
    }
    return true;
}

static bool is_valid_keyword(const char* s, size_t minLen, size_t maxLen) {
    return is_valid_message(s, minLen, maxLen);
}

static void to_ascii_lower_inplace(char* s) {
    if (!s) return;
    for (size_t i = 0; s[i] != '\0'; i++) {
        char c = s[i];
        if (c >= 'A' && c <= 'Z') {
            s[i] = (char)(c + 32);
        }
    }
}

static void init_logger(EventLogger* lg, size_t capacity) {
    if (!lg) return;
    if (capacity < 1 || capacity > MAX_EVENTS) capacity = MAX_EVENTS;
    lg->capacity = capacity;
    lg->count = 0;
    lg->nextId = 1;
}

static bool log_event(EventLogger* lg, const char* type, const char* severity, const char* source, const char* message, int64_t ts) {
    if (!lg) return false;
    if (!is_allowed_type(type) || !is_allowed_severity(severity)) return false;
    if (!is_valid_timestamp(ts)) return false;
    if (!is_valid_source(source, 1, MAX_SOURCE_LEN)) return false;
    if (!is_valid_message(message, 1, MAX_MESSAGE_LEN)) return false;

    // If at capacity, drop oldest (shift left)
    if (lg->count >= lg->capacity) {
        for (size_t i = 1; i < lg->count; i++) {
            lg->events[i - 1] = lg->events[i];
        }
        lg->count = lg->capacity - 1;
    }

    Event ev;
    ev.id = lg->nextId++;
    ev.timestampSec = ts;
    // Safe copies with bounds
    snprintf(ev.type, sizeof(ev.type), "%s", type);
    snprintf(ev.severity, sizeof(ev.severity), "%s", severity);
    snprintf(ev.source, sizeof(ev.source), "%s", source);
    snprintf(ev.message, sizeof(ev.message), "%s", message);

    lg->events[lg->count++] = ev;
    return true;
}

static size_t get_events_by_severity(const EventLogger* lg, const char* severity, Event* out, size_t out_cap) {
    if (!lg || !out || out_cap == 0) return 0;
    if (!is_allowed_severity(severity)) return 0;
    size_t n = 0;
    for (size_t i = 0; i < lg->count && n < out_cap; i++) {
        if (strcmp(lg->events[i].severity, severity) == 0) {
            out[n++] = lg->events[i];
        }
    }
    return n;
}

static size_t search_by_keyword(const EventLogger* lg, const char* keyword, Event* out, size_t out_cap) {
    if (!lg || !out || out_cap == 0) return 0;
    if (!is_valid_keyword(keyword, 1, 32)) return 0;

    // Lowercase keyword copy
    char kbuf[33];
    size_t klen = strnlen(keyword, 33);
    if (klen == 0 || klen > 32) return 0;
    snprintf(kbuf, sizeof(kbuf), "%s", keyword);
    to_ascii_lower_inplace(kbuf);

    size_t n = 0;
    for (size_t i = 0; i < lg->count && n < out_cap; i++) {
        // Lowercase message copy (bounded)
        char mbuf[MAX_MESSAGE_LEN + 1];
        snprintf(mbuf, sizeof(mbuf), "%s", lg->events[i].message);
        to_ascii_lower_inplace(mbuf);
        if (strstr(mbuf, kbuf) != NULL) {
            out[n++] = lg->events[i];
        }
    }
    return n;
}

typedef struct {
    // Count for each allowed type and severity
    int typeCounts[6];
    int sevCounts[5];
} Counts;

static Counts count_all(const EventLogger* lg) {
    Counts c;
    for (int i = 0; i < 6; i++) c.typeCounts[i] = 0;
    for (int i = 0; i < 5; i++) c.sevCounts[i] = 0;
    if (!lg) return c;

    for (size_t i = 0; i < lg->count; i++) {
        const Event* e = &lg->events[i];
        for (size_t t = 0; t < ALLOWED_TYPES_COUNT; t++) {
            if (strcmp(e->type, ALLOWED_TYPES[t]) == 0) c.typeCounts[t]++;
        }
        for (size_t s = 0; s < ALLOWED_SEVERITIES_COUNT; s++) {
            if (strcmp(e->severity, ALLOWED_SEVERITIES[s]) == 0) c.sevCounts[s]++;
        }
    }
    return c;
}

static size_t get_recent(const EventLogger* lg, size_t n, Event* out, size_t out_cap) {
    if (!lg || !out) return 0;
    if (n > lg->capacity) return 0;
    size_t take = (n > lg->count) ? lg->count : n;
    if (take > out_cap) take = out_cap;
    for (size_t i = 0; i < take; i++) {
        out[i] = lg->events[lg->count - take + i];
    }
    return take;
}

int main(void) {
    EventLogger logger;
    init_logger(&logger, 100);
    int64_t baseTs = 1700000000;

    // Test 1: valid event
    bool t1 = log_event(&logger, "LOGIN_SUCCESS", "INFO", "authsvc", "User alice logged in", baseTs);
    printf("Test1 success=%s\n", t1 ? "true" : "false");

    // Test 2: invalid type
    bool t2 = log_event(&logger, "UNKNOWN_TYPE", "LOW", "guard", "Unknown event", baseTs + 1);
    printf("Test2 success=%s\n", t2 ? "true" : "false");

    // Test 3: high severity
    bool t3 = log_event(&logger, "MALWARE_ALERT", "CRITICAL", "scanner01", "Malware signature detected", baseTs + 2);
    printf("Test3 success=%s\n", t3 ? "true" : "false");

    // Test 4: search
    (void)log_event(&logger, "ACCESS_DENIED", "MEDIUM", "gateway01", "Access denied to resource /admin", baseTs + 3);
    Event outSearch[10];
    size_t nSearch = search_by_keyword(&logger, "denied", outSearch, 10);
    printf("Test4 search count=%zu\n", nSearch);

    // Test 5: counts and recent
    (void)log_event(&logger, "DATA_EXPORT", "HIGH", "exporter", "Data export initiated", baseTs + 4);
    Counts c = count_all(&logger);
    // Print counts in fixed order to avoid format string issues
    printf("Test5 countByType: ");
    for (size_t i = 0; i < ALLOWED_TYPES_COUNT; i++) {
        printf("%s: %d; ", ALLOWED_TYPES[i], c.typeCounts[i]);
    }
    printf("\n");
    printf("Test5 countBySeverity: ");
    for (size_t i = 0; i < ALLOWED_SEVERITIES_COUNT; i++) {
        printf("%s: %d; ", ALLOWED_SEVERITIES[i], c.sevCounts[i]);
    }
    printf("\n");

    Event recent[3];
    size_t nRecent = get_recent(&logger, 3, recent, 3);
    printf("Recent last 3 size=%zu\n", nRecent);
    return 0;
}
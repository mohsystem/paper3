#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <stdlib.h>

#define MAX_EVENTS 1024
#define MAX_SOURCE 64
#define MAX_TYPE 64
#define MAX_MESSAGE 1024
#define MAX_SOURCES 128
#define MAX_FAILED_PER_SOURCE 1024

typedef enum {
    LEVEL_INFO = 0,
    LEVEL_WARN = 1,
    LEVEL_ERROR = 2,
    LEVEL_CRITICAL = 3,
    LEVEL_ALERT = 4
} Level;

typedef struct {
    long long id;
    long long timestampMs;
    Level level;
    char type[MAX_TYPE];
    char source[MAX_SOURCE];
    char message[MAX_MESSAGE];
} Event;

typedef struct {
    char source[MAX_SOURCE];
    long long timestamps[MAX_FAILED_PER_SOURCE];
    int count;
} FailedSource;

typedef struct {
    Event events[MAX_EVENTS];
    int size;
    int start; // ring buffer start index
    int stats[5];
    FailedSource failedSources[MAX_SOURCES];
    int failedSourceCount;
    long long nextId;
    long long bruteWindowMs;
    int bruteThreshold;
} SecurityLoggerMonitor;

static long long now_ms() {
    struct timespec ts;
#if defined(_WIN32) || defined(_WIN64)
    // Fallback for Windows if clock_gettime not available
    FILETIME ft;
    ULARGE_INTEGER uli;
    GetSystemTimeAsFileTime(&ft);
    uli.LowPart = ft.dwLowDateTime;
    uli.HighPart = ft.dwHighDateTime;
    // Convert from 100-ns intervals since Jan 1,1601 to ms since Unix epoch
    unsigned long long t = (uli.QuadPart - 116444736000000000ULL) / 10000ULL;
    return (long long)t;
#else
    clock_gettime(CLOCK_REALTIME, &ts);
    return (long long)ts.tv_sec * 1000LL + ts.tv_nsec / 1000000LL;
#endif
}

static void sanitize(const char* in, char* out, size_t outSize) {
    if (outSize == 0) return;
    size_t n = 0;
    int lastSpace = 0;
    if (in == NULL) {
        out[0] = '\0';
        return;
    }
    for (size_t i = 0; in[i] != '\0' && n + 1 < outSize; i++) {
        unsigned char c = (unsigned char)in[i];
        if (c == '\r' || c == '\n') {
            if (!lastSpace) {
                out[n++] = ' ';
                lastSpace = 1;
            }
        } else if (c >= 32 && c < 127) {
            out[n++] = (char)c;
            lastSpace = 0;
        } else if (isspace(c)) {
            if (!lastSpace) {
                out[n++] = ' ';
                lastSpace = 1;
            }
        } else {
            // ignore control chars
        }
    }
    out[n] = '\0';
}

static void init_monitor(SecurityLoggerMonitor* mon, long long bruteWindowMs, int bruteThreshold) {
    memset(mon, 0, sizeof(*mon));
    mon->size = 0;
    mon->start = 0;
    for (int i = 0; i < 5; i++) mon->stats[i] = 0;
    mon->failedSourceCount = 0;
    mon->nextId = 1;
    mon->bruteWindowMs = bruteWindowMs;
    mon->bruteThreshold = bruteThreshold > 0 ? bruteThreshold : 1;
}

static void append_event(SecurityLoggerMonitor* mon, const Event* e) {
    int idx;
    if (mon->size < MAX_EVENTS) {
        idx = (mon->start + mon->size) % MAX_EVENTS;
        mon->events[idx] = *e;
        mon->size++;
    } else {
        // evict oldest
        Event rem = mon->events[mon->start];
        if (rem.level >= 0 && rem.level <= LEVEL_ALERT) {
            if (mon->stats[rem.level] > 0) mon->stats[rem.level]--;
        }
        mon->events[mon->start] = *e;
        mon->start = (mon->start + 1) % MAX_EVENTS;
    }
    if (e->level >= 0 && e->level <= LEVEL_ALERT) {
        mon->stats[e->level]++;
    }
}

static FailedSource* get_failed_source(SecurityLoggerMonitor* mon, const char* source) {
    for (int i = 0; i < mon->failedSourceCount; i++) {
        if (strncmp(mon->failedSources[i].source, source, MAX_SOURCE) == 0) {
            return &mon->failedSources[i];
        }
    }
    if (mon->failedSourceCount < MAX_SOURCES) {
        FailedSource* fs = &mon->failedSources[mon->failedSourceCount++];
        memset(fs, 0, sizeof(*fs));
        strncpy(fs->source, source, MAX_SOURCE - 1);
        fs->source[MAX_SOURCE - 1] = '\0';
        fs->count = 0;
        return fs;
    }
    return NULL; // no space to track more sources
}

static void check_bruteforce(SecurityLoggerMonitor* mon, const char* source, long long ts) {
    FailedSource* fs = get_failed_source(mon, source);
    if (!fs) return;
    long long cutoff = ts - mon->bruteWindowMs;
    // remove older timestamps
    int write = 0;
    for (int i = 0; i < fs->count; i++) {
        if (fs->timestamps[i] >= cutoff) {
            fs->timestamps[write++] = fs->timestamps[i];
        }
    }
    fs->count = write;
    if (fs->count >= mon->bruteThreshold) {
        Event alert;
        alert.id = mon->nextId++;
        alert.timestampMs = ts;
        alert.level = LEVEL_ALERT;
        strncpy(alert.type, "alert_bruteforce", MAX_TYPE - 1); alert.type[MAX_TYPE - 1] = '\0';
        strncpy(alert.source, source, MAX_SOURCE - 1); alert.source[MAX_SOURCE - 1] = '\0';
        char msg[256];
        snprintf(msg, sizeof(msg), "Brute force suspected for source=%s count=%d", source, fs->count);
        sanitize(msg, alert.message, MAX_MESSAGE);
        append_event(mon, &alert);
    }
}

static long long log_event(SecurityLoggerMonitor* mon, const char* source, const char* type, Level level, const char* message, long long timestampMs) {
    Event e;
    e.id = mon->nextId++;
    e.timestampMs = timestampMs > 0 ? timestampMs : now_ms();
    sanitize(source, e.source, MAX_SOURCE);
    sanitize(type, e.type, MAX_TYPE);
    sanitize(message, e.message, MAX_MESSAGE);
    e.level = level;
    append_event(mon, &e);

    if (strncmp(e.type, "failed_login", MAX_TYPE) == 0) {
        FailedSource* fs = get_failed_source(mon, e.source);
        if (fs && fs->count < MAX_FAILED_PER_SOURCE) {
            fs->timestamps[fs->count++] = e.timestampMs;
            check_bruteforce(mon, e.source, e.timestampMs);
        }
    }
    return e.id;
}

static int get_recent_events(SecurityLoggerMonitor* mon, int limit, Event* outArr, int outArrSize) {
    if (limit <= 0 || outArrSize <= 0) return 0;
    int count = mon->size < limit ? mon->size : limit;
    for (int i = 0; i < count && i < outArrSize; i++) {
        int idx = (mon->start + mon->size - count + i) % MAX_EVENTS;
        outArr[i] = mon->events[idx];
    }
    return count;
}

static int get_alerts_since(SecurityLoggerMonitor* mon, long long sinceTs, Event* outArr, int outArrSize) {
    int n = 0;
    for (int i = 0; i < mon->size && n < outArrSize; i++) {
        int idx = (mon->start + i) % MAX_EVENTS;
        Event* e = &mon->events[idx];
        if (e->level == LEVEL_ALERT && e->timestampMs >= sinceTs) {
            outArr[n++] = *e;
        }
    }
    return n;
}

static const char* level_to_str(Level l) {
    switch (l) {
        case LEVEL_INFO: return "INFO";
        case LEVEL_WARN: return "WARN";
        case LEVEL_ERROR: return "ERROR";
        case LEVEL_CRITICAL: return "CRITICAL";
        case LEVEL_ALERT: return "ALERT";
        default: return "INFO";
    }
}

int main(void) {
    SecurityLoggerMonitor mon;
    init_monitor(&mon, 5LL * 60LL * 1000LL, 4);
    long long now = now_ms();

    // Test case 1
    log_event(&mon, "user1", "login_success", LEVEL_INFO, "User logged in", now + 10);
    // Test case 2
    log_event(&mon, "userX", "failed_login", LEVEL_ERROR, "Bad password attempt 1", now + 20);
    // Test case 3
    log_event(&mon, "userX", "failed_login", LEVEL_ERROR, "Bad password attempt 2", now + 30);
    // Test case 4
    log_event(&mon, "userX", "failed_login", LEVEL_ERROR, "Bad password attempt 3", now + 40);
    // Test case 5 - triggers alert
    log_event(&mon, "userX", "failed_login", LEVEL_ERROR, "Bad password attempt 4", now + 50);

    printf("Stats:\n");
    printf("INFO %d\n", mon.stats[LEVEL_INFO]);
    printf("WARN %d\n", mon.stats[LEVEL_WARN]);
    printf("ERROR %d\n", mon.stats[LEVEL_ERROR]);
    printf("CRITICAL %d\n", mon.stats[LEVEL_CRITICAL]);
    printf("ALERT %d\n", mon.stats[LEVEL_ALERT]);

    printf("Recent events:\n");
    Event buf[16];
    int n = get_recent_events(&mon, 16, buf, 16);
    for (int i = 0; i < n; i++) {
        printf("Event{id=%lld, ts=%lld, level=%s, type='%s', source='%s', message='%s'}\n",
               buf[i].id, buf[i].timestampMs, level_to_str(buf[i].level), buf[i].type, buf[i].source, buf[i].message);
    }

    printf("Alerts since now:\n");
    Event abuf[16];
    int an = get_alerts_since(&mon, now, abuf, 16);
    for (int i = 0; i < an; i++) {
        printf("Event{id=%lld, ts=%lld, level=%s, type='%s', source='%s', message='%s'}\n",
               abuf[i].id, abuf[i].timestampMs, level_to_str(abuf[i].level), abuf[i].type, abuf[i].source, abuf[i].message);
    }

    return 0;
}
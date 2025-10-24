#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <errno.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <windows.h>
#include <process.h>
#else
#include <unistd.h>
#endif

#define MAX_DETAILS 1024
#define MAX_FIELD 64
#define MAX_EVENTS 1000
#define LOG_PATH "security.log"
#define LOG_ROTATED "security.log.1"
#define MAX_LOG_BYTES (256 * 1024)

typedef enum {
    SEV_INFO = 0,
    SEV_WARNING = 1,
    SEV_ALERT = 2,
    SEV_CRITICAL = 3
} SeverityC;

typedef struct {
    char id[33];
    long long epochMillis;
    char iso8601[32];
    SeverityC severity;
    char source[MAX_FIELD + 1];
    char action[MAX_FIELD + 1];
    char target[MAX_FIELD + 1];
    char status[MAX_FIELD + 1];
    char details[MAX_DETAILS + 1];
} EventC;

static EventC RECENT[MAX_EVENTS];
static int RECENT_COUNT = 0;
static int COUNTS[4] = {0,0,0,0};

static void now_iso(char iso[32], long long *epochMs) {
    struct timespec ts;
#ifdef _WIN32
    FILETIME ft; ULARGE_INTEGER uli;
    GetSystemTimeAsFileTime(&ft);
    uli.LowPart = ft.dwLowDateTime;
    uli.HighPart = ft.dwHighDateTime;
    unsigned long long ns100 = uli.QuadPart - 116444736000000000ULL; // since 1601 to 1970
    unsigned long long ms = ns100 / 10000ULL;
    *epochMs = (long long)ms;
    time_t sec = (time_t)(ms / 1000ULL);
    struct tm gmt;
    gmtime_s(&gmt, &sec);
    int rem = (int)(ms % 1000ULL);
    snprintf(iso, 32, "%04d-%02d-%02dT%02d:%02d:%02d.%03dZ",
             gmt.tm_year + 1900, gmt.tm_mon + 1, gmt.tm_mday,
             gmt.tm_hour, gmt.tm_min, gmt.tm_sec, rem);
#else
    clock_gettime(CLOCK_REALTIME, &ts);
    *epochMs = (long long)ts.tv_sec * 1000LL + ts.tv_nsec / 1000000LL;
    time_t sec = ts.tv_sec;
    struct tm gmt;
    gmtime_r(&sec, &gmt);
    int rem = (int)((*epochMs) % 1000LL);
    snprintf(iso, 32, "%04d-%02d-%02dT%02d:%02d:%02d.%03dZ",
             gmt.tm_year + 1900, gmt.tm_mon + 1, gmt.tm_mday,
             gmt.tm_hour, gmt.tm_min, gmt.tm_sec, rem);
#endif
}

static void sanitize_str(const char *in, char *out, size_t outsz) {
    if (!in) { out[0] = '\0'; return; }
    size_t j = 0;
    for (size_t i = 0; in[i] != '\0' && j + 1 < outsz; i++) {
        unsigned char c = (unsigned char)in[i];
        if (c == '\n' || c == '\r' || c == '\t' || c >= 0x20) {
            out[j++] = (char)c;
        }
        if (j >= MAX_DETAILS) break;
    }
    out[j] = '\0';
    // trim
    // left trim
    size_t start = 0;
    while (out[start] && isspace((unsigned char)out[start])) start++;
    if (start > 0) memmove(out, out + start, strlen(out + start) + 1);
    // right trim
    size_t len = strlen(out);
    while (len > 0 && isspace((unsigned char)out[len - 1])) { out[len - 1] = '\0'; len--; }
}

static void redact_key(char *s, const char *key) {
    char *lower = NULL;
    size_t n = strlen(s);
    lower = (char*)malloc(n + 1);
    if (!lower) return;
    for (size_t i = 0; i < n; i++) lower[i] = (char)tolower((unsigned char)s[i]);
    lower[n] = '\0';

    size_t keylen = strlen(key);
    char *k = (char*)malloc(keylen + 1);
    if (!k) { free(lower); return; }
    for (size_t i = 0; i < keylen; i++) k[i] = (char)tolower((unsigned char)key[i]);
    k[keylen] = '\0';

    size_t i = 0;
    while (i < n) {
        char *pos = strstr(lower + i, k);
        if (!pos) break;
        size_t idx = (size_t)(pos - lower);
        size_t j = idx + keylen;
        while (j < n && isspace((unsigned char)s[j])) j++;
        if (j < n && s[j] == '=') {
            j++;
            while (j < n && isspace((unsigned char)s[j])) j++;
            // replace until delimiter
            size_t start = j;
            while (j < n && s[j] != '&' && s[j] != ',' && s[j] != ';' && !isspace((unsigned char)s[j])) j++;
            // perform replacement in-place by overwriting range [start, j) with "******" and shifting tail left if needed
            const char *mask = "******";
            size_t masklen = strlen(mask);
            size_t tail = n - j;
            if (masklen == (j - start)) {
                memcpy(s + start, mask, masklen);
            } else if (masklen < (j - start)) {
                memcpy(s + start, mask, masklen);
                memmove(s + start + masklen, s + j, tail + 1);
                n = start + masklen + tail;
            } else {
                // mask longer than value: need to expand string; but we limit to buffer size. We'll just truncate mask.
                memcpy(s + start, mask, (j - start));
            }
            // recompute lower
            free(lower);
            n = strlen(s);
            lower = (char*)malloc(n + 1);
            if (!lower) { free(k); return; }
            for (size_t x = 0; x < n; x++) lower[x] = (char)tolower((unsigned char)s[x]);
            lower[n] = '\0';
            i = j; // continue
        } else {
            i = idx + keylen;
        }
    }
    free(lower);
    free(k);
}

static void redact_secrets(char *s) {
    const char *keys[] = {"password", "token", "secret", "apikey", "api_key", "authorization", "auth"};
    for (size_t i = 0; i < sizeof(keys)/sizeof(keys[0]); i++) {
        redact_key(s, keys[i]);
    }
}

static void json_escape(const char *in, char *out, size_t outsz) {
    size_t j = 0;
    for (size_t i = 0; in[i] != '\0' && j + 1 < outsz; i++) {
        unsigned char c = (unsigned char)in[i];
        const char *esc = NULL;
        char buf[7];
        switch (c) {
            case '"': esc = "\\\""; break;
            case '\\': esc = "\\\\"; break;
            case '\b': esc = "\\b"; break;
            case '\f': esc = "\\f"; break;
            case '\n': esc = "\\n"; break;
            case '\r': esc = "\\r"; break;
            case '\t': esc = "\\t"; break;
            default:
                if (c < 0x20) {
                    snprintf(buf, sizeof(buf), "\\u%04x", c);
                    esc = buf;
                }
        }
        if (esc) {
            size_t elen = strlen(esc);
            if (j + elen >= outsz) break;
            memcpy(out + j, esc, elen);
            j += elen;
        } else {
            out[j++] = (char)c;
        }
    }
    out[j] = '\0';
}

static int rotate_if_needed(const char *path, const char *rotated, size_t max_size) {
    struct stat st;
    if (stat(path, &st) == 0) {
        if ((size_t)st.st_size >= max_size) {
            // remove rotated if exists
            remove(rotated);
            if (rename(path, rotated) != 0) {
                // best-effort
            }
        }
    }
    return 0;
}

static void rand_bytes(unsigned char *buf, size_t len) {
#ifdef _WIN32
    HCRYPTPROV hProv = 0;
    if (CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
        CryptGenRandom(hProv, (DWORD)len, buf);
        CryptReleaseContext(hProv, 0);
        return;
    }
#endif
    FILE *f = fopen("/dev/urandom", "rb");
    if (f) {
        fread(buf, 1, len, f);
        fclose(f);
        return;
    }
    // fallback
    unsigned int seed = (unsigned int)time(NULL);
#ifdef _WIN32
    seed ^= (unsigned int)_getpid();
#else
    seed ^= (unsigned int)getpid();
#endif
    for (size_t i = 0; i < len; i++) {
        seed = 1664525U * seed + 1013904223U;
        buf[i] = (unsigned char)((seed >> 24) & 0xFF);
    }
}

static void gen_id_hex(char out[33]) {
    unsigned char b[16];
    rand_bytes(b, sizeof(b));
    static const char *hex = "0123456789abcdef";
    for (int i = 0; i < 16; i++) {
        out[2*i] = hex[(b[i] >> 4) & 0xF];
        out[2*i + 1] = hex[b[i] & 0xF];
    }
    out[32] = '\0';
}

static void to_json(const EventC *e, char *out, size_t outsz) {
    char srcEsc[2*MAX_FIELD+16], actEsc[2*MAX_FIELD+16], tgtEsc[2*MAX_FIELD+16], statEsc[2*MAX_FIELD+16], detEsc[2*MAX_DETAILS+16];
    json_escape(e->source, srcEsc, sizeof(srcEsc));
    json_escape(e->action, actEsc, sizeof(actEsc));
    json_escape(e->target, tgtEsc, sizeof(tgtEsc));
    json_escape(e->status, statEsc, sizeof(statEsc));
    json_escape(e->details, detEsc, sizeof(detEsc));

    snprintf(out, outsz,
        "{\"id\":\"%s\",\"ts\":\"%s\",\"epochMillis\":%lld,\"severity\":\"%s\",\"source\":\"%s\",\"action\":\"%s\",\"target\":\"%s\",\"status\":\"%s\",\"details\":\"%s\"}",
        e->id, e->iso8601, e->epochMillis,
        e->severity == SEV_INFO ? "INFO" : e->severity == SEV_WARNING ? "WARNING" : e->severity == SEV_ALERT ? "ALERT" : "CRITICAL",
        srcEsc, actEsc, tgtEsc, statEsc, detEsc
    );
}

static EventC log_event(const char *source, const char *action, const char *target, const char *status, const char *details, SeverityC severity) {
    EventC ev;
    memset(&ev, 0, sizeof(ev));
    gen_id_hex(ev.id);
    now_iso(ev.iso8601, &ev.epochMillis);
    ev.severity = severity;

    char buf[MAX_DETAILS + 1];

    sanitize_str(source, buf, MAX_FIELD + 1); strncpy(ev.source, buf, MAX_FIELD); ev.source[MAX_FIELD] = '\0';
    sanitize_str(action, buf, MAX_FIELD + 1); strncpy(ev.action, buf, MAX_FIELD); ev.action[MAX_FIELD] = '\0';
    sanitize_str(target, buf, MAX_FIELD + 1); strncpy(ev.target, buf, MAX_FIELD); ev.target[MAX_FIELD] = '\0';
    sanitize_str(status, buf, MAX_FIELD + 1); strncpy(ev.status, buf, MAX_FIELD); ev.status[MAX_FIELD] = '\0';
    sanitize_str(details, ev.details, MAX_DETAILS + 1); redact_secrets(ev.details);

    if (ev.source[0] == '\0') strncpy(ev.source, "unknown", MAX_FIELD);
    if (ev.action[0] == '\0') strncpy(ev.action, "unknown", MAX_FIELD);
    if (ev.target[0] == '\0') strncpy(ev.target, "unknown", MAX_FIELD);
    if (ev.status[0] == '\0') strncpy(ev.status, "unknown", MAX_FIELD);

    rotate_if_needed(LOG_PATH, LOG_ROTATED, MAX_LOG_BYTES);
    FILE *f = fopen(LOG_PATH, "a");
    if (f) {
        char json[2*MAX_DETAILS + 256];
        to_json(&ev, json, sizeof(json));
        fprintf(f, "%s\n", json);
        fclose(f);
    }

    if (RECENT_COUNT < MAX_EVENTS) {
        RECENT[RECENT_COUNT++] = ev;
    } else {
        memmove(&RECENT[0], &RECENT[1], sizeof(EventC) * (MAX_EVENTS - 1));
        RECENT[MAX_EVENTS - 1] = ev;
    }
    if (severity >= SEV_INFO && severity <= SEV_CRITICAL) {
        COUNTS[(int)severity]++;
    }
    return ev;
}

static int get_recent_events(int n, EventC *out, int outcap) {
    if (n <= 0 || outcap <= 0) return 0;
    int k = n < RECENT_COUNT ? n : RECENT_COUNT;
    for (int i = 0; i < k && i < outcap; i++) {
        out[i] = RECENT[RECENT_COUNT - 1 - i];
    }
    return k;
}

static void get_severity_counts(int counts[4]) {
    for (int i = 0; i < 4; i++) counts[i] = COUNTS[i];
}

static int detect_anomalies(int windowSeconds, int thresholdFailuresPerSource, char alerts[][256], int max_alerts) {
    long long nowMs;
    char tmpIso[32];
    now_iso(tmpIso, &nowMs);
    long long floor = nowMs - (long long)(windowSeconds > 0 ? windowSeconds : 1) * 1000LL;

    // simple freq map using arrays
    typedef struct { char source[MAX_FIELD + 1]; int count; } Pair;
    Pair pairs[128];
    int pairCount = 0;

    for (int i = 0; i < RECENT_COUNT; i++) {
        EventC *e = &RECENT[i];
        if (e->epochMillis >= floor && strcasecmp(e->status, "FAILURE") == 0) {
            int found = -1;
            for (int j = 0; j < pairCount; j++) {
                if (strcmp(pairs[j].source, e->source) == 0) { found = j; break; }
            }
            if (found < 0 && pairCount < 128) {
                strncpy(pairs[pairCount].source, e->source, MAX_FIELD);
                pairs[pairCount].source[MAX_FIELD] = '\0';
                pairs[pairCount].count = 1;
                pairCount++;
            } else if (found >= 0) {
                pairs[found].count++;
            }
        }
    }
    int outc = 0;
    for (int i = 0; i < pairCount && outc < max_alerts; i++) {
        if (pairs[i].count >= thresholdFailuresPerSource) {
            snprintf(alerts[outc], 256, "Anomaly: source=%s failures=%d withinLastSec=%d",
                     pairs[i].source, pairs[i].count, windowSeconds);
            outc++;
        }
    }
    return outc;
}

int main(void) {
    log_event("auth-service", "LOGIN", "user:alice", "SUCCESS", "login via SSO", SEV_INFO);
    log_event("webapp1", "LOGIN", "user:bob", "FAILURE", "bad credentials password=supersecret", SEV_WARNING);
    log_event("webapp1", "LOGIN", "user:bob", "FAILURE", "ip=10.0.0.5 token=abcd1234", SEV_WARNING);
    log_event("webapp1", "LOGIN", "user:bob", "FAILURE", "too many attempts", SEV_ALERT);
    log_event("ids", "INTRUSION", "host:db1", "DETECTED", "signature matched secret=mysecret; action=blocked", SEV_CRITICAL);

    int counts[4];
    get_severity_counts(counts);
    printf("Severity counts: {INFO=%d, WARNING=%d, ALERT=%d, CRITICAL=%d}\n", counts[0], counts[1], counts[2], counts[3]);

    EventC last[5];
    int k = get_recent_events(5, last, 5);
    printf("Recent events:\n");
    for (int i = 0; i < k; i++) {
        char json[2*MAX_DETAILS + 256];
        to_json(&last[i], json, sizeof(json));
        printf("%s\n", json);
    }

    char alerts[10][256];
    int a = detect_anomalies(300, 3, alerts, 10);
    printf("Anomalies:\n");
    for (int i = 0; i < a; i++) {
        printf("%s\n", alerts[i]);
    }

    return 0;
}
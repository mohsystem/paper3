#include <stdio.h>
#include <string.h>
#include <ctype.h>

static void trim(const char* s, const char** outStart, const char** outEnd) {
    const char* start = s;
    while (*start && isspace((unsigned char)*start)) start++;
    const char* end = start + strlen(start);
    while (end > start && isspace((unsigned char)*(end - 1))) end--;
    *outStart = start;
    *outEnd = end;
}

static void tolower_str(char* s) {
    for (; *s; ++s) {
        *s = (char)tolower((unsigned char)*s);
    }
}

static int extract_host(const char* url, char* hostBuf, size_t hostBufSize) {
    const char *start, *end;
    trim(url, &start, &end);
    if (start >= end) return 0;

    const char* p = start;
    const char* schemeSep = strstr(p, "://");
    if (schemeSep && schemeSep < end) {
        p = schemeSep + 3;
    } else if ((end - p) >= 2 && p[0] == '/' && p[1] == '/') {
        p = p + 2;
    } else {
        return 0;
    }
    const char* stop = p;
    while (stop < end && *stop != '/' && *stop != '?' && *stop != '#') stop++;
    if (stop <= p) return 0;

    // Authority [p, stop)
    const char* at = NULL;
    for (const char* q = p; q < stop; ++q) {
        if (*q == '@') at = q;
    }
    const char* hostStart = at ? (at + 1) : p;
    const char* hostEnd = stop;

    // IPv6 [ ... ]
    if (hostStart < hostEnd && *hostStart == '[') {
        const char* rb = hostStart;
        while (rb < hostEnd && *rb != ']') rb++;
        if (rb == hostEnd) return 0;
        // host is between hostStart+1 and rb
        size_t len = (size_t)(rb - (hostStart + 1));
        if (len + 1 > hostBufSize) return 0;
        memcpy(hostBuf, hostStart + 1, len);
        hostBuf[len] = '\0';
        tolower_str(hostBuf);
        return 1;
    } else {
        // host[:port]
        const char* colon = hostStart;
        while (colon < hostEnd && *colon != ':') colon++;
        size_t len = (size_t)(colon - hostStart);
        if (len == 0 || len + 1 > hostBufSize) return 0;
        memcpy(hostBuf, hostStart, len);
        hostBuf[len] = '\0';
        tolower_str(hostBuf);
        return 1;
    }
}

int redirect_url(const char* url) {
    if (!url) return 404;
    char host[256];
    if (!extract_host(url, host, sizeof(host))) {
        return 404;
    }
    if (strcmp(host, "example.com") == 0) {
        return 302;
    }
    return 404;
}

int main(void) {
    const char* tests[5] = {
        "https://example.com/page",
        "http://Example.com",
        "https://sub.example.com/home",
        "//example.com/resource",
        "mailto:user@example.com"
    };
    for (int i = 0; i < 5; ++i) {
        int code = redirect_url(tests[i]);
        printf("%s -> %d\n", tests[i], code);
    }
    return 0;
}
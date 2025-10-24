#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int minutes; // minutes since 00:00 in [0, 1439]
} Clock;

static int normalize_minutes(long long totalMinutes) {
    long long m = totalMinutes % (24LL * 60LL);
    if (m < 0) m += 24LL * 60LL;
    return (int)m;
}

// Factory: create a clock at given hour and minute
Clock clock_at(int hour, int minute) {
    Clock c;
    c.minutes = normalize_minutes((long long)hour * 60LL + minute);
    return c;
}

// Add minutes, return new Clock
Clock clock_add(Clock c, long long minutesToAdd) {
    Clock r;
    r.minutes = normalize_minutes((long long)c.minutes + minutesToAdd);
    return r;
}

// Subtract minutes, return new Clock
Clock clock_subtract(Clock c, long long minutesToSubtract) {
    Clock r;
    r.minutes = normalize_minutes((long long)c.minutes - minutesToSubtract);
    return r;
}

// Get hour
int clock_hour(Clock c) {
    return c.minutes / 60;
}

// Get minute
int clock_minute(Clock c) {
    return c.minutes % 60;
}

// Format to "HH:MM" into provided buffer, returns buf
char* clock_to_string(Clock c, char* buf, size_t bufsize) {
    if (bufsize < 6) { // need at least "HH:MM" + '\0' => 6
        return NULL;
    }
    snprintf(buf, bufsize, "%02d:%02d", clock_hour(c), clock_minute(c));
    return buf;
}

// Equality
int clock_equals(Clock a, Clock b) {
    return a.minutes == b.minutes;
}

// Example helper that takes parameters and returns formatted result into provided buffer
char* clock_after_adding(int h, int m, long long delta, char* buf, size_t bufsize) {
    Clock c = clock_add(clock_at(h, m), delta);
    return clock_to_string(c, buf, bufsize);
}

int main(void) {
    char buf[16];

    Clock c1 = clock_add(clock_at(10, 0), 3);
    printf("Test1 %s\n", clock_to_string(c1, buf, sizeof(buf))); // 10:03

    Clock c2 = clock_add(clock_at(23, 30), 90);
    printf("Test2 %s\n", clock_to_string(c2, buf, sizeof(buf))); // 01:00

    Clock c3 = clock_subtract(clock_at(0, 10), 20);
    printf("Test3 %s\n", clock_to_string(c3, buf, sizeof(buf))); // 23:50

    Clock c4a = clock_at(15, 37);
    Clock c4b = clock_at(14, 97); // same as 15:37
    printf("Test4 equals %s\n", clock_equals(c4a, c4b) ? "true" : "false"); // true

    Clock c5a = clock_at(5, 32);
    Clock c5b = clock_add(c5a, 24 * 60); // wrap full day
    printf("Test5 equals %s %s\n", clock_equals(c5a, c5b) ? "true" : "false",
           clock_to_string(c5b, buf, sizeof(buf))); // true 05:32

    return 0;
}
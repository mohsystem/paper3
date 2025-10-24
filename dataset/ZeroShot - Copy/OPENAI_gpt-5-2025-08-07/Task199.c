#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int minutes; /* minutes since midnight, 0..1439 */
} Clock;

static const int MINUTES_PER_DAY = 24 * 60;

static int normalize_long_long(long long total) {
    long long mod = total % MINUTES_PER_DAY;
    if (mod < 0) mod += MINUTES_PER_DAY;
    return (int)mod;
}

Clock clock_create(int hour, int minute) {
    long long total = (long long)hour * 60LL + (long long)minute;
    Clock c;
    c.minutes = normalize_long_long(total);
    return c;
}

Clock clock_add(Clock c, int minutesToAdd) {
    long long total = (long long)c.minutes + (long long)minutesToAdd;
    Clock r;
    r.minutes = normalize_long_long(total);
    return r;
}

Clock clock_subtract(Clock c, int minutesToSubtract) {
    return clock_add(c, -minutesToSubtract);
}

int clock_hour(Clock c) {
    return c.minutes / 60;
}

int clock_minute(Clock c) {
    return c.minutes % 60;
}

int clock_equals(Clock a, Clock b) {
    return a.minutes == b.minutes;
}

/* Returns a newly allocated string "HH:MM". Caller must free. */
char* clock_to_string(Clock c) {
    char* s = (char*)malloc(6); /* "HH:MM" + '\0' */
    if (!s) {
        return NULL;
    }
    int h = clock_hour(c);
    int m = clock_minute(c);
    /* snprintf ensures null-termination and safe formatting */
    if (snprintf(s, 6, "%02d:%02d", h, m) < 0) {
        free(s);
        return NULL;
    }
    return s;
}

/* Example API functions that accept inputs and return outputs */
Clock create_clock(int hour, int minute) {
    return clock_create(hour, minute);
}
Clock add_minutes(Clock c, int minutes) {
    return clock_add(c, minutes);
}
Clock subtract_minutes(Clock c, int minutes) {
    return clock_subtract(c, minutes);
}
int clocks_equal(Clock a, Clock b) {
    return clock_equals(a, b);
}
char* clock_to_string_api(Clock c) {
    return clock_to_string(c);
}

int main(void) {
    /* 5 test cases */
    Clock t1 = add_minutes(create_clock(10, 0), 3);
    char* s1 = clock_to_string_api(t1);
    printf("Test1: %s\n", s1 ? s1 : "(alloc error)");
    free(s1);

    Clock t2 = add_minutes(create_clock(23, 59), 2);
    char* s2 = clock_to_string_api(t2);
    printf("Test2: %s\n", s2 ? s2 : "(alloc error)");
    free(s2);

    Clock t3 = subtract_minutes(create_clock(0, 3), 4);
    char* s3 = clock_to_string_api(t3);
    printf("Test3: %s\n", s3 ? s3 : "(alloc error)");
    free(s3);

    Clock t4a = create_clock(25, 0);
    Clock t4b = create_clock(1, 0);
    printf("Test4: equal=%s\n", clocks_equal(t4a, t4b) ? "true" : "false");

    Clock t5 = add_minutes(create_clock(2, 30), -1500); /* -25 hours */
    char* s5 = clock_to_string_api(t5);
    Clock expect = create_clock(1, 30);
    printf("Test5: %s equal 01:30? %s\n", s5 ? s5 : "(alloc error)", clocks_equal(t5, expect) ? "true" : "false");
    free(s5);

    return 0;
}
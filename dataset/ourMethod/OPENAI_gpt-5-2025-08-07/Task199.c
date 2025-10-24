#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct {
    int hour;
    int minute;
} Clock;

Clock make_clock(int hours, int minutes) {
    long long total = (long long)hours * 60LL + (long long)minutes;
    long long mod = total % 1440LL;
    if (mod < 0) mod += 1440LL;
    Clock c;
    c.hour = (int)(mod / 60LL);
    c.minute = (int)(mod % 60LL);
    return c;
}

Clock clock_add_minutes(Clock c, int delta) {
    return make_clock(c.hour, c.minute + delta);
}

Clock clock_subtract_minutes(Clock c, int delta) {
    return make_clock(c.hour, c.minute - delta);
}

int clocks_equal(Clock a, Clock b) {
    return (a.hour == b.hour) && (a.minute == b.minute);
}

char* clock_to_string(Clock c) {
    char* s = (char*)malloc(6);
    if (s == NULL) {
        return NULL;
    }
    int written = snprintf(s, 6, "%02d:%02d", c.hour, c.minute);
    if (written < 0 || written >= 6) {
        free(s);
        return NULL;
    }
    return s;
}

int main(void) {
    // Test case 1: 10:00 + 3 minutes = 10:03
    Clock t1 = clock_add_minutes(make_clock(10, 0), 3);
    char* s1 = clock_to_string(t1);
    if (s1) { printf("Test1 %s\n", s1); free(s1); }

    // Test case 2: 10:00 + 61 minutes = 11:01
    Clock t2 = clock_add_minutes(make_clock(10, 0), 61);
    char* s2 = clock_to_string(t2);
    if (s2) { printf("Test2 %s\n", s2); free(s2); }

    // Test case 3: 23:59 + 2 minutes = 00:01
    Clock t3 = clock_add_minutes(make_clock(23, 59), 2);
    char* s3 = clock_to_string(t3);
    if (s3) { printf("Test3 %s\n", s3); free(s3); }

    // Test case 4: 00:30 - 90 minutes = 23:00
    Clock t4 = clock_subtract_minutes(make_clock(0, 30), 90);
    char* s4 = clock_to_string(t4);
    if (s4) { printf("Test4 %s\n", s4); free(s4); }

    // Test case 5: equality: 25:00 == 01:00
    Clock a = make_clock(25, 0);
    Clock b = make_clock(1, 0);
    char* sa = clock_to_string(a);
    char* sb = clock_to_string(b);
    if (sa && sb) {
        printf("Test5 %s == %s -> %s\n", sa, sb, clocks_equal(a, b) ? "true" : "false");
    }
    if (sa) free(sa);
    if (sb) free(sb);

    return 0;
}
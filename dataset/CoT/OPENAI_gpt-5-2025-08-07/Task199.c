#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int minutes; // normalized to [0, 1439]
} Clock;

// Normalize minutes safely handling negatives
static int normalize_minutes(long long total) {
    long long mod = total % 1440LL;
    if (mod < 0) mod += 1440LL;
    return (int)mod;
}

// Create a clock from hour and minute
Clock clock_make(int hour, int minute) {
    long long total = (long long)hour * 60LL + (long long)minute;
    Clock c;
    c.minutes = normalize_minutes(total);
    return c;
}

// Add minutes, return new clock
Clock clock_add(Clock c, int minutes_to_add) {
    long long total = (long long)c.minutes + (long long)minutes_to_add;
    Clock r;
    r.minutes = normalize_minutes(total);
    return r;
}

// Subtract minutes, return new clock
Clock clock_sub(Clock c, int minutes_to_sub) {
    long long total = (long long)c.minutes - (long long)minutes_to_sub;
    Clock r;
    r.minutes = normalize_minutes(total);
    return r;
}

// Compare two clocks for equality
int clock_equals(Clock a, Clock b) {
    return a.minutes == b.minutes;
}

// Convert clock to "HH:MM" into buffer; requires out_size >= 6
void clock_to_string(Clock c, char* out, size_t out_size) {
    if (out == NULL || out_size < 6) {
        if (out && out_size > 0) out[0] = '\0';
        return;
    }
    int h = c.minutes / 60;
    int m = c.minutes % 60;
    // snprintf ensures no overflow and null-termination
    (void)snprintf(out, out_size, "%02d:%02d", h, m);
}

int main(void) {
    char buf[6];

    // Test 1: Basic creation and toString
    Clock c1 = clock_make(10, 37);
    clock_to_string(c1, buf, sizeof(buf));
    printf("Test1: %s\n", buf); // Expected 10:37

    // Test 2: Add minutes with hour rollover
    Clock c2 = clock_add(clock_make(10, 0), 3);
    clock_to_string(c2, buf, sizeof(buf));
    printf("Test2: %s\n", buf); // Expected 10:03

    // Test 3: Add minutes across midnight
    Clock c3 = clock_add(clock_make(23, 59), 2);
    clock_to_string(c3, buf, sizeof(buf));
    printf("Test3: %s\n", buf); // Expected 00:01

    // Test 4: Subtract minutes across midnight
    Clock c4 = clock_sub(clock_make(0, 5), 10);
    clock_to_string(c4, buf, sizeof(buf));
    printf("Test4: %s\n", buf); // Expected 23:55

    // Test 5: Equality of clocks representing same time
    Clock c5a = clock_make(24, 0);
    Clock c5b = clock_make(48, 0);
    clock_to_string(c5a, buf, sizeof(buf));
    char buf2[6];
    clock_to_string(c5b, buf2, sizeof(buf2));
    printf("Test5: %s == %s -> %s\n", buf, buf2, clock_equals(c5a, c5b) ? "true" : "false"); // Expected true

    return 0;
}
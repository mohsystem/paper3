#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define MINUTES_PER_HOUR 60
#define HOURS_PER_DAY 24
#define MINUTES_PER_DAY (HOURS_PER_DAY * MINUTES_PER_HOUR)

typedef struct {
    int totalMinutes;
} Clock;

Clock clock_create(int hour, int minute) {
    int effectiveTotalMinutes = hour * MINUTES_PER_HOUR + minute;
    // The formula (a % n + n) % n ensures the result is always positive
    // in the range [0, n-1], handling C's % operator behavior with negatives.
    int normalizedMinutes = (effectiveTotalMinutes % MINUTES_PER_DAY + MINUTES_PER_DAY) % MINUTES_PER_DAY;
    Clock c = {normalizedMinutes};
    return c;
}

Clock clock_add(Clock c, int minutes_to_add) {
    return clock_create(0, c.totalMinutes + minutes_to_add);
}

Clock clock_subtract(Clock c, int minutes_to_subtract) {
    return clock_create(0, c.totalMinutes - minutes_to_subtract);
}

bool clock_equals(Clock c1, Clock c2) {
    return c1.totalMinutes == c2.totalMinutes;
}

void clock_toString(Clock c, char* buffer, size_t size) {
    // "HH:MM\0" needs at least 6 characters.
    if (buffer == NULL || size < 6) {
        return;
    }
    int hour = c.totalMinutes / MINUTES_PER_HOUR;
    int minute = c.totalMinutes % MINUTES_PER_HOUR;
    snprintf(buffer, size, "%02d:%02d", hour, minute);
}

void run_tests() {
    char buffer[10];

    // Test Case 1: Create a clock and check its string representation
    Clock clock1 = clock_create(10, 30);
    clock_toString(clock1, buffer, sizeof(buffer));
    printf("Test 1: Create 10:30 -> %s\n", buffer);

    // Test Case 2: Add minutes
    Clock clock2 = clock_add(clock1, 70);
    clock_toString(clock2, buffer, sizeof(buffer));
    printf("Test 2: 10:30 + 70 min -> %s\n", buffer);

    // Test Case 3: Subtract minutes, wrapping backwards
    Clock clock3 = clock_subtract(clock_create(0, 15), 30);
    clock_toString(clock3, buffer, sizeof(buffer));
    printf("Test 3: 00:15 - 30 min -> %s\n", buffer);

    // Test Case 4: Equality check with different but equivalent inputs
    Clock clock4a = clock_create(14, 0);
    Clock clock4b = clock_create(13, 60);
    printf("Test 4: 14:00 equals 13:60 -> %s\n", clock_equals(clock4a, clock4b) ? "true" : "false");

    // Test Case 5: Add minutes, wrapping forwards past midnight
    Clock clock5 = clock_add(clock_create(23, 50), 20);
    clock_toString(clock5, buffer, sizeof(buffer));
    printf("Test 5: 23:50 + 20 min -> %s\n", buffer);
}

int main() {
    run_tests();
    return 0;
}
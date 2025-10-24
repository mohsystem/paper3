#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define MINUTES_PER_HOUR 60
#define HOURS_PER_DAY 24
#define MINUTES_PER_DAY (MINUTES_PER_HOUR * HOURS_PER_DAY)

typedef struct {
    int totalMinutes;
} Clock;

int normalize_minutes(int minutes) {
    // C's % operator can yield negative results for negative inputs
    return (minutes % MINUTES_PER_DAY + MINUTES_PER_DAY) % MINUTES_PER_DAY;
}

Clock createClock(int hour, int minute) {
    Clock c;
    c.totalMinutes = normalize_minutes(hour * MINUTES_PER_HOUR + minute);
    return c;
}

void add(Clock* clock, int minutes) {
    clock->totalMinutes = normalize_minutes(clock->totalMinutes + minutes);
}

void subtract(Clock* clock, int minutes) {
    add(clock, -minutes);
}

void toString(const Clock* clock, char* buffer, size_t buffer_size) {
    if (buffer_size < 6) return; // "HH:MM\0"
    int hour = clock->totalMinutes / MINUTES_PER_HOUR;
    int minute = clock->totalMinutes % MINUTES_PER_HOUR;
    snprintf(buffer, buffer_size, "%02d:%02d", hour, minute);
}

bool isEqual(const Clock* c1, const Clock* c2) {
    return c1->totalMinutes == c2->totalMinutes;
}

int main() {
    char buffer[10];

    // Test Case 1: Create a clock and test toString
    Clock clock1 = createClock(10, 30);
    toString(&clock1, buffer, sizeof(buffer));
    printf("Test Case 1: Create clock (10:30) -> %s\n", buffer);

    // Test Case 2: Add minutes without day rollover
    add(&clock1, 20);
    toString(&clock1, buffer, sizeof(buffer));
    printf("Test Case 2: Add 20 mins to 10:30 -> %s\n", buffer);

    // Test Case 3: Add minutes with day rollover
    Clock clock2 = createClock(23, 50);
    add(&clock2, 20);
    toString(&clock2, buffer, sizeof(buffer));
    printf("Test Case 3: Add 20 mins to 23:50 -> %s\n", buffer);

    // Test Case 4: Subtract minutes with day rollover
    Clock clock3 = createClock(0, 10);
    subtract(&clock3, 20);
    toString(&clock3, buffer, sizeof(buffer));
    printf("Test Case 4: Subtract 20 mins from 00:10 -> %s\n", buffer);

    // Test Case 5: Equality
    Clock clockA = createClock(14, 30);
    Clock clockB = createClock(14, 30);
    Clock clockC = createClock(10, 0);
    Clock clockD = createClock(38, 30); // equivalent to 14:30
    printf("Test Case 5: Equality Check\n");
    printf("  14:30 == 14:30 -> %s\n", isEqual(&clockA, &clockB) ? "true" : "false");
    printf("  14:30 == 10:00 -> %s\n", isEqual(&clockA, &clockC) ? "true" : "false");
    printf("  14:30 == normalized 38:30 -> %s\n", isEqual(&clockA, &clockD) ? "true" : "false");

    return 0;
}
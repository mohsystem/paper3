#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define MINUTES_IN_DAY (24 * 60)

typedef struct {
    int totalMinutes;
} Clock;

// Helper function to normalize minutes, handling negative modulo results
int normalize_minutes(int minutes) {
    return (minutes % MINUTES_IN_DAY + MINUTES_IN_DAY) % MINUTES_IN_DAY;
}

Clock createClock(int hours, int minutes) {
    Clock c;
    c.totalMinutes = normalize_minutes(hours * 60 + minutes);
    return c;
}

void addMinutes(Clock* clock, int minutes) {
    clock->totalMinutes = normalize_minutes(clock->totalMinutes + minutes);
}

void subtractMinutes(Clock* clock, int minutes) {
    addMinutes(clock, -minutes);
}

bool isEqual(Clock c1, Clock c2) {
    return c1.totalMinutes == c2.totalMinutes;
}

void clockToString(Clock clock, char* buffer, size_t size) {
    int hours = clock.totalMinutes / 60;
    int minutes = clock.totalMinutes % 60;
    snprintf(buffer, size, "%02d:%02d", hours, minutes);
}

int main() {
    printf("C Test Cases:\n");
    char buffer[6]; // HH:MM\0

    // Test 1: Creation and ToString
    Clock c1 = createClock(8, 0);
    clockToString(c1, buffer, sizeof(buffer));
    printf("Test 1: Create 08:00 -> %s\n", buffer);

    // Test 2: Equality
    Clock c2 = createClock(8, 0);
    Clock c3 = createClock(9, 0);
    printf("Test 2: 08:00 == 08:00 -> %s\n", isEqual(c1, c2) ? "true" : "false");
    printf("Test 2: 08:00 == 09:00 -> %s\n", isEqual(c1, c3) ? "true" : "false");

    // Test 3: Addition
    Clock c4 = createClock(10, 0);
    addMinutes(&c4, 63);
    clockToString(c4, buffer, sizeof(buffer));
    printf("Test 3: 10:00 + 63 mins -> %s\n", buffer);

    // Test 4: Subtraction with rollover
    Clock c5 = createClock(0, 30);
    subtractMinutes(&c5, 60);
    clockToString(c5, buffer, sizeof(buffer));
    printf("Test 4: 00:30 - 60 mins -> %s\n", buffer);

    // Test 5: Complex Addition with rollover
    Clock c6 = createClock(23, 45);
    addMinutes(&c6, 180);
    clockToString(c6, buffer, sizeof(buffer));
    printf("Test 5: 23:45 + 180 mins -> %s\n", buffer);

    return 0;
}
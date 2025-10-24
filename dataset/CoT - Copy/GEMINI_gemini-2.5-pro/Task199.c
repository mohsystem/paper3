#include <stdio.h>

#define MINUTES_IN_DAY (24 * 60)

typedef struct {
    int hours;
    int minutes;
} Clock;

Clock createClock(int hours, int minutes) {
    Clock c;
    int totalMinutes = hours * 60 + minutes;
    // The formula (a % n + n) % n handles negative results correctly.
    totalMinutes = (totalMinutes % MINUTES_IN_DAY + MINUTES_IN_DAY) % MINUTES_IN_DAY;
    c.hours = totalMinutes / 60;
    c.minutes = totalMinutes % 60;
    return c;
}

Clock addMinutes(Clock c, int minutesToAdd) {
    return createClock(c.hours, c.minutes + minutesToAdd);
}

Clock subtractMinutes(Clock c, int minutesToSubtract) {
    return createClock(c.hours, c.minutes - minutesToSubtract);
}

int areEqual(Clock c1, Clock c2) {
    return c1.hours == c2.hours && c1.minutes == c2.minutes;
}

void printClock(const char* prefix, Clock c) {
    printf("%s%02d:%02d\n", prefix, c.hours, c.minutes);
}

int main() {
    // Test Case 1: Simple creation and toString
    Clock clock1 = createClock(8, 30);
    printf("Test Case 1: Simple creation\n");
    printClock("Clock(8, 30) -> ", clock1);
    printf("--------------------\n");

    // Test Case 2: Adding minutes without hour rollover
    Clock clock2 = addMinutes(createClock(10, 0), 25);
    printf("Test Case 2: Add minutes (no rollover)\n");
    printClock("Clock(10, 0).add(25) -> ", clock2);
    printf("--------------------\n");

    // Test Case 3: Adding minutes with hour and day rollover
    Clock clock3 = addMinutes(createClock(23, 45), 30);
    printf("Test Case 3: Add minutes (with rollover)\n");
    printClock("Clock(23, 45).add(30) -> ", clock3);
    printf("--------------------\n");

    // Test Case 4: Subtracting minutes with hour and day rollover
    Clock clock4 = subtractMinutes(createClock(0, 10), 30);
    printf("Test Case 4: Subtract minutes (with rollover)\n");
    printClock("Clock(0, 10).subtract(30) -> ", clock4);
    printf("--------------------\n");

    // Test Case 5: Equality and normalization
    Clock clock5a = createClock(15, 30);
    Clock clock5b = createClock(14, 90);
    Clock clock5c = createClock(16, -30);
    Clock clock5d = createClock(39, 30);
    Clock clock5e = createClock(15, 31);
    printf("Test Case 5: Equality and normalization\n");
    printf("Clock(15, 30) equals Clock(14, 90)? %s\n", areEqual(clock5a, clock5b) ? "true" : "false");
    printf("Clock(15, 30) equals Clock(16, -30)? %s\n", areEqual(clock5a, clock5c) ? "true" : "false");
    printf("Clock(15, 30) equals Clock(39, 30)? %s\n", areEqual(clock5a, clock5d) ? "true" : "false");
    printf("Clock(15, 30) equals Clock(15, 31)? %s\n", areEqual(clock5a, clock5e) ? "true" : "false");
    printf("--------------------\n");

    return 0;
}
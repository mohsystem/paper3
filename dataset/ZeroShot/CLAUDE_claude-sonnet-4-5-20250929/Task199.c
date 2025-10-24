
#include <stdio.h>
#include <stdbool.h>

typedef struct {
    int hours;
    int minutes;
} Task199;

void normalize(Task199* clock, int totalMinutes) {
    totalMinutes = totalMinutes % (24 * 60);
    if (totalMinutes < 0) {
        totalMinutes += 24 * 60;
    }
    clock->hours = totalMinutes / 60;
    clock->minutes = totalMinutes % 60;
}

Task199 createClock(int hours, int minutes) {
    Task199 clock;
    int totalMinutes = hours * 60 + minutes;
    normalize(&clock, totalMinutes);
    return clock;
}

void addMinutes(Task199* clock, int minutesToAdd) {
    int totalMinutes = clock->hours * 60 + clock->minutes + minutesToAdd;
    normalize(clock, totalMinutes);
}

void subtractMinutes(Task199* clock, int minutesToSubtract) {
    addMinutes(clock, -minutesToSubtract);
}

bool equals(Task199 clock1, Task199 clock2) {
    return clock1.hours == clock2.hours && clock1.minutes == clock2.minutes;
}

void printClock(Task199 clock) {
    printf("%02d:%02d", clock.hours, clock.minutes);
}

int main() {
    // Test case 1: Basic clock creation
    Task199 clock1 = createClock(10, 30);
    printf("Test 1 - Clock creation: ");
    printClock(clock1);
    printf("\\n");
    
    // Test case 2: Add minutes
    Task199 clock2 = createClock(10, 30);
    addMinutes(&clock2, 90);
    printf("Test 2 - Add 90 minutes to 10:30: ");
    printClock(clock2);
    printf("\\n");
    
    // Test case 3: Subtract minutes
    Task199 clock3 = createClock(10, 30);
    subtractMinutes(&clock3, 45);
    printf("Test 3 - Subtract 45 minutes from 10:30: ");
    printClock(clock3);
    printf("\\n");
    
    // Test case 4: Clock equality
    Task199 clock4a = createClock(15, 45);
    Task199 clock4b = createClock(15, 45);
    printf("Test 4 - Clock equality (15:45 == 15:45): %s\\n", equals(clock4a, clock4b) ? "true" : "false");
    
    // Test case 5: Wrap around midnight
    Task199 clock5 = createClock(23, 30);
    addMinutes(&clock5, 45);
    printf("Test 5 - Add 45 minutes to 23:30 (wrap around): ");
    printClock(clock5);
    printf("\\n");
    
    return 0;
}

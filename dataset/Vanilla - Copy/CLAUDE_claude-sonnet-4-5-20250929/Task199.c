
#include <stdio.h>
#include <stdbool.h>

typedef struct {
    int hours;
    int minutes;
} Task199;

Task199 createClock(int h, int m) {
    Task199 clock;
    int totalMinutes = (h * 60 + m) % (24 * 60);
    if (totalMinutes < 0) {
        totalMinutes += 24 * 60;
    }
    clock.hours = totalMinutes / 60;
    clock.minutes = totalMinutes % 60;
    return clock;
}

Task199 addMinutes(Task199 clock, int minutesToAdd) {
    int totalMinutes = clock.hours * 60 + clock.minutes + minutesToAdd;
    return createClock(0, totalMinutes);
}

Task199 subtractMinutes(Task199 clock, int minutesToSubtract) {
    return addMinutes(clock, -minutesToSubtract);
}

bool equals(Task199 clock1, Task199 clock2) {
    return clock1.hours == clock2.hours && clock1.minutes == clock2.minutes;
}

void printClock(Task199 clock) {
    printf("%02d:%02d", clock.hours, clock.minutes);
}

int main() {
    // Test case 1: Create a clock and display it
    Task199 clock1 = createClock(10, 30);
    printf("Test 1 - Clock at 10:30: ");
    printClock(clock1);
    printf("\\n");
    
    // Test case 2: Add minutes
    Task199 clock2 = addMinutes(clock1, 45);
    printf("Test 2 - Add 45 minutes to 10:30: ");
    printClock(clock2);
    printf("\\n");
    
    // Test case 3: Subtract minutes
    Task199 clock3 = subtractMinutes(clock1, 90);
    printf("Test 3 - Subtract 90 minutes from 10:30: ");
    printClock(clock3);
    printf("\\n");
    
    // Test case 4: Wrap around midnight (add)
    Task199 clock4 = createClock(23, 45);
    Task199 clock5 = addMinutes(clock4, 30);
    printf("Test 4 - Add 30 minutes to 23:45: ");
    printClock(clock5);
    printf("\\n");
    
    // Test case 5: Equality check
    Task199 clock6 = createClock(11, 15);
    Task199 clock7 = createClock(11, 15);
    Task199 clock8 = createClock(11, 16);
    printf("Test 5 - 11:15 equals 11:15: %s\\n", equals(clock6, clock7) ? "true" : "false");
    printf("Test 5 - 11:15 equals 11:16: %s\\n", equals(clock6, clock8) ? "true" : "false");
    
    return 0;
}

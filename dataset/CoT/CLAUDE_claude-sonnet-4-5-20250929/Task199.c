
#include <stdio.h>
#include <stdbool.h>

typedef struct {
    int hours;
    int minutes;
} Task199;

Task199 createClock(int hours, int minutes) {
    Task199 clock;
    
    if (hours < 0 || minutes < 0) {
        fprintf(stderr, "Error: Hours and minutes cannot be negative\\n");
        clock.hours = 0;
        clock.minutes = 0;
        return clock;
    }
    
    int totalMinutes = hours * 60 + minutes;
    totalMinutes = totalMinutes % (24 * 60);
    if (totalMinutes < 0) {
        totalMinutes += 24 * 60;
    }
    
    clock.hours = totalMinutes / 60;
    clock.minutes = totalMinutes % 60;
    return clock;
}

Task199 addMinutes(Task199 clock, int minutesToAdd) {
    int totalMinutes = clock.hours * 60 + clock.minutes + minutesToAdd;
    totalMinutes = totalMinutes % (24 * 60);
    if (totalMinutes < 0) {
        totalMinutes += 24 * 60;
    }
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
    // Test case 1: Basic clock creation
    Task199 clock1 = createClock(10, 30);
    printf("Test 1 - Clock1: ");
    printClock(clock1);
    printf("\\n");
    
    // Test case 2: Adding minutes
    Task199 clock2 = addMinutes(clock1, 45);
    printf("Test 2 - Clock1 + 45 minutes: ");
    printClock(clock2);
    printf("\\n");
    
    // Test case 3: Subtracting minutes
    Task199 clock3 = subtractMinutes(clock2, 90);
    printf("Test 3 - Clock2 - 90 minutes: ");
    printClock(clock3);
    printf("\\n");
    
    // Test case 4: Equality check
    Task199 clock4 = createClock(10, 30);
    printf("Test 4 - Clock1 equals Clock4: %s\\n", equals(clock1, clock4) ? "true" : "false");
    
    // Test case 5: Wrap around midnight
    Task199 clock5 = createClock(23, 45);
    Task199 clock6 = addMinutes(clock5, 30);
    printf("Test 5 - 23:45 + 30 minutes: ");
    printClock(clock6);
    printf("\\n");
    
    return 0;
}

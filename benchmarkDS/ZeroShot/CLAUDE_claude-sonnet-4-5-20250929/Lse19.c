
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>

int addThousand(int value) {
    return value + 1000;
}

int main(int argc, char* argv[]) {
    // Test cases
    printf("Test 1: %d\\n", addThousand(500));      // Expected: 1500
    printf("Test 2: %d\\n", addThousand(0));        // Expected: 1000
    printf("Test 3: %d\\n", addThousand(-100));     // Expected: 900
    printf("Test 4: %d\\n", addThousand(2500));     // Expected: 3500
    printf("Test 5: %d\\n", addThousand(-1000));    // Expected: 0
    
    // Command line argument handling
    if (argc > 1) {
        char* endptr;
        errno = 0;
        long value = strtol(argv[1], &endptr, 10);
        
        if (errno == 0 && *endptr == '\\0' && value >= INT_MIN && value <= INT_MAX) {
            printf("Command line result: %d\\n", addThousand((int)value));
        } else {
            fprintf(stderr, "Invalid input. Please provide a valid integer.\\n");
        }
    }
    
    return 0;
}

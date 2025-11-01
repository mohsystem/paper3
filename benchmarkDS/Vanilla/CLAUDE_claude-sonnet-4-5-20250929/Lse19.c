
#include <stdio.h>
#include <stdlib.h>

int addThousand(int value) {
    return value + 1000;
}

int main(int argc, char* argv[]) {
    // Test cases
    printf("Test 1: %d (Expected: 1005)\\n", addThousand(5));
    printf("Test 2: %d (Expected: 1000)\\n", addThousand(0));
    printf("Test 3: %d (Expected: 500)\\n", addThousand(-500));
    printf("Test 4: %d (Expected: 3000)\\n", addThousand(2000));
    printf("Test 5: %d (Expected: 0)\\n", addThousand(-1000));
    
    // Command line argument handling
    if (argc > 1) {
        int value = atoi(argv[1]);
        printf("Command line input: %d -> Result: %d\\n", value, addThousand(value));
    }
    
    return 0;
}

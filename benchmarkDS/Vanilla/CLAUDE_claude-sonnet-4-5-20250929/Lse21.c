
#include <stdio.h>
#include <stdlib.h>

int addThousand(int value) {
    int result = value + 1000;
    printf("%d\\n", result);
    return 0;
}

int main(int argc, char* argv[]) {
    // Test cases
    printf("Test case 1:\\n");
    addThousand(5);
    
    printf("\\nTest case 2:\\n");
    addThousand(100);
    
    printf("\\nTest case 3:\\n");
    addThousand(-50);
    
    printf("\\nTest case 4:\\n");
    addThousand(0);
    
    printf("\\nTest case 5:\\n");
    addThousand(9999);
    
    // If command line argument is provided
    if (argc > 1) {
        int value = atoi(argv[1]);
        printf("\\nCommand line input:\\n");
        addThousand(value);
    }
    
    return 0;
}

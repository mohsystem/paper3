#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Note: The caller is responsible for freeing the memory of the returned string.
char* whoLikesIt(const char* names[], int count) {
    char* result = NULL;
    int size = 0;

    switch (count) {
        case 0:
            size = strlen("no one likes this") + 1;
            result = (char*)malloc(size);
            snprintf(result, size, "no one likes this");
            break;
        case 1:
            size = strlen(names[0]) + strlen(" likes this") + 1;
            result = (char*)malloc(size);
            snprintf(result, size, "%s likes this", names[0]);
            break;
        case 2:
            size = strlen(names[0]) + strlen(" and ") + strlen(names[1]) + strlen(" like this") + 1;
            result = (char*)malloc(size);
            snprintf(result, size, "%s and %s like this", names[0], names[1]);
            break;
        case 3:
            size = strlen(names[0]) + strlen(", ") + strlen(names[1]) + strlen(" and ") + strlen(names[2]) + strlen(" like this") + 1;
            result = (char*)malloc(size);
            snprintf(result, size, "%s, %s and %s like this", names[0], names[1], names[2]);
            break;
        default:
            // Calculate size needed for the number
            int num_chars = snprintf(NULL, 0, "%d", count - 2);
            size = strlen(names[0]) + strlen(", ") + strlen(names[1]) + strlen(" and ") + num_chars + strlen(" others like this") + 1;
            result = (char*)malloc(size);
            snprintf(result, size, "%s, %s and %d others like this", names[0], names[1], count - 2);
            break;
    }
    return result;
}

void run_test(const char* names[], int count) {
    char* output = whoLikesIt(names, count);
    if (output) {
        printf("%s\n", output);
        free(output);
    }
}

int main() {
    // Test Case 1
    const char* test1[] = {};
    run_test(test1, 0);

    // Test Case 2
    const char* test2[] = {"Peter"};
    run_test(test2, 1);

    // Test Case 3
    const char* test3[] = {"Jacob", "Alex"};
    run_test(test3, 2);

    // Test Case 4
    const char* test4[] = {"Max", "John", "Mark"};
    run_test(test4, 3);

    // Test Case 5
    const char* test5[] = {"Alex", "Jacob", "Mark", "Max"};
    run_test(test5, 4);

    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* likes(int count, const char* names[]) {
    if (count <= 0) {
        const char* s = "no one likes this";
        char* out = (char*)malloc(strlen(s) + 1);
        if (out) strcpy(out, s);
        return out;
    } else if (count == 1) {
        int needed = snprintf(NULL, 0, "%s likes this", names[0]);
        char* out = (char*)malloc((size_t)needed + 1);
        if (out) snprintf(out, (size_t)needed + 1, "%s likes this", names[0]);
        return out;
    } else if (count == 2) {
        int needed = snprintf(NULL, 0, "%s and %s like this", names[0], names[1]);
        char* out = (char*)malloc((size_t)needed + 1);
        if (out) snprintf(out, (size_t)needed + 1, "%s and %s like this", names[0], names[1]);
        return out;
    } else if (count == 3) {
        int needed = snprintf(NULL, 0, "%s, %s and %s like this", names[0], names[1], names[2]);
        char* out = (char*)malloc((size_t)needed + 1);
        if (out) snprintf(out, (size_t)needed + 1, "%s, %s and %s like this", names[0], names[1], names[2]);
        return out;
    } else {
        int others = count - 2;
        int needed = snprintf(NULL, 0, "%s, %s and %d others like this", names[0], names[1], others);
        char* out = (char*)malloc((size_t)needed + 1);
        if (out) snprintf(out, (size_t)needed + 1, "%s, %s and %d others like this", names[0], names[1], others);
        return out;
    }
}

int main() {
    // Test case 1: []
    char* r1 = likes(0, NULL);
    printf("%s\n", r1);
    free(r1);

    // Test case 2: ["Peter"]
    const char* t2[] = {"Peter"};
    char* r2 = likes(1, t2);
    printf("%s\n", r2);
    free(r2);

    // Test case 3: ["Jacob", "Alex"]
    const char* t3[] = {"Jacob", "Alex"};
    char* r3 = likes(2, t3);
    printf("%s\n", r3);
    free(r3);

    // Test case 4: ["Max", "John", "Mark"]
    const char* t4[] = {"Max", "John", "Mark"};
    char* r4 = likes(3, t4);
    printf("%s\n", r4);
    free(r4);

    // Test case 5: ["Alex", "Jacob", "Mark", "Max", "Lily"]
    const char* t5[] = {"Alex", "Jacob", "Mark", "Max", "Lily"};
    char* r5 = likes(5, t5);
    printf("%s\n", r5);
    free(r5);

    return 0;
}
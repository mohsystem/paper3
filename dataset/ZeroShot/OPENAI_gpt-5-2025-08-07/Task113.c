#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

char* concatenate(const char* parts[], size_t count) {
    if (parts == NULL) {
        char* empty = (char*)malloc(1);
        if (empty != NULL) {
            empty[0] = '\0';
        }
        return empty;
    }

    size_t total = 0;
    for (size_t i = 0; i < count; ++i) {
        size_t len = (parts[i] == NULL) ? 0 : strlen(parts[i]);
        if (len > SIZE_MAX - total - 1) {
            // Overflow would occur
            return NULL;
        }
        total += len;
    }

    char* result = (char*)malloc(total + 1);
    if (result == NULL) {
        return NULL;
    }

    size_t pos = 0;
    for (size_t i = 0; i < count; ++i) {
        if (parts[i] != NULL) {
            size_t len = strlen(parts[i]);
            memcpy(result + pos, parts[i], len);
            pos += len;
        }
    }
    result[pos] = '\0';
    return result;
}

int main(void) {
    // Test case 1
    const char* t1[] = {"Hello", " ", "World", "!"};
    char* r1 = concatenate(t1, sizeof(t1) / sizeof(t1[0]));
    if (r1) { printf("Test 1: %s\n", r1); free(r1); } else { puts("Test 1: Error"); }

    // Test case 2 (UTF-8)
    const char* t2[] = {"na\xC3\xAFve", " ", "caf\xC3\xA9", " ", "\xF0\x9F\x98\x8A"};
    char* r2 = concatenate(t2, sizeof(t2) / sizeof(t2[0]));
    if (r2) { printf("Test 2: %s\n", r2); free(r2); } else { puts("Test 2: Error"); }

    // Test case 3 (NULL entries)
    const char* t3[] = {"A", NULL, "B", NULL, "C"};
    char* r3 = concatenate(t3, sizeof(t3) / sizeof(t3[0]));
    if (r3) { printf("Test 3: %s\n", r3); free(r3); } else { puts("Test 3: Error"); }

    // Test case 4 (empty array)
    const char** t4 = NULL;
    char* r4 = concatenate(t4, 0);
    if (r4) { printf("Test 4: %s\n", r4); free(r4); } else { puts("Test 4: Error"); }

    // Test case 5 (empty strings)
    const char* t5[] = {"", "", "x", "", "y", "", "z"};
    char* r5 = concatenate(t5, sizeof(t5) / sizeof(t5[0]));
    if (r5) { printf("Test 5: %s\n", r5); free(r5); } else { puts("Test 5: Error"); }

    return 0;
}
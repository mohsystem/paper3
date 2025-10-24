#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char* safe(const char* s) {
    return s ? s : "";
}

static size_t slen(const char* s) {
    return s ? strlen(s) : 0;
}

static size_t digits_count(size_t n) {
    size_t c = 1;
    while (n >= 10) {
        n /= 10;
        c++;
    }
    return c;
}

char* likes(const char* names[], size_t count) {
    if (count == 0 || names == NULL) {
        const char* msg = "no one likes this";
        char* out = (char*)malloc(strlen(msg) + 1);
        if (!out) return NULL;
        strcpy(out, msg);
        return out;
    } else if (count == 1) {
        size_t l0 = slen(names[0]);
        const char* suffix = " likes this";
        size_t size_needed = l0 + strlen(suffix);
        char* out = (char*)malloc(size_needed + 1);
        if (!out) return NULL;
        snprintf(out, size_needed + 1, "%s%s", safe(names[0]), suffix);
        return out;
    } else if (count == 2) {
        size_t l0 = slen(names[0]);
        size_t l1 = slen(names[1]);
        const char* mid = " and ";
        const char* suffix = " like this";
        size_t size_needed = l0 + strlen(mid) + l1 + strlen(suffix);
        char* out = (char*)malloc(size_needed + 1);
        if (!out) return NULL;
        snprintf(out, size_needed + 1, "%s and %s like this", safe(names[0]), safe(names[1]));
        return out;
    } else if (count == 3) {
        size_t l0 = slen(names[0]);
        size_t l1 = slen(names[1]);
        size_t l2 = slen(names[2]);
        size_t size_needed = l0 + 2 + l1 + 5 + l2 + 10; // ", " + " and " + " like this"
        char* out = (char*)malloc(size_needed + 1);
        if (!out) return NULL;
        snprintf(out, size_needed + 1, "%s, %s and %s like this", safe(names[0]), safe(names[1]), safe(names[2]));
        return out;
    } else {
        size_t l0 = slen(names[0]);
        size_t l1 = slen(names[1]);
        size_t others = count - 2;
        size_t d = digits_count(others);
        size_t size_needed = l0 + 2 + l1 + 5 + d + 17; // ", " + " and " + " others like this"
        char* out = (char*)malloc(size_needed + 1);
        if (!out) return NULL;
        snprintf(out, size_needed + 1, "%s, %s and %zu others like this", safe(names[0]), safe(names[1]), others);
        return out;
    }
}

int main(void) {
    // Test 1: []
    char* r0 = likes(NULL, 0);
    printf("Test 1: %s\n", r0 ? r0 : "(allocation failed)");
    free(r0);

    // Test 2: ["Peter"]
    const char* t1[] = {"Peter"};
    char* r1 = likes(t1, 1);
    printf("Test 2: %s\n", r1 ? r1 : "(allocation failed)");
    free(r1);

    // Test 3: ["Jacob", "Alex"]
    const char* t2[] = {"Jacob", "Alex"};
    char* r2 = likes(t2, 2);
    printf("Test 3: %s\n", r2 ? r2 : "(allocation failed)");
    free(r2);

    // Test 4: ["Max", "John", "Mark"]
    const char* t3[] = {"Max", "John", "Mark"};
    char* r3 = likes(t3, 3);
    printf("Test 4: %s\n", r3 ? r3 : "(allocation failed)");
    free(r3);

    // Test 5: ["Alex", "Jacob", "Mark", "Max"]
    const char* t4[] = {"Alex", "Jacob", "Mark", "Max"};
    char* r4 = likes(t4, 4);
    printf("Test 5: %s\n", r4 ? r4 : "(allocation failed)");
    free(r4);

    return 0;
}
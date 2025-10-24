#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char* str_dup(const char* s) {
    if (s == NULL) {
        s = "null";
    }
    size_t len = strlen(s);
    char* out = (char*)malloc(len + 1);
    if (!out) return NULL;
    memcpy(out, s, len + 1);
    return out;
}

char* likes(char* names[], int n) {
    if (n <= 0 || names == NULL) {
        return str_dup("no one likes this");
    }
    const char* a = (names[0] != NULL) ? names[0] : "null";
    if (n == 1) {
        int needed = snprintf(NULL, 0, "%s likes this", a);
        if (needed < 0) return NULL;
        char* buf = (char*)malloc((size_t)needed + 1);
        if (!buf) return NULL;
        snprintf(buf, (size_t)needed + 1, "%s likes this", a);
        return buf;
    }
    const char* b = (n > 1 && names[1] != NULL) ? names[1] : "null";
    if (n == 2) {
        int needed = snprintf(NULL, 0, "%s and %s like this", a, b);
        if (needed < 0) return NULL;
        char* buf = (char*)malloc((size_t)needed + 1);
        if (!buf) return NULL;
        snprintf(buf, (size_t)needed + 1, "%s and %s like this", a, b);
        return buf;
    }
    const char* c = (n > 2 && names[2] != NULL) ? names[2] : "null";
    if (n == 3) {
        int needed = snprintf(NULL, 0, "%s, %s and %s like this", a, b, c);
        if (needed < 0) return NULL;
        char* buf = (char*)malloc((size_t)needed + 1);
        if (!buf) return NULL;
        snprintf(buf, (size_t)needed + 1, "%s, %s and %s like this", a, b, c);
        return buf;
    }
    int others = n - 2;
    int needed = snprintf(NULL, 0, "%s, %s and %d others like this", a, b, others);
    if (needed < 0) return NULL;
    char* buf = (char*)malloc((size_t)needed + 1);
    if (!buf) return NULL;
    snprintf(buf, (size_t)needed + 1, "%s, %s and %d others like this", a, b, others);
    return buf;
}

int main(void) {
    char* t0[] = { };
    char* t1[] = { "Peter" };
    char* t2[] = { "Jacob", "Alex" };
    char* t3[] = { "Max", "John", "Mark" };
    char* t4[] = { "Alex", "Jacob", "Mark", "Max" };

    char* r0 = likes(t0, 0);
    char* r1 = likes(t1, 1);
    char* r2 = likes(t2, 2);
    char* r3 = likes(t3, 3);
    char* r4 = likes(t4, 4);

    printf("[] --> %s\n", r0 ? r0 : "(alloc failed)");
    printf("[Peter] --> %s\n", r1 ? r1 : "(alloc failed)");
    printf("[Jacob, Alex] --> %s\n", r2 ? r2 : "(alloc failed)");
    printf("[Max, John, Mark] --> %s\n", r3 ? r3 : "(alloc failed)");
    printf("[Alex, Jacob, Mark, Max] --> %s\n", r4 ? r4 : "(alloc failed)");

    free(r0);
    free(r1);
    free(r2);
    free(r3);
    free(r4);
    return 0;
}
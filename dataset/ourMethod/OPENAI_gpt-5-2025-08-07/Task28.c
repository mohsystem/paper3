#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char* sanitize_name(const char* s) {
    return (s != NULL) ? s : "unknown";
}

char* format_likes(size_t count, const char* names[]) {
    // Handle names == NULL when count == 0; if count > 0 and names == NULL, treat as unknowns
    const char* n0 = (count >= 1) ? sanitize_name(names ? names[0] : NULL) : NULL;
    const char* n1 = (count >= 2) ? sanitize_name(names ? names[1] : NULL) : NULL;
    const char* n2 = (count >= 3) ? sanitize_name(names ? names[2] : NULL) : NULL;

    int needed = 0;
    if (count == 0) {
        needed = snprintf(NULL, 0, "no one likes this");
    } else if (count == 1) {
        needed = snprintf(NULL, 0, "%s likes this", n0);
    } else if (count == 2) {
        needed = snprintf(NULL, 0, "%s and %s like this", n0, n1);
    } else if (count == 3) {
        needed = snprintf(NULL, 0, "%s, %s and %s like this", n0, n1, n2);
    } else {
        size_t others = count - 2U;
        needed = snprintf(NULL, 0, "%s, %s and %zu others like this", n0, n1, others);
    }

    if (needed < 0) {
        return NULL; // snprintf error
    }

    char* out = (char*)malloc((size_t)needed + 1U);
    if (!out) {
        return NULL;
    }

    if (count == 0) {
        if (snprintf(out, (size_t)needed + 1U, "no one likes this") < 0) {
            free(out);
            return NULL;
        }
    } else if (count == 1) {
        if (snprintf(out, (size_t)needed + 1U, "%s likes this", n0) < 0) {
            free(out);
            return NULL;
        }
    } else if (count == 2) {
        if (snprintf(out, (size_t)needed + 1U, "%s and %s like this", n0, n1) < 0) {
            free(out);
            return NULL;
        }
    } else if (count == 3) {
        if (snprintf(out, (size_t)needed + 1U, "%s, %s and %s like this", n0, n1, n2) < 0) {
            free(out);
            return NULL;
        }
    } else {
        size_t others = count - 2U;
        if (snprintf(out, (size_t)needed + 1U, "%s, %s and %zu others like this", n0, n1, others) < 0) {
            free(out);
            return NULL;
        }
    }

    return out;
}

int main(void) {
    // 5 test cases
    const char** t0 = NULL; // []
    const char* t1[] = {"Peter"};
    const char* t2[] = {"Jacob", "Alex"};
    const char* t3[] = {"Max", "John", "Mark"};
    const char* t4[] = {"Alex", "Jacob", "Mark", "Max"};

    char* r0 = format_likes(0U, t0);
    char* r1 = format_likes(1U, t1);
    char* r2 = format_likes(2U, t2);
    char* r3 = format_likes(3U, t3);
    char* r4 = format_likes(4U, t4);

    if (r0) { printf("%s\n", r0); free(r0); }
    if (r1) { printf("%s\n", r1); free(r1); }
    if (r2) { printf("%s\n", r2); free(r2); }
    if (r3) { printf("%s\n", r3); free(r3); }
    if (r4) { printf("%s\n", r4); free(r4); }

    return 0;
}
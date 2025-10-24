#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

typedef struct {
    char* original;
    char* lastLower;
    size_t lastLen;
    size_t index;
} Item;

static char* safe_strdup(const char* s) {
    if (s == NULL) return NULL;
    size_t len = strlen(s);
    char* p = (char*)malloc(len + 1);
    if (!p) return NULL;
    memcpy(p, s, len + 1);
    return p;
}

static int extract_last_lower(const char* s, char** out_lower, size_t* out_len) {
    if (!s || !out_lower || !out_len) return 0;

    const char* start = s;
    while (*start && isspace((unsigned char)*start)) start++;

    const char* end = s + strlen(s);
    while (end > start && isspace((unsigned char)*(end - 1))) end--;

    if (start >= end) return 0;

    const char* p = end - 1;
    while (p >= start && !isspace((unsigned char)*p)) p--;
    if (p < start) {
        // No space found -> invalid (needs first and last name)
        return 0;
    }
    const char* last_start = p + 1;
    size_t len = (size_t)(end - last_start);
    if (len == 0) return 0;

    char* lower = (char*)malloc(len + 1);
    if (!lower) return 0;
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)last_start[i];
        lower[i] = (char)tolower(c);
    }
    lower[len] = '\0';

    *out_lower = lower;
    *out_len = len;
    return 1;
}

static int cmp_items(const void* a, const void* b) {
    const Item* ia = (const Item*)a;
    const Item* ib = (const Item*)b;
    if (ia->lastLen < ib->lastLen) return -1;
    if (ia->lastLen > ib->lastLen) return 1;
    int sc = strcmp(ia->lastLower, ib->lastLower);
    if (sc != 0) return sc;
    if (ia->index < ib->index) return -1;
    if (ia->index > ib->index) return 1;
    return 0;
}

// Returns a newly allocated array of string duplicates sorted by last name length,
// then alphabetically by last name (case-insensitive). On error, returns NULL.
// Caller must free each string in the returned array and then free the array itself.
// out_count will be set to count on success.
char** last_name_len_sort(const char* names[], size_t count, size_t* out_count) {
    if (!names) return NULL;
    Item* items = (Item*)calloc(count, sizeof(Item));
    if (!items) return NULL;

    for (size_t i = 0; i < count; i++) {
        if (!names[i]) {
            // cleanup
            for (size_t j = 0; j < i; j++) {
                free(items[j].original);
                free(items[j].lastLower);
            }
            free(items);
            return NULL;
        }
        items[i].original = safe_strdup(names[i]);
        if (!items[i].original) {
            for (size_t j = 0; j < i; j++) {
                free(items[j].original);
                free(items[j].lastLower);
            }
            free(items);
            return NULL;
        }
        if (!extract_last_lower(names[i], &items[i].lastLower, &items[i].lastLen)) {
            for (size_t j = 0; j <= i; j++) {
                free(items[j].original);
                if (items[j].lastLower) free(items[j].lastLower);
            }
            free(items);
            return NULL;
        }
        items[i].index = i;
    }

    qsort(items, count, sizeof(Item), cmp_items);

    char** out = (char**)malloc(count * sizeof(char*));
    if (!out) {
        for (size_t i = 0; i < count; i++) {
            free(items[i].original);
            free(items[i].lastLower);
        }
        free(items);
        return NULL;
    }
    for (size_t i = 0; i < count; i++) {
        out[i] = items[i].original; // transfer ownership
        free(items[i].lastLower);
    }
    free(items);
    if (out_count) *out_count = count;
    return out;
}

static void print_list(char** arr, size_t n) {
    printf("[");
    for (size_t i = 0; i < n; i++) {
        printf("%s", arr[i]);
        if (i + 1 < n) printf(", ");
    }
    printf("]\n");
}

static void free_list(char** arr, size_t n) {
    if (!arr) return;
    for (size_t i = 0; i < n; i++) {
        free(arr[i]);
    }
    free(arr);
}

int main(void) {
    const char* test1[] = {
        "Jennifer Figueroa",
        "Heather Mcgee",
        "Amanda Schwartz",
        "Nicole Yoder",
        "Melissa Hoffman"
    };
    const char* test2[] = {
        "Jim Bo",
        "Ana Li",
        "Zoe Wu",
        "Tom Ng",
        "Foo Bar"
    };
    const char* test3[] = {
        "Mary Jane Watson",
        "Jean Claude Van Damme",
        "Peter Parker",
        "Tony Stark",
        "Bruce Wayne"
    };
    const char* test4[] = {
        "  Alice   Smith  ",
        "bob a",
        "CARL B",
        "dave  aa",
        "Eve   Z"
    };
    const char* test5[] = {
        "Alice Hope",
        "Bob Hope",
        "Carl Hope",
        "Ann Hope",
        "Zed Hope"
    };

    const struct { const char** arr; size_t n; } tests[] = {
        { test1, sizeof(test1)/sizeof(test1[0]) },
        { test2, sizeof(test2)/sizeof(test2[0]) },
        { test3, sizeof(test3)/sizeof(test3[0]) },
        { test4, sizeof(test4)/sizeof(test4[0]) },
        { test5, sizeof(test5)/sizeof(test5[0]) }
    };

    for (size_t i = 0; i < sizeof(tests)/sizeof(tests[0]); i++) {
        size_t out_n = 0;
        char** res = last_name_len_sort(tests[i].arr, tests[i].n, &out_n);
        printf("Test %zu: ", i + 1);
        if (res) {
            print_list(res, out_n);
            free_list(res, out_n);
        } else {
            printf("Error: invalid input or memory allocation failure\n");
        }
    }

    return 0;
}
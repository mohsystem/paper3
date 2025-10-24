#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct NameEntry {
    char *original;
    char *last;
    size_t last_len;
} NameEntry;

static void extract_last_name(const char *name, char *out, size_t out_size) {
    if (!out || out_size == 0) return;
    out[0] = '\0';
    if (!name) return;
    size_t len = strlen(name);
    size_t end = len;
    while (end > 0 && isspace((unsigned char)name[end - 1])) end--;
    if (end == 0) return;
    size_t i = end;
    while (i > 0 && !isspace((unsigned char)name[i - 1])) i--;
    size_t start = i;
    size_t l = end - start;
    if (l >= out_size) l = out_size - 1;
    if (l > 0) {
        memcpy(out, name + start, l);
    }
    out[l] = '\0';
}

static int ascii_tolower(int c) {
    if (c >= 'A' && c <= 'Z') return c - 'A' + 'a';
    return c;
}

static int ascii_casecmp(const char *a, const char *b) {
    unsigned char ca, cb;
    while (*a && *b) {
        ca = (unsigned char)ascii_tolower((unsigned char)*a);
        cb = (unsigned char)ascii_tolower((unsigned char)*b);
        if (ca < cb) return -1;
        if (ca > cb) return 1;
        a++; b++;
    }
    if (*a == '\0' && *b == '\0') return 0;
    return (*a == '\0') ? -1 : 1;
}

static int cmp_entries(const void *pa, const void *pb) {
    const NameEntry *a = (const NameEntry *)pa;
    const NameEntry *b = (const NameEntry *)pb;
    if (a->last_len < b->last_len) return -1;
    if (a->last_len > b->last_len) return 1;
    int lc = ascii_casecmp(a->last, b->last);
    if (lc != 0) return lc;
    return ascii_casecmp(a->original ? a->original : "", b->original ? b->original : "");
}

char **lastNameLensort(const char **names, int n, int *outSize) {
    if (outSize) *outSize = 0;
    if (!names || n <= 0) {
        return NULL;
    }
    NameEntry *entries = (NameEntry *)calloc((size_t)n, sizeof(NameEntry));
    if (!entries) {
        return NULL;
    }
    int ok = 1;
    for (int i = 0; i < n; ++i) {
        const char *src = names[i] ? names[i] : "";
        entries[i].original = (char *)malloc(strlen(src) + 1);
        if (!entries[i].original) { ok = 0; break; }
        strcpy(entries[i].original, src);

        size_t bufsize = strlen(src) + 1;
        char *buf = (char *)malloc(bufsize);
        if (!buf) { ok = 0; break; }
        extract_last_name(src, buf, bufsize);
        entries[i].last = buf;
        entries[i].last_len = strlen(buf);
    }
    if (!ok) {
        for (int i = 0; i < n; ++i) {
            if (entries[i].original) free(entries[i].original);
            if (entries[i].last) free(entries[i].last);
        }
        free(entries);
        return NULL;
    }

    qsort(entries, (size_t)n, sizeof(NameEntry), cmp_entries);

    char **result = (char **)malloc(sizeof(char *) * (size_t)n);
    if (!result) {
        for (int i = 0; i < n; ++i) {
            free(entries[i].original);
            free(entries[i].last);
        }
        free(entries);
        return NULL;
    }
    for (int i = 0; i < n; ++i) {
        result[i] = entries[i].original; // transfer ownership
        free(entries[i].last);
    }
    free(entries);
    if (outSize) *outSize = n;
    return result;
}

static void print_array(char **arr, int n) {
    printf("[");
    for (int i = 0; i < n; ++i) {
        if (i) printf(", ");
        printf("\"%s\"", arr[i] ? arr[i] : "");
    }
    printf("]\n");
}

static void free_array(char **arr, int n) {
    if (!arr) return;
    for (int i = 0; i < n; ++i) {
        free(arr[i]);
    }
    free(arr);
}

int main(void) {
    // Test case 1 (given example)
    const char *t1[] = {
        "Jennifer Figueroa",
        "Heather Mcgee",
        "Amanda Schwartz",
        "Nicole Yoder",
        "Melissa Hoffman"
    };
    int n1 = 0;
    char **r1 = lastNameLensort(t1, 5, &n1);
    print_array(r1, n1);
    free_array(r1, n1);

    // Test case 2 (all same last-name length -> alphabetical by last name)
    const char *t2[] = {"Anna Zed", "Bob Kay", "Cara May", "Dan Ray"};
    int n2 = 0;
    char **r2 = lastNameLensort(t2, 4, &n2);
    print_array(r2, n2);
    free_array(r2, n2);

    // Test case 3 (extra spaces and punctuation)
    const char *t3[] = {"  John   Doe  ", "Alice   Smith", "Bob  O'Niel"};
    int n3 = 0;
    char **r3 = lastNameLensort(t3, 3, &n3);
    print_array(r3, n3);
    free_array(r3, n3);

    // Test case 4 (same last name -> fall back to full name alphabetical)
    const char *t4[] = {"Cindy Lee", "Anna Lee", "Brian Lee"};
    int n4 = 0;
    char **r4 = lastNameLensort(t4, 3, &n4);
    print_array(r4, n4);
    free_array(r4, n4);

    // Test case 5 (edge cases: empty, single token, spaces)
    const char *t5[] = {"", "Single", "Mary Ann", "  "};
    int n5 = 0;
    char **r5 = lastNameLensort(t5, 4, &n5);
    print_array(r5, n5);
    free_array(r5, n5);

    return 0;
}
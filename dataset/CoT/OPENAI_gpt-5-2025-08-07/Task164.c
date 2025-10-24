#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* Safe string duplication */
static char* safe_strdup(const char* s) {
    if (!s) {
        char* e = (char*)malloc(1);
        if (e) e[0] = '\0';
        return e;
    }
    size_t len = strlen(s);
    char* d = (char*)malloc(len + 1);
    if (!d) exit(EXIT_FAILURE);
    memcpy(d, s, len + 1);
    return d;
}

/* Trim spaces and find start pointer of last name (last token) */
static const char* last_name_start(const char* s) {
    if (!s) return "";
    const char* end = s + strlen(s);
    // Move left past trailing spaces
    while (end > s && isspace((unsigned char)*(end - 1))) end--;
    if (end == s) return s; // empty or all spaces
    const char* p = end - 1;
    // Find space preceding last token
    while (p >= s && !isspace((unsigned char)*p)) p--;
    return p + 1;
}

/* Length of last name */
static size_t last_name_len(const char* s) {
    const char* start = last_name_start(s);
    size_t len = 0;
    while (start[len] && !isspace((unsigned char)start[len])) len++;
    return len;
}

/* Case-insensitive strcmp for substrings until space or '\0' */
static int ci_strcmp_last(const char* a, const char* b) {
    while (*a && *b && !isspace((unsigned char)*a) && !isspace((unsigned char)*b)) {
        unsigned char ca = (unsigned char)tolower((unsigned char)*a);
        unsigned char cb = (unsigned char)tolower((unsigned char)*b);
        if (ca != cb) return (ca < cb) ? -1 : 1;
        a++; b++;
    }
    int enda = (*a == '\0' || isspace((unsigned char)*a));
    int endb = (*b == '\0' || isspace((unsigned char)*b));
    if (enda && endb) return 0;
    return enda ? -1 : 1;
}

/* Deterministic tie-breaker (case-sensitive) for last name substrings */
static int cs_strcmp_last(const char* a, const char* b) {
    while (*a && *b && !isspace((unsigned char)*a) && !isspace((unsigned char)*b)) {
        unsigned char ca = (unsigned char)*a;
        unsigned char cb = (unsigned char)*b;
        if (ca != cb) return (ca < cb) ? -1 : 1;
        a++; b++;
    }
    int enda = (*a == '\0' || isspace((unsigned char)*a));
    int endb = (*b == '\0' || isspace((unsigned char)*b));
    if (enda && endb) return 0;
    return enda ? -1 : 1;
}

/* Comparator for qsort */
static int cmp_names(const void* A, const void* B) {
    const char* a = *(const char* const*)A;
    const char* b = *(const char* const*)B;
    size_t la_len = last_name_len(a);
    size_t lb_len = last_name_len(b);
    if (la_len != lb_len) return (la_len < lb_len) ? -1 : 1;
    const char* la = last_name_start(a);
    const char* lb = last_name_start(b);
    int ci = ci_strcmp_last(la, lb);
    if (ci != 0) return ci;
    return cs_strcmp_last(la, lb);
}

/* Returns a newly allocated array of duplicated strings sorted accordingly */
char** lastNameLensort(const char** names, int n) {
    if (n <= 0 || names == NULL) {
        return NULL;
    }
    char** out = (char**)malloc((size_t)n * sizeof(char*));
    if (!out) exit(EXIT_FAILURE);
    for (int i = 0; i < n; ++i) {
        out[i] = safe_strdup(names[i]);
    }
    qsort(out, (size_t)n, sizeof(char*), cmp_names);
    return out;
}

/* Utility to print array */
static void print_arr(char** arr, int n) {
    printf("[");
    for (int i = 0; i < n; ++i) {
        if (i) printf(", ");
        printf("\"%s\"", arr[i]);
    }
    printf("]\n");
}

static void free_arr(char** arr, int n) {
    if (!arr) return;
    for (int i = 0; i < n; ++i) {
        free(arr[i]);
    }
    free(arr);
}

int main(void) {
    // Test case 1
    const char* t1[] = {
        "Jennifer Figueroa",
        "Heather Mcgee",
        "Amanda Schwartz",
        "Nicole Yoder",
        "Melissa Hoffman"
    };
    int n1 = (int)(sizeof(t1)/sizeof(t1[0]));
    char** r1 = lastNameLensort(t1, n1);
    print_arr(r1, n1);
    free_arr(r1, n1);

    // Test case 2
    const char* t2[] = {
        "John Smith",
        "Amy Brown",
        "Zoey Clark",
        "Evan White",
        "Liam Jones"
    };
    int n2 = (int)(sizeof(t2)/sizeof(t2[0]));
    char** r2 = lastNameLensort(t2, n2);
    print_arr(r2, n2);
    free_arr(r2, n2);

    // Test case 3
    const char* t3[] = {
        "Alice   van Helsing",
        "Bob de la Cruz",
        "  Carla   Delaney ",
        "Derek O'Neill",
        "Eve   Stone  "
    };
    int n3 = (int)(sizeof(t3)/sizeof(t3[0]));
    char** r3 = lastNameLensort(t3, n3);
    print_arr(r3, n3);
    free_arr(r3, n3);

    // Test case 4
    const char* t4[] = { "Solo Person" };
    int n4 = (int)(sizeof(t4)/sizeof(t4[0]));
    char** r4 = lastNameLensort(t4, n4);
    print_arr(r4, n4);
    free_arr(r4, n4);

    // Test case 5
    const char* t5[] = {
        "Ann Lee",
        "Ben Lee",
        "Cara Zee",
        "Dan Lee",
        "Eli Zee"
    };
    int n5 = (int)(sizeof(t5)/sizeof(t5[0]));
    char** r5 = lastNameLensort(t5, n5);
    print_arr(r5, n5);
    free_arr(r5, n5);

    return 0;
}
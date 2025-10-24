#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static char* my_strdup(const char* s) {
    size_t n = strlen(s);
    char* d = (char*)malloc(n + 1);
    if (!d) return NULL;
    memcpy(d, s, n + 1);
    return d;
}

static void get_last_name_info(const char* s, const char** last_start, int* last_len) {
    const char* end = s + strlen(s);
    while (end > s && isspace((unsigned char)*(end - 1))) end--;
    const char* p = end;
    while (p > s && !isspace((unsigned char)*(p - 1))) p--;
    *last_start = p;
    *last_len = (int)(end - p);
}

static int ci_ncmp(const char* a, const char* b, int n) {
    for (int i = 0; i < n; ++i) {
        unsigned char ca = (unsigned char)a[i];
        unsigned char cb = (unsigned char)b[i];
        int da = tolower(ca);
        int db = tolower(cb);
        if (da != db) return da - db;
    }
    return 0;
}

static int ci_strcmp_full(const char* a, const char* b) {
    while (*a && *b) {
        int da = tolower((unsigned char)*a);
        int db = tolower((unsigned char)*b);
        if (da != db) return da - db;
        ++a; ++b;
    }
    if (*a) return 1;
    if (*b) return -1;
    return 0;
}

static int cmp_names(const void* pa, const void* pb) {
    const char* a = *(const char* const*)pa;
    const char* b = *(const char* const*)pb;
    const char* la; int la_len;
    const char* lb; int lb_len;
    get_last_name_info(a, &la, &la_len);
    get_last_name_info(b, &lb, &lb_len);
    if (la_len != lb_len) return la_len - lb_len;
    int cmp = ci_ncmp(la, lb, la_len); // same lengths
    if (cmp != 0) return cmp;
    return ci_strcmp_full(a, b);
}

char** lastNameLensort(const char** names, int n) {
    char** arr = (char**)malloc(sizeof(char*) * n);
    if (!arr) return NULL;
    for (int i = 0; i < n; ++i) {
        arr[i] = my_strdup(names[i]);
    }
    qsort(arr, n, sizeof(char*), cmp_names);
    return arr;
}

static void print_array(char** arr, int n) {
    printf("[");
    for (int i = 0; i < n; ++i) {
        printf("\"%s\"", arr[i]);
        if (i + 1 < n) printf(", ");
    }
    printf("]\n");
}

int main(void) {
    const char* t1[] = {
        "Jennifer Figueroa",
        "Heather Mcgee",
        "Amanda Schwartz",
        "Nicole Yoder",
        "Melissa Hoffman"
    };
    const char* t2[] = {
        "John Doe",
        "Alice Roe",
        "Bob Poe",
        "Zed Kay"
    };
    const char* t3[] = {
        "Anna Smith",
        "Brian Smith",
        "Aaron Smith",
        "Zoe Adams"
    };
    const char* t4[] = {
        "Al A",
        "Bea Bee",
        "Ce Ce",
        "Dee Eee",
        "Eff Eff"
    };
    const char* t5[] = {
        "Mary Ann Van Dyke",
        "Jean-Claude Van Damme",
        "Jo Van",
        "Li Wei",
        "X AE A-12 Musk"
    };

    int n1 = sizeof(t1)/sizeof(t1[0]);
    int n2 = sizeof(t2)/sizeof(t2[0]);
    int n3 = sizeof(t3)/sizeof(t3[0]);
    int n4 = sizeof(t4)/sizeof(t4[0]);
    int n5 = sizeof(t5)/sizeof(t5[0]);

    char** r1 = lastNameLensort(t1, n1);
    char** r2 = lastNameLensort(t2, n2);
    char** r3 = lastNameLensort(t3, n3);
    char** r4 = lastNameLensort(t4, n4);
    char** r5 = lastNameLensort(t5, n5);

    print_array(r1, n1);
    print_array(r2, n2);
    print_array(r3, n3);
    print_array(r4, n4);
    print_array(r5, n5);

    // Free allocated arrays
    char** rs[] = { r1, r2, r3, r4, r5 };
    int ns[] = { n1, n2, n3, n4, n5 };
    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < ns[i]; ++j) free(rs[i][j]);
        free(rs[i]);
    }

    return 0;
}
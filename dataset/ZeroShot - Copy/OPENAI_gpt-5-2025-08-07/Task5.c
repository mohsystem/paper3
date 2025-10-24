#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct {
    char* data;
    size_t len;
    size_t cap;
} Str;

static int str_ensure(Str* s, size_t need) {
    if (s->cap >= need) return 1;
    size_t newcap = s->cap ? s->cap : 16;
    while (newcap < need) {
        if (newcap > (size_t)(~0) / 2) { newcap = need; break; }
        newcap *= 2;
    }
    char* nd = (char*)realloc(s->data, newcap);
    if (!nd) return 0;
    s->data = nd;
    s->cap = newcap;
    return 1;
}

static void str_init(Str* s) {
    s->data = NULL;
    s->len = 0;
    s->cap = 0;
}

static int str_set(Str* s, const char* lit) {
    size_t n = strlen(lit);
    if (!str_ensure(s, n + 1)) return 0;
    memcpy(s->data, lit, n + 1);
    s->len = n;
    return 1;
}

static void str_free(Str* s) {
    if (s->data) free(s->data);
    s->data = NULL;
    s->len = 0;
    s->cap = 0;
}

static char* safe_strdup(const char* src) {
    size_t n = strlen(src);
    char* p = (char*)malloc(n + 1);
    if (!p) return NULL;
    memcpy(p, src, n + 1);
    return p;
}

static int is_valid_digits_c(const char* s) {
    if (!s || !*s) return 0;
    for (const char* p = s; *p; ++p) {
        if (!isdigit((unsigned char)*p)) return 0;
    }
    return 1;
}

static char* normalize_input(const char* s) {
    const char* p = s;
    while (*p == '0') ++p;
    if (*p == '\0') {
        return safe_strdup("0");
    }
    return safe_strdup(p);
}

static int multiply_by_digit(Str* s, int d) {
    if (d < 0 || d > 9) return 0;
    if (d == 0) return str_set(s, "0");
    if (d == 1) return 1;
    if (s->len == 1 && s->data[0] == '0') return 1;
    size_t n = s->len;
    char* tmp = (char*)malloc(n + 2);
    if (!tmp) return 0;
    size_t ti = n + 1;
    tmp[ti] = '\0';
    int carry = 0;
    for (size_t k = 0; k < n; ++k) {
        int idx = (int)(n - 1 - k);
        int digit = s->data[idx] - '0';
        int prod = digit * d + carry;
        tmp[--ti] = (char)('0' + (prod % 10));
        carry = prod / 10;
    }
    if (carry > 0) {
        tmp[--ti] = (char)('0' + carry);
    }
    size_t newlen = (n + 1) - ti;
    if (!str_ensure(s, newlen + 1)) { free(tmp); return 0; }
    memcpy(s->data, &tmp[ti], newlen);
    s->data[newlen] = '\0';
    s->len = newlen;
    free(tmp);
    return 1;
}

static int product_of_digits_c(const char* digits, Str* out) {
    if (!str_set(out, "1")) return 0;
    for (const char* p = digits; *p; ++p) {
        if (!isdigit((unsigned char)*p)) return 0;
        int d = *p - '0';
        if (d == 0) {
            return str_set(out, "0");
        }
        if (!multiply_by_digit(out, d)) return 0;
    }
    return 1;
}

int persistence(const char* num) {
    if (!is_valid_digits_c(num)) {
        return -1; // indicate invalid input
    }
    char* s = normalize_input(num);
    if (!s) return -1;
    if (strlen(s) == 1) {
        free(s);
        return 0;
    }
    int count = 0;
    while (strlen(s) > 1) {
        Str prod;
        str_init(&prod);
        if (!product_of_digits_c(s, &prod)) {
            str_free(&prod);
            free(s);
            return -1;
        }
        free(s);
        s = safe_strdup(prod.data);
        str_free(&prod);
        if (!s) return -1;
        count++;
    }
    free(s);
    return count;
}

int main(void) {
    const char* tests[5] = {"39", "999", "4", "25", "77"};
    for (int i = 0; i < 5; ++i) {
        int res = persistence(tests[i]);
        printf("persistence(%s) = %d\n", tests[i], res);
    }
    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#define MAX_LEN 100000u

static size_t safe_strlen_limit(const char* s, size_t limit) {
    if (s == NULL) return SIZE_MAX;
    size_t len = 0u;
    while (s[len] != '\0') {
        if (len >= limit) return SIZE_MAX; // Exceeds limit
        len++;
    }
    return len;
}

static int add_will_overflow_size_t(size_t a, size_t b) {
    return b > SIZE_MAX - a;
}

char* copy_string(const char* s) {
    size_t len = safe_strlen_limit(s, MAX_LEN);
    if (len == SIZE_MAX) return NULL;
    if (add_will_overflow_size_t(len, 1u)) return NULL;
    char* out = (char*)malloc(len + 1u);
    if (!out) return NULL;
    if (len > 0u) memcpy(out, s, len);
    out[len] = '\0';
    return out;
}

char* concat_strings(const char* a, const char* b) {
    size_t la = safe_strlen_limit(a, MAX_LEN);
    size_t lb = safe_strlen_limit(b, MAX_LEN);
    if (la == SIZE_MAX || lb == SIZE_MAX) return NULL;
    if (add_will_overflow_size_t(la, lb)) return NULL;
    size_t total = la + lb;
    if (total > MAX_LEN) return NULL;
    if (add_will_overflow_size_t(total, 1u)) return NULL;

    char* out = (char*)malloc(total + 1u);
    if (!out) return NULL;
    if (la > 0u) memcpy(out, a, la);
    if (lb > 0u) memcpy(out + la, b, lb);
    out[total] = '\0';
    return out;
}

char* reverse_string(const char* s) {
    size_t len = safe_strlen_limit(s, MAX_LEN);
    if (len == SIZE_MAX) return NULL;
    if (add_will_overflow_size_t(len, 1u)) return NULL;
    char* out = (char*)malloc(len + 1u);
    if (!out) return NULL;
    for (size_t i = 0u; i < len; ++i) {
        out[i] = s[len - 1u - i];
    }
    out[len] = '\0';
    return out;
}

char* to_upper_case(const char* s) {
    size_t len = safe_strlen_limit(s, MAX_LEN);
    if (len == SIZE_MAX) return NULL;
    if (add_will_overflow_size_t(len, 1u)) return NULL;
    char* out = (char*)malloc(len + 1u);
    if (!out) return NULL;
    for (size_t i = 0u; i < len; ++i) {
        unsigned char ch = (unsigned char)s[i];
        out[i] = (char)toupper(ch);
    }
    out[len] = '\0';
    return out;
}

char* safe_substring(const char* s, size_t start, size_t length) {
    size_t len = safe_strlen_limit(s, MAX_LEN);
    if (len == SIZE_MAX) return NULL;
    if (start > len) return NULL;
    if (length > len - start) return NULL;
    if (length > MAX_LEN) return NULL;
    if (add_will_overflow_size_t(length, 1u)) return NULL;

    char* out = (char*)malloc(length + 1u);
    if (!out) return NULL;
    if (length > 0u) memcpy(out, s + start, length);
    out[length] = '\0';
    return out;
}

int main(void) {
    // Test 1: Copy
    char* t1 = copy_string("Hello");
    if (t1) { printf("Copy: %s\n", t1); free(t1); } else { printf("Error: copy_string failed\n"); }

    // Test 2: Concat
    char* t2 = concat_strings("Hello", " World");
    if (t2) { printf("Concat: %s\n", t2); free(t2); } else { printf("Error: concat_strings failed\n"); }

    // Test 3: Reverse
    char* t3 = reverse_string("RaceCar");
    if (t3) { printf("Reverse: %s\n", t3); free(t3); } else { printf("Error: reverse_string failed\n"); }

    // Test 4: To Upper
    char* t4 = to_upper_case("security123");
    if (t4) { printf("Upper: %s\n", t4); free(t4); } else { printf("Error: to_upper_case failed\n"); }

    // Test 5: Safe Substring
    char* t5 = safe_substring("abcdef", 2u, 3u);
    if (t5) { printf("Substring: %s\n", t5); free(t5); } else { printf("Error: safe_substring failed\n"); }

    return 0;
}
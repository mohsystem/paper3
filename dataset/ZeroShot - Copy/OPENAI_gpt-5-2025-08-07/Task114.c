#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

static char* safe_strdup(const char* s) {
    if (s == NULL) return NULL;
    size_t len = strlen(s);
    if (len > SIZE_MAX - 1) return NULL;
    char* out = (char*)malloc(len + 1);
    if (!out) return NULL;
    memcpy(out, s, len + 1);
    return out;
}

char* copy_string(const char* s) {
    return safe_strdup(s);
}

char* concat_strings(const char* a, const char* b) {
    if (a == NULL || b == NULL) return NULL;
    size_t la = strlen(a), lb = strlen(b);
    if (la > SIZE_MAX - lb - 1) return NULL; // +1 for null terminator
    char* out = (char*)malloc(la + lb + 1);
    if (!out) return NULL;
    memcpy(out, a, la);
    memcpy(out + la, b, lb);
    out[la + lb] = '\0';
    return out;
}

char* reverse_string(const char* s) {
    if (s == NULL) return NULL;
    size_t len = strlen(s);
    if (len > SIZE_MAX - 1) return NULL;
    char* out = (char*)malloc(len + 1);
    if (!out) return NULL;
    for (size_t i = 0; i < len; ++i) {
        out[i] = s[len - 1 - i];
    }
    out[len] = '\0';
    return out;
}

char* to_uppercase(const char* s) {
    if (s == NULL) return NULL;
    size_t len = strlen(s);
    if (len > SIZE_MAX - 1) return NULL;
    char* out = (char*)malloc(len + 1);
    if (!out) return NULL;
    for (size_t i = 0; i < len; ++i) {
        unsigned char ch = (unsigned char)s[i];
        out[i] = (char)toupper(ch);
    }
    out[len] = '\0';
    return out;
}

int is_palindrome(const char* s) {
    if (s == NULL) return 0;
    size_t i = 0;
    size_t j = strlen(s);
    if (j == 0) return 1;
    j--;
    while (i < j) {
        unsigned char ci = (unsigned char)s[i];
        unsigned char cj = (unsigned char)s[j];
        if (!isalnum(ci)) { i++; continue; }
        if (!isalnum(cj)) { j--; continue; }
        if (tolower(ci) != tolower(cj)) return 0;
        i++; j--;
    }
    return 1;
}

char* replace_substring(const char* s, const char* target, const char* replacement) {
    if (s == NULL || target == NULL || replacement == NULL) return NULL;
    if (target[0] == '\0') {
        // Avoid ambiguous behavior; return a copy
        return safe_strdup(s);
    }

    size_t len_s = strlen(s);
    size_t len_t = strlen(target);
    size_t len_r = strlen(replacement);

    // Count occurrences
    size_t count = 0;
    const char* p = s;
    while ((p = strstr(p, target)) != NULL) {
        count++;
        p += len_t;
    }

    if (count == 0) {
        return safe_strdup(s);
    }

    size_t new_len;
    if (len_r >= len_t) {
        size_t diff = len_r - len_t;
        if (diff != 0) {
            if (count > 0 && diff > (SIZE_MAX - len_s) / count) {
                return NULL; // overflow
            }
        }
        new_len = len_s + count * diff;
    } else {
        size_t diff = len_t - len_r;
        // len_s - count * diff cannot underflow because each replacement reduces size by diff
        new_len = len_s - count * diff;
    }

    if (new_len > SIZE_MAX - 1) return NULL;
    char* out = (char*)malloc(new_len + 1);
    if (!out) return NULL;

    const char* src = s;
    char* dst = out;
    while ((p = strstr(src, target)) != NULL) {
        size_t prefix = (size_t)(p - src);
        if (prefix > 0) {
            memcpy(dst, src, prefix);
            dst += prefix;
        }
        if (len_r > 0) {
            memcpy(dst, replacement, len_r);
            dst += len_r;
        }
        src = p + len_t;
    }
    // Copy the remainder
    size_t remainder = strlen(src);
    if (remainder > 0) {
        memcpy(dst, src, remainder);
        dst += remainder;
    }
    *dst = '\0';
    return out;
}

int main(void) {
    // 5 test cases
    char* t1 = copy_string("Hello, World!");
    if (t1) {
        printf("Copy: %s\n", t1);
        free(t1);
    } else {
        printf("Copy: (null)\n");
    }

    char* t2 = concat_strings("Hello ", "World");
    if (t2) {
        printf("Concat: %s\n", t2);
        free(t2);
    } else {
        printf("Concat: (null)\n");
    }

    char* t3 = reverse_string("abcdefg");
    if (t3) {
        printf("Reverse: %s\n", t3);
        free(t3);
    } else {
        printf("Reverse: (null)\n");
    }

    char* t4 = to_uppercase("SecuRe123!");
    if (t4) {
        printf("Upper: %s\n", t4);
        free(t4);
    } else {
        printf("Upper: (null)\n");
    }

    char* t5 = replace_substring("the cat sat on the mat", "at", "oodle");
    if (t5) {
        printf("Replace: %s\n", t5);
        free(t5);
    } else {
        printf("Replace: (null)\n");
    }

    return 0;
}
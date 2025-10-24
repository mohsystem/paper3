/* Chain-of-Through Secure Implementation
   1) Problem: safe string operations in C.
   2) Security: avoid unsafe APIs, validate, check overflows, manage memory.
   3) Secure coding: bounds checks, helper safe arithmetic.
   4) Code review: no gets/strcpy/strcat, check malloc results, free all allocations.
   5) Output: robust functions + 5 tests.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

/* Helpers for safe size arithmetic */
static int safe_add_size(size_t a, size_t b, size_t* out) {
    if (SIZE_MAX - a < b) return 0;
    *out = a + b;
    return 1;
}
static int safe_mul_size(size_t a, size_t b, size_t* out) {
    if (a != 0 && b > SIZE_MAX / a) return 0;
    *out = a * b;
    return 1;
}

/* Treat NULL as empty string */
static const char* nz(const char* s) {
    return s ? s : "";
}

/* Copy string safely (returns newly allocated string) */
char* copy_string(const char* s) {
    const char* src = nz(s);
    size_t n = strlen(src);
    size_t alloc;
    if (!safe_add_size(n, 1, &alloc)) return NULL;
    char* out = (char*)malloc(alloc);
    if (!out) return NULL;
    memcpy(out, src, n + 1);
    return out;
}

/* Concatenate two strings (returns newly allocated string) */
char* concat_strings(const char* a, const char* b) {
    const char* sa = nz(a);
    const char* sb = nz(b);
    size_t na = strlen(sa), nb = strlen(sb);
    size_t sz, alloc;
    if (!safe_add_size(na, nb, &sz)) return NULL;
    if (!safe_add_size(sz, 1, &alloc)) return NULL;
    char* out = (char*)malloc(alloc);
    if (!out) return NULL;
    memcpy(out, sa, na);
    memcpy(out + na, sb, nb);
    out[na + nb] = '\0';
    return out;
}

/* Reverse a string (returns newly allocated string) */
char* reverse_string(const char* s) {
    const char* src = nz(s);
    size_t n = strlen(src);
    size_t alloc;
    if (!safe_add_size(n, 1, &alloc)) return NULL;
    char* out = (char*)malloc(alloc);
    if (!out) return NULL;
    for (size_t i = 0; i < n; ++i) out[i] = src[n - 1 - i];
    out[n] = '\0';
    return out;
}

/* Count non-overlapping occurrences */
size_t count_occurrences(const char* s, const char* sub) {
    const char* src = nz(s);
    const char* needle = nz(sub);
    if (*needle == '\0') return 0;
    size_t count = 0;
    const char* p = src;
    size_t L = strlen(needle);
    while ((p = strstr(p, needle)) != NULL) {
        count++;
        p += L;
    }
    return count;
}

/* Replace all occurrences of target with replacement (returns newly allocated string) */
char* replace_substring(const char* s, const char* target, const char* replacement) {
    const char* src = nz(s);
    const char* t = target ? target : "";
    const char* r = replacement ? replacement : "";
    if (*t == '\0') {
        return copy_string(src);
    }
    size_t src_len = strlen(src);
    size_t t_len = strlen(t);
    size_t r_len = strlen(r);

    /* First pass: count occurrences */
    size_t occ = 0;
    const char* p = src;
    while ((p = strstr(p, t)) != NULL) {
        occ++;
        p += t_len;
    }

    /* Compute new length with overflow checks: src_len + occ*(r_len - t_len) + 1 */
    size_t delta;
    if (r_len >= t_len) {
        size_t diff = r_len - t_len;
        if (!safe_mul_size(occ, diff, &delta)) return NULL;
        if (!safe_add_size(src_len, delta, &delta)) return NULL;
    } else {
        size_t diff = t_len - r_len;
        /* subtract occ*diff, safe because result <= src_len */
        size_t sub;
        if (!safe_mul_size(occ, diff, &sub)) return NULL;
        if (src_len < sub) return NULL; /* should not happen */
        delta = src_len - sub;
    }
    size_t alloc;
    if (!safe_add_size(delta, 1, &alloc)) return NULL;

    char* out = (char*)malloc(alloc);
    if (!out) return NULL;

    /* Second pass: build output */
    const char* cur = src;
    char* w = out;
    const char* hit;
    while ((hit = strstr(cur, t)) != NULL) {
        size_t chunk = (size_t)(hit - cur);
        memcpy(w, cur, chunk);
        w += chunk;
        memcpy(w, r, r_len);
        w += r_len;
        cur = hit + t_len;
    }
    size_t tail = strlen(cur);
    memcpy(w, cur, tail);
    w += tail;
    *w = '\0';
    return out;
}

/* Safe substring with bounds checks (returns newly allocated string) */
char* safe_substring(const char* s, size_t start, size_t length) {
    const char* src = nz(s);
    size_t n = strlen(src);
    if (start >= n || length == 0) {
        char* out = (char*)malloc(1);
        if (out) out[0] = '\0';
        return out;
    }
    size_t end;
    if (!safe_add_size(start, length, &end)) end = SIZE_MAX;
    if (end > n) end = n;
    if (end < start) {
        char* out = (char*)malloc(1);
        if (out) out[0] = '\0';
        return out;
    }
    size_t slice_len = end - start;
    size_t alloc;
    if (!safe_add_size(slice_len, 1, &alloc)) return NULL;
    char* out = (char*)malloc(alloc);
    if (!out) return NULL;
    memcpy(out, src + start, slice_len);
    out[slice_len] = '\0';
    return out;
}

/* Repeat string with overflow checks (returns newly allocated string) */
char* repeat_string(const char* s, size_t times) {
    const char* src = nz(s);
    size_t n = strlen(src);
    if (times == 0 || n == 0) {
        char* out = (char*)malloc(1);
        if (out) out[0] = '\0';
        return out;
    }
    size_t total;
    if (!safe_mul_size(n, times, &total)) return NULL;
    size_t alloc;
    if (!safe_add_size(total, 1, &alloc)) return NULL;
    char* out = (char*)malloc(alloc);
    if (!out) return NULL;
    char* p = out;
    for (size_t i = 0; i < times; ++i) {
        memcpy(p, src, n);
        p += n;
    }
    *p = '\0';
    return out;
}

/* To upper (returns newly allocated string) */
char* to_upper(const char* s) {
    const char* src = nz(s);
    size_t n = strlen(src);
    size_t alloc;
    if (!safe_add_size(n, 1, &alloc)) return NULL;
    char* out = (char*)malloc(alloc);
    if (!out) return NULL;
    for (size_t i = 0; i < n; ++i) {
        out[i] = (char)toupper((unsigned char)src[i]);
    }
    out[n] = '\0';
    return out;
}

/* To lower (returns newly allocated string) */
char* to_lower(const char* s) {
    const char* src = nz(s);
    size_t n = strlen(src);
    size_t alloc;
    if (!safe_add_size(n, 1, &alloc)) return NULL;
    char* out = (char*)malloc(alloc);
    if (!out) return NULL;
    for (size_t i = 0; i < n; ++i) {
        out[i] = (char)tolower((unsigned char)src[i]);
    }
    out[n] = '\0';
    return out;
}

/* Trim leading/trailing whitespace (returns newly allocated string) */
char* trim_safe(const char* s) {
    const char* src = nz(s);
    size_t n = strlen(src);
    size_t i = 0, j = n;
    while (i < j && isspace((unsigned char)src[i])) ++i;
    while (j > i && isspace((unsigned char)src[j - 1])) --j;
    size_t len = j - i;
    size_t alloc;
    if (!safe_add_size(len, 1, &alloc)) return NULL;
    char* out = (char*)malloc(alloc);
    if (!out) return NULL;
    memcpy(out, src + i, len);
    out[len] = '\0';
    return out;
}

/* Print helper that tolerates NULL strings by printing "(null)" */
static void print_str(const char* label, const char* s) {
    printf("%s%s\n", label, s ? s : "(null)");
}

int main(void) {
    const char* A[5] = {"Hello", "", "  Spaces  ", "abcabcabc", NULL};
    const char* B[5] = {"World", "Test", "", "xyz", NULL};
    const char* T[5] = {"l", "Test", " ", "abc", NULL};
    const char* R[5] = {"L", "Exam", "", "A", NULL};

    for (int i = 0; i < 5; ++i) {
        const char* a = A[i];
        const char* b = B[i];
        const char* tgt = T[i];
        const char* rep = R[i];

        printf("=== Test %d ===\n", i + 1);

        char* c1 = copy_string(a);
        print_str("copy_string(a): ", c1);

        char* c2 = concat_strings(a, b);
        print_str("concat_strings(a,b): ", c2);

        char* c3 = reverse_string(a);
        print_str("reverse_string(a): ", c3);

        char* c4 = replace_substring(a, tgt, rep);
        print_str("replace_substring(a,tgt,rep): ", c4);

        char* c5 = safe_substring(a ? a : "", 1u, 3u);
        print_str("safe_substring(a,1,3): ", c5);

        size_t cnt = count_occurrences(a, tgt);
        printf("count_occurrences(a,tgt): %zu\n", cnt);

        char* c6 = repeat_string("ab", 3u);
        print_str("repeat_string(\"ab\",3): ", c6);

        char* c7 = to_upper(a);
        print_str("to_upper(a): ", c7);

        char* c8 = to_lower(a);
        print_str("to_lower(a): ", c8);

        char* c9 = trim_safe(a);
        print_str("trim_safe(a): ", c9);

        /* Free all allocated memory */
        free(c1);
        free(c2);
        free(c3);
        free(c4);
        free(c5);
        free(c6);
        free(c7);
        free(c8);
        free(c9);
    }
    return 0;
}
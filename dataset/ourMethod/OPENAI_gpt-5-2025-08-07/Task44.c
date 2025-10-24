#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>

#define INPUT_MAX 2048

typedef struct {
    size_t length;
    size_t letters;
    size_t digits;
    int is_palindrome; /* 1 yes, 0 no */
} StringMetrics;

/* Safe line reader: reads up to max_len-1 chars, strips trailing newline.
   Returns 1 on success, 0 on EOF/error, -1 if line too long (rest flushed). */
static int safe_read_line(char *buf, size_t max_len, FILE *in) {
    if (buf == NULL || max_len == 0 || in == NULL) return 0;
    if (fgets(buf, (int)max_len, in) == NULL) {
        return 0;
    }
    size_t len = strnlen(buf, max_len);
    if (len == max_len - 1 && buf[len - 1] != '\n') {
        /* flush remainder */
        int ch;
        int too_long = 1;
        do {
            ch = fgetc(in);
        } while (ch != '\n' && ch != EOF);
        if (len > 0 && buf[len - 1] == '\n') buf[len - 1] = '\0';
        return -1 * too_long;
    }
    if (len > 0 && buf[len - 1] == '\n') buf[len - 1] = '\0';
    return 1;
}

/* Allow only printable ASCII 32..126. Reject control chars.
   Returns 1 on success, 0 on invalid input or exceeds max_len/out_size. */
static int sanitize_string(const char *in, char *out, size_t out_size, size_t max_len) {
    if (!in || !out || out_size == 0) return 0;
    size_t in_len = strnlen(in, max_len + 1);
    if (in_len == max_len + 1) {
        return 0; /* too long */
    }
    if (in_len + 1 > out_size) return 0;
    for (size_t i = 0; i < in_len; ++i) {
        unsigned char uc = (unsigned char)in[i];
        if (uc < 32 || uc > 126) {
            return 0;
        }
        out[i] = (char)uc;
    }
    out[in_len] = '\0';
    return 1;
}

/* Analyze string: compute metrics and produce reversed into reversed_out.
   Returns 1 on success, 0 on error (e.g., reversed_out too small). */
static int analyze_string(const char *s, char *reversed_out, size_t rev_size, StringMetrics *metrics) {
    if (!s || !reversed_out || rev_size == 0 || !metrics) return 0;
    size_t len = strnlen(s, INPUT_MAX + 1);
    if (len == INPUT_MAX + 1) return 0;

    if (rev_size < len + 1) return 0;
    /* reversed */
    for (size_t i = 0; i < len; ++i) {
        reversed_out[i] = s[len - 1 - i];
    }
    reversed_out[len] = '\0';

    /* metrics */
    metrics->length = len;
    metrics->letters = 0;
    metrics->digits = 0;

    /* palindrome: alnum-only, case-insensitive */
    static char norm[INPUT_MAX + 1];
    size_t n = 0;
    for (size_t i = 0; i < len && n < INPUT_MAX; ++i) {
        unsigned char uc = (unsigned char)s[i];
        if (isalpha(uc)) {
            metrics->letters++;
            norm[n++] = (char)tolower(uc);
        } else if (isdigit(uc)) {
            metrics->digits++;
            norm[n++] = (char)uc;
        }
    }
    size_t i = 0, j = (n == 0) ? 0 : n - 1;
    int is_pal = 1;
    while (i < j) {
        if (norm[i] != norm[j]) {
            is_pal = 0;
            break;
        }
        ++i; --j;
    }
    metrics->is_palindrome = is_pal;
    return 1;
}

/* Parse integer list from line. Delimiters: spaces or commas.
   Validates each number is within [minV, maxV].
   Writes up to max_count values into out, sets *out_count.
   Returns 1 on success (at least one number), 0 on error. */
static int parse_int_list(const char *line, long long *out, size_t max_count, long long minV, long long maxV, size_t *out_count) {
    if (!line || !out || !out_count || max_count == 0) return 0;

    size_t len = strnlen(line, INPUT_MAX + 1);
    if (len == INPUT_MAX + 1) return 0;

    const char *p = line;
    const char *e = line + len;
    size_t count = 0;

    while (p < e) {
        while (p < e && (isspace((unsigned char)*p) || *p == ',')) ++p;
        if (p >= e) break;

        const char *start = p;

        if (*p == '+' || *p == '-') ++p;
        int has_digit = 0;
        while (p < e && isdigit((unsigned char)*p)) {
            has_digit = 1;
            ++p;
        }
        const char *endTok = p;
        if (!has_digit) return 0;
        if (p < e && !(isspace((unsigned char)*p) || *p == ',')) {
            return 0;
        }

        /* Copy token to buffer for strtoll with full-consumption check */
        size_t tok_len = (size_t)(endTok - start);
        if (tok_len == 0 || tok_len > 64) return 0;
        char buf[65];
        memset(buf, 0, sizeof(buf));
        memcpy(buf, start, tok_len);
        buf[tok_len] = '\0';

        errno = 0;
        char *endptr = NULL;
        long long val = strtoll(buf, &endptr, 10);
        if (errno != 0 || endptr == buf) return 0;
        /* Ensure any trailing chars are whitespace only (should be none) */
        while (*endptr != '\0') {
            if (!isspace((unsigned char)*endptr)) return 0;
            ++endptr;
        }
        if (val < minV || val > maxV) return 0;

        if (count >= max_count) return 0;
        out[count++] = val;
    }

    if (count == 0) return 0;
    *out_count = count;
    return 1;
}

/* Compute statistics over numbers array.
   Returns 1 on success (count>0), 0 otherwise. */
static int analyze_numbers(const long long *arr, size_t count, long long *minV, long long *maxV, long long *sumV, double *avgV) {
    if (!arr || count == 0 || !minV || !maxV || !sumV || !avgV) return 0;
    long long minv = arr[0], maxv = arr[0];
    long long sum = 0;
    for (size_t i = 0; i < count; ++i) {
        if (arr[i] < minv) minv = arr[i];
        if (arr[i] > maxv) maxv = arr[i];
        sum += arr[i];
    }
    *minV = minv;
    *maxV = maxv;
    *sumV = sum;
    *avgV = (double)sum / (double)count;
    return 1;
}

/* -------- Tests -------- */
static void run_string_tests(void) {
    const char *cases[5] = {
        "Hello, World!",
        "RaceCar",
        "123321",
        "abc123",
        ""
    };
    printf("String Tests:\n");
    for (int i = 0; i < 5; ++i) {
        char sanitized[512];
        if (!sanitize_string(cases[i], sanitized, sizeof(sanitized), 256)) {
            printf("Test %d: invalid input\n", i + 1);
            continue;
        }
        char reversed[512];
        StringMetrics m;
        if (!analyze_string(sanitized, reversed, sizeof(reversed), &m)) {
            printf("Test %d: analysis error\n", i + 1);
            continue;
        }
        printf("Test %d: len=%zu letters=%zu digits=%zu palindrome=%s reversed=\"%s\"\n",
               i + 1, m.length, m.letters, m.digits, m.is_palindrome ? "yes" : "no", reversed);
    }
}

static void run_number_tests(void) {
    const char *cases[5] = {
        "1 2 3 4 5",
        "-5, 10, -15, 20",
        "1000000 -1000000 0",
        "42",
        "1, 2, x"
    };
    printf("Number Tests:\n");
    for (int i = 0; i < 5; ++i) {
        long long nums[100];
        size_t count = 0;
        int ok = parse_int_list(cases[i], nums, 100, -1000000LL, 1000000LL, &count);
        if (!ok) {
            printf("Test %d: invalid input\n", i + 1);
            continue;
        }
        long long minv, maxv, sumv;
        double avgv;
        if (!analyze_numbers(nums, count, &minv, &maxv, &sumv, &avgv)) {
            printf("Test %d: analysis error\n", i + 1);
            continue;
        }
        printf("Test %d: count=%zu min=%lld max=%lld sum=%lld avg=%.2f\n",
               i + 1, count, minv, maxv, sumv, avgv);
    }
}

/* -------- Interactive -------- */
static void interactive(void) {
    char buf[INPUT_MAX + 1];
    printf("\nInteractive mode:\n");
    printf("Choose operation:\n");
    printf("1) Analyze a string\n");
    printf("2) Analyze a list of integers\n");
    printf("Enter choice (1 or 2): ");
    int rc = safe_read_line(buf, sizeof(buf), stdin);
    if (rc <= 0) {
        printf("Input error.\n");
        return;
    }
    if (strcmp(buf, "1") != 0 && strcmp(buf, "2") != 0) {
        printf("Invalid choice.\n");
        return;
    }
    if (strcmp(buf, "1") == 0) {
        printf("Enter a printable ASCII string (max 1024 chars): ");
        rc = safe_read_line(buf, sizeof(buf), stdin);
        if (rc <= 0) {
            printf("Input error or too long.\n");
            return;
        }
        char sanitized[1025];
        if (!sanitize_string(buf, sanitized, sizeof(sanitized), 1024)) {
            printf("Invalid characters or too long.\n");
            return;
        }
        char reversed[1025];
        StringMetrics m;
        if (!analyze_string(sanitized, reversed, sizeof(reversed), &m)) {
            printf("Analysis error.\n");
            return;
        }
        printf("Length: %zu\n", m.length);
        printf("Letters: %zu\n", m.letters);
        printf("Digits: %zu\n", m.digits);
        printf("Palindrome (alnum-only, case-insensitive): %s\n", m.is_palindrome ? "yes" : "no");
        printf("Reversed: %s\n", reversed);
    } else {
        printf("Enter integers separated by spaces or commas (range -1000000..1000000, max 100 numbers): ");
        rc = safe_read_line(buf, sizeof(buf), stdin);
        if (rc <= 0) {
            printf("Input error or too long.\n");
            return;
        }
        long long nums[100];
        size_t count = 0;
        if (!parse_int_list(buf, nums, 100, -1000000LL, 1000000LL, &count)) {
            printf("Invalid number list.\n");
            return;
        }
        long long minv, maxv, sumv;
        double avgv;
        if (!analyze_numbers(nums, count, &minv, &maxv, &sumv, &avgv)) {
            printf("Analysis error.\n");
            return;
        }
        printf("Count: %zu\n", count);
        printf("Min: %lld\n", minv);
        printf("Max: %lld\n", maxv);
        printf("Sum: %lld\n", sumv);
        printf("Average: %.2f\n", avgv);
    }
}

int main(void) {
    run_string_tests();
    run_number_tests();
    interactive();
    return 0;
}
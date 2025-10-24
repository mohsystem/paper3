#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

static int is_near_integer(double x) {
    double r = nearbyint(x);
    return fabs(x - r) < 1e-12;
}

static char* format_number(double x) {
    char buf[64];
    if (is_near_integer(x)) {
        // Cast safely to long long where possible
        long long r = llround(x);
        snprintf(buf, sizeof(buf), "%lld", r);
    } else {
        snprintf(buf, sizeof(buf), "%.12g", x);
    }
    char* out = (char*)malloc(strlen(buf) + 1);
    if (!out) return NULL;
    strcpy(out, buf);
    return out;
}

char* process_input(const char* input) {
    if (input == NULL) {
        char* out = (char*)malloc(22);
        if (!out) return NULL;
        strcpy(out, "ERROR: Input is null");
        return out;
    }
    // Make a modifiable copy
    char* copy = (char*)malloc(strlen(input) + 1);
    if (!copy) {
        char* out = (char*)malloc(27);
        if (!out) return NULL;
        strcpy(out, "ERROR: Unexpected error");
        return out;
    }
    strcpy(copy, input);

    // Trim leading and trailing whitespace
    char* s = copy;
    while (*s && (*s == ' ' || *s == '\t' || *s == '\n' || *s == '\r' || *s == '\f' || *s == '\v')) s++;
    char* end = s + strlen(s);
    while (end > s && (end[-1] == ' ' || end[-1] == '\t' || end[-1] == '\n' || end[-1] == '\r' || end[-1] == '\f' || end[-1] == '\v')) end--;
    *end = '\0';

    if (*s == '\0') {
        free(copy);
        char* out = (char*)malloc(20);
        if (!out) return NULL;
        strcpy(out, "ERROR: Empty input");
        return out;
    }

    // Tokenize
    char* saveptr = NULL;
    char* tok = strtok_r(s, " \t\r\n\v\f", &saveptr);
    if (!tok) {
        free(copy);
        char* out = (char*)malloc(20);
        if (!out) return NULL;
        strcpy(out, "ERROR: Empty input");
        return out;
    }
    // Lowercase op
    for (char* p = tok; *p; ++p) {
        if (*p >= 'A' && *p <= 'Z') *p = (char)(*p - 'A' + 'a');
    }
    char op[16];
    strncpy(op, tok, sizeof(op)-1);
    op[sizeof(op)-1] = '\0';

    // Read numbers
    double nums[64];
    int n = 0;
    while ((tok = strtok_r(NULL, " \t\r\n\v\f", &saveptr)) != NULL) {
        char* endp = NULL;
        double v = strtod(tok, &endp);
        if (endp == tok || *endp != '\0') {
            size_t len = strlen(tok) + 23;
            char* out = (char*)malloc(len);
            if (!out) { free(copy); return NULL; }
            snprintf(out, len, "ERROR: Invalid number '%s'", tok);
            free(copy);
            return out;
        }
        if (n < 64) nums[n++] = v;
        else {
            free(copy);
            char* out = (char*)malloc(33);
            if (!out) return NULL;
            strcpy(out, "ERROR: Too many arguments");
            return out;
        }
    }

    char* result = NULL;

    if (strcmp(op, "add") == 0) {
        if (n < 2) {
            result = (char*)malloc(36);
            if (result) strcpy(result, "ERROR: 'add' requires at least 2 numbers");
        } else {
            double sum = 0.0;
            for (int i = 0; i < n; ++i) sum += nums[i];
            if (!isfinite(sum)) {
                result = (char*)malloc(24);
                if (result) strcpy(result, "ERROR: Numeric overflow");
            } else {
                result = format_number(sum);
            }
        }
    } else if (strcmp(op, "mul") == 0) {
        if (n < 2) {
            result = (char*)malloc(36);
            if (result) strcpy(result, "ERROR: 'mul' requires at least 2 numbers");
        } else {
            double prod = 1.0;
            for (int i = 0; i < n; ++i) prod *= nums[i];
            if (!isfinite(prod)) {
                result = (char*)malloc(24);
                if (result) strcpy(result, "ERROR: Numeric overflow");
            } else {
                result = format_number(prod);
            }
        }
    } else if (strcmp(op, "sub") == 0) {
        if (n != 2) {
            result = (char*)malloc(37);
            if (result) strcpy(result, "ERROR: 'sub' requires exactly 2 numbers");
        } else {
            double res = nums[0] - nums[1];
            if (!isfinite(res)) {
                result = (char*)malloc(24);
                if (result) strcpy(result, "ERROR: Numeric overflow");
            } else {
                result = format_number(res);
            }
        }
    } else if (strcmp(op, "div") == 0) {
        if (n != 2) {
            result = (char*)malloc(37);
            if (result) strcpy(result, "ERROR: 'div' requires exactly 2 numbers");
        } else if (nums[1] == 0.0) {
            result = (char*)malloc(25);
            if (result) strcpy(result, "ERROR: Division by zero");
        } else {
            double res = nums[0] / nums[1];
            if (!isfinite(res)) {
                result = (char*)malloc(24);
                if (result) strcpy(result, "ERROR: Numeric overflow");
            } else {
                result = format_number(res);
            }
        }
    } else if (strcmp(op, "pow") == 0) {
        if (n != 2) {
            result = (char*)malloc(37);
            if (result) strcpy(result, "ERROR: 'pow' requires exactly 2 numbers");
        } else {
            double res = pow(nums[0], nums[1]);
            if (!isfinite(res)) {
                result = (char*)malloc(24);
                if (result) strcpy(result, "ERROR: Numeric overflow");
            } else {
                result = format_number(res);
            }
        }
    } else {
        size_t len = strlen(op) + 25;
        result = (char*)malloc(len);
        if (result) snprintf(result, len, "ERROR: Unknown operation '%s'", op);
    }

    free(copy);
    if (!result) {
        result = (char*)malloc(27);
        if (result) strcpy(result, "ERROR: Unexpected error");
    }
    return result;
}

int main(void) {
    const char* tests[5] = {
        "add 2 3 4",
        "div 10 0",
        "pow 2 10",
        "mul 1.5 4",
        "sub 5 abc"
    };
    for (int i = 0; i < 5; ++i) {
        char* out = process_input(tests[i]);
        if (out) {
            printf("%s => %s\n", tests[i], out);
            free(out);
        } else {
            printf("%s => %s\n", tests[i], "ERROR: Unexpected error");
        }
    }
    return 0;
}
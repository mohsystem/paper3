#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

typedef struct {
    bool success;
    int value;
    const char* error;
} Result;

Result convert_to_int(const char* s) {
    if (s == NULL) {
        Result r = {false, 0, "null input"};
        return r;
    }

    const char* p = s;
    while (*p && isspace((unsigned char)*p)) p++;
    if (*p == '\0') {
        Result r = {false, 0, "empty input"};
        return r;
    }

    int sign = 1;
    if (*p == '+' || *p == '-') {
        sign = (*p == '-') ? -1 : 1;
        p++;
    }
    if (*p == '\0') {
        Result r = {false, 0, "no digits"};
        return r;
    }

    long long acc = 0;
    long long limit = (sign == 1) ? INT_MAX : -(long long)INT_MIN; // 2147483648 for negative
    const char* start_digits = p;

    while (*p && *p >= '0' && *p <= '9') {
        int digit = *p - '0';
        if (acc > (limit - digit) / 10LL) {
            Result r = {false, 0, "overflow"};
            return r;
        }
        acc = acc * 10LL + digit;
        p++;
    }

    if (p == start_digits) {
        Result r = {false, 0, "no digits"};
        return r;
    }

    while (*p && isspace((unsigned char)*p)) p++;
    if (*p != '\0') {
        Result r = {false, 0, "invalid character in input"};
        return r;
    }

    int value = (sign == 1) ? (int)acc : (int)(-acc);
    Result r = {true, value, NULL};
    return r;
}

int main(int argc, char* argv[]) {
    const char* inputs[5] = {"123", "-42", "2147483647", "-2147483648", "99abc"};
    const char** arr = inputs;
    int count = 5;

    if (argc > 1) {
        arr = (const char**)&argv[1];
        count = argc - 1;
    }

    for (int i = 0; i < count; i++) {
        const char* s = arr[i];
        Result r = convert_to_int(s);
        if (r.success) {
            printf("Input: \"%s\" -> OK %d\n", s, r.value);
        } else {
            printf("Input: \"%s\" -> ERROR %s\n", s, r.error);
        }
    }
    return 0;
}
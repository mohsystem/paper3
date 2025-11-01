#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>

static const char* VEHICLES[] = {"Car", "Truck", "Bike", "Bus", "Van"};
static const size_t VEHICLES_COUNT = sizeof(VEHICLES) / sizeof(VEHICLES[0]);

static void trim(const char* src, char* dst, size_t dstSize) {
    if (dstSize == 0) return;
    size_t len = src ? strlen(src) : 0;
    size_t start = 0;
    while (start < len && isspace((unsigned char)src[start])) start++;
    size_t end = len;
    while (end > start && isspace((unsigned char)src[end - 1])) end--;
    size_t outLen = end > start ? (end - start) : 0;
    if (outLen >= dstSize) outLen = dstSize - 1;
    if (outLen > 0) {
        memmove(dst, src + start, outLen);
    }
    dst[outLen] = '\0';
}

const char* getVehicleByIndex(const char* indexInput) {
    static const char* ERR_NULL = "Invalid input: null";
    static const char* ERR_EMPTY = "Invalid input: empty";
    static const char* ERR_NOT_INT = "Invalid input: not an integer";
    static const char* ERR_RANGE = "Invalid index: out of range";
    static char err_bounds[64];

    if (indexInput == NULL) {
        return ERR_NULL;
    }

    char buf[128];
    trim(indexInput, buf, sizeof(buf));
    if (buf[0] == '\0') {
        return ERR_EMPTY;
    }

    // Validate format: optional +/- followed by digits
    size_t i = 0;
    if (buf[0] == '+' || buf[0] == '-') i = 1;
    if (buf[i] == '\0') return ERR_NOT_INT;
    for (; buf[i] != '\0'; ++i) {
        if (!isdigit((unsigned char)buf[i])) return ERR_NOT_INT;
    }

    errno = 0;
    char* endptr = NULL;
    long val = strtol(buf, &endptr, 10);
    if (errno == ERANGE) {
        return ERR_RANGE;
    }
    if (endptr == buf || *endptr != '\0') {
        return ERR_NOT_INT;
    }
    if (val < 0 || val >= (long)VEHICLES_COUNT) {
        // Build bounds error message safely
        snprintf(err_bounds, sizeof(err_bounds), "Invalid index: must be between 0 and %zu", VEHICLES_COUNT - 1);
        return err_bounds;
    }
    return VEHICLES[(size_t)val];
}

int main(void) {
    const char* tests[5] = {"2", " 0 ", "-1", "10", "abc"};
    for (size_t t = 0; t < 5; ++t) {
        const char* result = getVehicleByIndex(tests[t]);
        printf("Input: '%s' => %s\n", tests[t], result);
    }
    return 0;
}
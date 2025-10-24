#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <stdbool.h>

#define MAX_INPUT_LENGTH 1000

static bool is_length_safe(const char* s) {
    if (s == NULL) return false;
    size_t n = strlen(s);
    return n > 0 && n <= MAX_INPUT_LENGTH;
}

static bool is_ascii(const char* s) {
    for (const unsigned char* p = (const unsigned char*)s; *p; ++p) {
        if (*p > 0x7F) return false;
    }
    return true;
}

static bool isValidIPv4(const char* s) {
    if (!is_length_safe(s) || !is_ascii(s)) return false;
    int segCount = 0;
    int segLen = 0;
    int segVal = 0;
    bool leadingZero = false;
    int dots = 0;
    size_t n = strlen(s);
    for (size_t i = 0; i < n; ++i) {
        char c = s[i];
        if (c == '.') {
            if (segLen == 0) return false;
            if (leadingZero && segLen > 1) return false;
            if (segVal < 0 || segVal > 255) return false;
            dots++;
            if (dots > 3) return false;
            segCount++;
            segLen = 0;
            segVal = 0;
            leadingZero = false;
        } else if (c >= '0' && c <= '9') {
            if (segLen == 0) {
                if (c == '0') {
                    leadingZero = true;
                    segVal = 0;
                    segLen = 1;
                } else {
                    leadingZero = false;
                    segVal = c - '0';
                    segLen = 1;
                }
            } else {
                if (leadingZero) return false;
                if (segLen >= 3) return false; // max 3 digits
                segVal = segVal * 10 + (c - '0');
                segLen++;
                if (segVal > 255) return false;
            }
        } else {
            return false;
        }
    }
    if (dots != 3) return false;
    if (segLen == 0) return false;
    if (leadingZero && segLen > 1) return false;
    if (segVal < 0 || segVal > 255) return false;
    segCount++;
    return segCount == 4;
}

static char* normalizeIPv4(const char* s) {
    if (!isValidIPv4(s)) {
        char* empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }
    // Parse again to extract segment values
    int vals[4] = {0,0,0,0};
    int idx = 0;
    int segVal = 0;
    int segLen = 0;
    bool leadingZero = false;
    size_t n = strlen(s);
    for (size_t i = 0; i <= n; ++i) {
        char c = (i < n) ? s[i] : '.';
        if (c == '.') {
            vals[idx++] = segVal;
            segVal = 0;
            segLen = 0;
            leadingZero = false;
        } else {
            if (segLen == 0) {
                if (c == '0') {
                    leadingZero = true;
                    segVal = 0;
                    segLen = 1;
                } else {
                    leadingZero = false;
                    segVal = c - '0';
                    segLen = 1;
                }
            } else {
                segVal = segVal * 10 + (c - '0');
                segLen++;
            }
        }
    }
    char* out = (char*)malloc(16); // max "255.255.255.255" + null
    if (!out) return NULL;
    // Use snprintf safely
    int written = snprintf(out, 16, "%d.%d.%d.%d", vals[0], vals[1], vals[2], vals[3]);
    if (written < 0 || written >= 16) {
        free(out);
        return NULL;
    }
    return out;
}

static int parse_hex_group(const char* start, size_t len, uint16_t* out) {
    if (len == 0 || len > 4) return 0;
    uint32_t val = 0;
    for (size_t i = 0; i < len; ++i) {
        unsigned char c = (unsigned char)start[i];
        if (!isxdigit(c)) return 0;
        uint32_t v;
        if (isdigit(c)) v = c - '0';
        else v = 10 + (tolower(c) - 'a');
        val = (val << 4) | v;
        if (val > 0xFFFFu) return 0;
    }
    *out = (uint16_t)val;
    return 1;
}

static int parseIPv6Side(const char* side, uint16_t* out, int* outCount) {
    *outCount = 0;
    if (side[0] == '\0') return 1; // zero groups
    const char* p = side;
    const char* tokenStart = p;
    while (1) {
        if (*p == ':' || *p == '\0') {
            size_t len = (size_t)(p - tokenStart);
            if (len == 0) return 0; // empty group invalid here
            if (*outCount >= 8) return 0;
            uint16_t val;
            if (!parse_hex_group(tokenStart, len, &val)) return 0;
            out[(*outCount)++] = val;
            if (*p == '\0') break;
            p++;
            tokenStart = p;
        } else {
            p++;
        }
    }
    return 1;
}

static int parseIPv6ToGroups(const char* s, uint16_t groups[8]) {
    if (!is_length_safe(s) || !is_ascii(s)) return 0;
    size_t len = strlen(s);
    if (len == 0) return 0;
    // validate allowed chars
    for (size_t i = 0; i < len; ++i) {
        char c = s[i];
        if (!(c == ':' || isxdigit((unsigned char)c))) return 0;
    }
    const char* first = strstr(s, "::");
    if (first != NULL) {
        const char* second = strstr(first + 2, "::");
        if (second != NULL) return 0; // more than one '::'
        size_t leftLen = (size_t)(first - s);
        size_t rightLen = len - leftLen - 2;
        char* left = (char*)malloc(leftLen + 1);
        char* right = (char*)malloc(rightLen + 1);
        if (!left || !right) {
            free(left);
            free(right);
            return 0;
        }
        memcpy(left, s, leftLen);
        left[leftLen] = '\0';
        memcpy(right, first + 2, rightLen);
        right[rightLen] = '\0';

        uint16_t lg[8], rg[8];
        int lcount = 0, rcount = 0;
        int okL = parseIPv6Side(left, lg, &lcount);
        int okR = parseIPv6Side(right, rg, &rcount);
        free(left);
        free(right);
        if (!okL || !okR) return 0;
        if (lcount + rcount >= 8) return 0;
        int idx = 0;
        for (int i = 0; i < lcount; ++i) groups[idx++] = lg[i];
        int zeros = 8 - (lcount + rcount);
        for (int i = 0; i < zeros; ++i) groups[idx++] = 0;
        for (int i = 0; i < rcount; ++i) groups[idx++] = rg[i];
        if (idx != 8) return 0;
        return 1;
    } else {
        uint16_t tmp[8];
        int count = 0;
        if (!parseIPv6Side(s, tmp, &count)) return 0;
        if (count != 8) return 0;
        for (int i = 0; i < 8; ++i) groups[i] = tmp[i];
        return 1;
    }
}

static bool isValidIPv6(const char* s) {
    uint16_t g[8];
    return parseIPv6ToGroups(s, g) ? true : false;
}

static char* normalizeIPv6(const char* s) {
    uint16_t g[8];
    if (!parseIPv6ToGroups(s, g)) {
        char* empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }
    // Expanded form length = 39 chars + null
    char buf[41];
    size_t pos = 0;
    for (int i = 0; i < 8; ++i) {
        int n = snprintf(buf + pos, sizeof(buf) - pos, "%04x%s", g[i], (i < 7 ? ":" : ""));
        if (n < 0) {
            char* empty = (char*)malloc(1);
            if (empty) empty[0] = '\0';
            return empty;
        }
        pos += (size_t)n;
        if (pos >= sizeof(buf)) {
            char* empty = (char*)malloc(1);
            if (empty) empty[0] = '\0';
            return empty;
        }
    }
    char* out = (char*)malloc(pos + 1);
    if (!out) return NULL;
    memcpy(out, buf, pos + 1);
    return out;
}

static const char* detectIPType(const char* s) {
    if (!is_length_safe(s) || !is_ascii(s)) return "Invalid";
    if (isValidIPv4(s)) return "IPv4";
    if (isValidIPv6(s)) return "IPv6";
    return "Invalid";
}

static char* processIP(const char* s) {
    const char* t = detectIPType(s);
    if (strcmp(t, "IPv4") == 0) {
        char* norm = normalizeIPv4(s);
        if (!norm) return NULL;
        size_t need = strlen(norm) + 25;
        char* out = (char*)malloc(need);
        if (!out) { free(norm); return NULL; }
        snprintf(out, need, "Type: IPv4, Normalized: %s", norm);
        free(norm);
        return out;
    } else if (strcmp(t, "IPv6") == 0) {
        char* norm = normalizeIPv6(s);
        if (!norm) return NULL;
        size_t need = strlen(norm) + 25;
        char* out = (char*)malloc(need);
        if (!out) { free(norm); return NULL; }
        snprintf(out, need, "Type: IPv6, Normalized: %s", norm);
        free(norm);
        return out;
    } else {
        char* out = (char*)malloc(8);
        if (!out) return NULL;
        strcpy(out, "Invalid");
        return out;
    }
}

int main(void) {
    const char* tests[5] = {
        "192.168.1.1",
        "255.255.255.255",
        "01.2.3.4",
        "2001:0db8:85a3:0000:0000:8a2e:0370:7334",
        "2001:db8::8a2e:370:7334"
    };
    for (int i = 0; i < 5; ++i) {
        char* res = processIP(tests[i]);
        if (res) {
            printf("%s => %s\n", tests[i], res);
            free(res);
        } else {
            printf("%s => (processing error)\n", tests[i]);
        }
    }
    return 0;
}
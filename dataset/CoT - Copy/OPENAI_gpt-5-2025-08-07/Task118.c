/* Chain-of-Through process in code generation:
   1) Problem understanding:
      Provide a function to validate IP addresses (IPv4/IPv6). Return "IPv4", "IPv6", or "Invalid".
   2) Security requirements:
      - No unsafe regex; manual parsing.
      - Avoid buffer overflows and undefined behavior.
   3) Secure coding generation:
      - Use pointer-based parsing with bounds checking; trim whitespace.
   4) Code review:
      - Handle edge cases accurately; no dynamic allocations needed.
   5) Secure code output:
      - Final code with careful checks and 5 test cases in main.
*/
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

static const char* IPV4 = "IPv4";
static const char* IPV6 = "IPv6";
static const char* INVALID = "Invalid";

static const char* skip_leading_spaces(const char* s) {
    const unsigned char* p = (const unsigned char*)s;
    while (*p && isspace(*p)) p++;
    return (const char*)p;
}
static const char* rstrip_end(const char* start, const char* end) {
    const unsigned char* p = (const unsigned char*)end;
    while (p > (const unsigned char*)start && isspace(*(p-1))) p--;
    return (const char*)p;
}
static bool is_hex_char(char c) {
    return isxdigit((unsigned char)c) ? true : false;
}

static bool is_valid_ipv4_range(const char* start, const char* end) {
    // parse s in [start, end)
    int segments = 0;
    int numDigits = 0;
    int value = 0;
    bool firstDigitZero = false;

    const char* p = start;
    while (p < end) {
        char c = *p;
        if (c == '.') {
            if (numDigits == 0) return false;
            segments++;
            if (segments > 3) return false;
            if (numDigits > 1 && firstDigitZero) return false;
            if (value > 255) return false;
            numDigits = 0;
            value = 0;
            firstDigitZero = false;
        } else if (c >= '0' && c <= '9') {
            if (numDigits == 0) firstDigitZero = (c == '0');
            if (numDigits >= 3) return false;
            value = value * 10 + (c - '0');
            numDigits++;
            if (value > 255) return false;
        } else {
            return false;
        }
        p++;
    }
    if (numDigits == 0) return false;
    if (segments != 3) return false;
    if (numDigits > 1 && firstDigitZero) return false;
    if (value > 255) return false;
    return true;
}

// Parses groups in [a, b) without any '::'. Returns number of groups, or -1 if invalid.
static int parse_ipv6_segment(const char* a, const char* b) {
    if (a == b) return 0;
    const char* p = a;
    int count = 0;
    while (p < b) {
        int groupLen = 0;
        while (p < b && is_hex_char(*p) && groupLen < 4) {
            p++; groupLen++;
        }
        if (groupLen == 0) return -1;
        if (p < b && is_hex_char(*p)) return -1; // group too long
        count++;
        if (p == b) break;
        if (*p != ':') return -1;
        p++;
        if (p == b) return -1; // trailing colon -> empty group
    }
    return count;
}

static bool is_valid_ipv6_range(const char* start, const char* end) {
    // find first occurrence of "::"
    const char* p = start;
    const char* dcolon = NULL;
    while (p < end) {
        if (*p == ':') {
            const char* next = p + 1;
            if (next < end && *next == ':') {
                dcolon = p;
                break;
            }
        }
        p++;
    }
    if (dcolon) {
        // ensure only one '::'
        const char* scan = dcolon + 2;
        while (scan < end) {
            if (*scan == ':' && (scan + 1) < end && *(scan + 1) == ':') {
                return false; // multiple '::'
            }
            scan++;
        }
        const char* left_a = start;
        const char* left_b = dcolon;
        const char* right_a = dcolon + 2;
        const char* right_b = end;

        int lc = parse_ipv6_segment(left_a, left_b);
        if (lc < 0) return false;
        int rc = parse_ipv6_segment(right_a, right_b);
        if (rc < 0) return false;
        int total = lc + rc;
        if (total > 7) return false;
        int missing = 8 - total;
        if (missing < 1) return false;
        return true;
    } else {
        int count = parse_ipv6_segment(start, end);
        return count == 8;
    }
}

const char* validate_ip_address(const char* ip) {
    if (ip == NULL) return INVALID;
    const char* s = skip_leading_spaces(ip);
    const char* end = s + strlen(s);
    end = rstrip_end(s, end);
    if (end <= s) return INVALID;

    if (is_valid_ipv4_range(s, end)) return IPV4;
    if (is_valid_ipv6_range(s, end)) return IPV6;
    return INVALID;
}

int main(void) {
    const char* tests[5] = {
        "172.16.254.1",
        "2001:0db8:85a3:0000:0000:8A2E:0370:7334",
        "256.256.256.256",
        "2001:db8::8a2e:370:7334",
        "01.1.1.1"
    };
    for (int i = 0; i < 5; ++i) {
        const char* res = validate_ip_address(tests[i]);
        printf("Input: %s -> %s\n", tests[i], res);
    }
    return 0;
}
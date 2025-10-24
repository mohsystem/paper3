#include <stdio.h>
#include <string.h>
#include <ctype.h>

static int is_hex_group_token(const char* s, size_t len) {
    if (len < 1 || len > 4) return 0;
    for (size_t i = 0; i < len; i++) {
        unsigned char ch = (unsigned char)s[i];
        if (!(isdigit(ch) ||
              (ch >= 'a' && ch <= 'f') ||
              (ch >= 'A' && ch <= 'F'))) {
            return 0;
        }
    }
    return 1;
}

static int parse_ipv6_side(const char* s, size_t len, int* count_out) {
    if (len == 0) {
        *count_out = 0;
        return 1;
    }
    size_t i = 0;
    int count = 0;
    while (i < len) {
        size_t j = i;
        while (j < len && s[j] != ':') j++;
        size_t tokLen = j - i;
        if (tokLen == 0) return 0;
        if (!is_hex_group_token(s + i, tokLen)) return 0;
        count++;
        if (j == len) break;
        i = j + 1;
        if (i == len) return 0; // trailing colon -> empty token
    }
    *count_out = count;
    return 1;
}

int isValidIPv4(const char* ip) {
    if (ip == NULL || *ip == '\0') return 0;
    int seg = 0;
    const char* p = ip;
    while (*p) {
        if (seg == 4) return 0;
        if (!isdigit((unsigned char)*p)) return 0;
        if (*p == '0') {
            p++;
            if (isdigit((unsigned char)*p)) return 0;
        } else {
            int val = 0;
            int len = 0;
            while (*p && isdigit((unsigned char)*p)) {
                if (len >= 3) return 0;
                val = val * 10 + (*p - '0');
                if (val > 255) return 0;
                len++;
                p++;
            }
            if (len == 0) return 0;
        }
        seg++;
        if (seg < 4) {
            if (*p != '.') return 0;
            p++;
            if (*p == '\0') return 0;
        } else {
            if (*p != '\0') return 0;
        }
    }
    return seg == 4;
}

int isValidIPv6(const char* ip) {
    if (ip == NULL || *ip == '\0') return 0;
    // do not support embedded IPv4
    for (const char* t = ip; *t; ++t) {
        if (*t == '.') return 0;
    }
    const char* dc = strstr(ip, "::");
    if (dc) {
        const char* next = strstr(dc + 2, "::");
        if (next) return 0;
        size_t leftLen = (size_t)(dc - ip);
        const char* rightPtr = dc + 2;
        size_t rightLen = strlen(rightPtr);
        int leftCount = 0, rightCount = 0;
        if (leftLen > 0) {
            if (!parse_ipv6_side(ip, leftLen, &leftCount)) return 0;
        }
        if (rightLen > 0) {
            if (!parse_ipv6_side(rightPtr, rightLen, &rightCount)) return 0;
        }
        int total = leftCount + rightCount;
        return total < 8;
    } else {
        int count = 0;
        if (!parse_ipv6_side(ip, strlen(ip), &count)) return 0;
        return count == 8;
    }
}

const char* classifyIP(const char* ip) {
    if (isValidIPv4(ip)) return "IPv4";
    if (isValidIPv6(ip)) return "IPv6";
    return "Invalid";
}

int main(void) {
    const char* tests[5] = {
        "192.168.1.1",
        "255.256.0.1",
        "2001:0db8:85a3:0000:0000:8a2e:0370:7334",
        "2001:db8::1",
        "01.1.1.1"
    };
    for (int i = 0; i < 5; i++) {
        printf("%s => %s\n", tests[i], classifyIP(tests[i]));
    }
    return 0;
}
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

int isValidIPv4C(const char* ip) {
    if (ip == NULL) return 0;
    // trim spaces
    const char* s = ip;
    while (*s && isspace((unsigned char)*s)) s++;
    size_t len = strlen(s);
    while (len > 0 && isspace((unsigned char)s[len-1])) len--;
    if (len == 0) return 0;

    if (s[0] == '.' || s[len-1] == '.') return 0;

    int parts = 0;
    int i = 0;
    while (i < (int)len) {
        int start = i;
        int digits = 0;
        while (i < (int)len && s[i] != '.') {
            if (!isdigit((unsigned char)s[i])) return 0;
            digits++;
            if (digits > 3) return 0;
            i++;
        }
        if (digits == 0) return 0;
        if (digits > 1 && s[start] == '0') return 0;
        char buf[4] = {0};
        memcpy(buf, s + start, (size_t)digits);
        int val = atoi(buf);
        if (val < 0 || val > 255) return 0;
        parts++;
        if (i < (int)len && s[i] == '.') i++; // skip dot
    }
    return parts == 4;
}

static int is_hex_digit(char c) {
    return isdigit((unsigned char)c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

static int is_hex_group(const char* s, int len) {
    if (len < 1 || len > 4) return 0;
    for (int i = 0; i < len; i++) if (!is_hex_digit(s[i])) return 0;
    return 1;
}

int isValidIPv6C(const char* ip) {
    if (ip == NULL) return 0;
    // trim
    const char* start = ip;
    while (*start && isspace((unsigned char)*start)) start++;
    const char* end = ip + strlen(ip);
    while (end > start && isspace((unsigned char)*(end-1))) end--;
    if (start == end) return 0;

    for (const char* p = start; p < end; ++p) if (*p == '.') return 0; // no embedded IPv4 support

    // find '::'
    const char* dc = NULL;
    for (const char* p = start; p + 1 < end; ++p) {
        if (*p == ':' && *(p+1) == ':') {
            if (dc != NULL) return 0; // multiple '::'
            dc = p;
            p++; // skip next ':'
        }
    }

    int groups = 0;

    if (dc) {
        // left side
        const char* p = start;
        const char* segStart = p;
        if (p == dc) {
            // empty left side
        } else {
            while (p < dc) {
                if (*p == ':') {
                    int segLen = (int)(p - segStart);
                    if (segLen == 0) return 0;
                    if (!is_hex_group(segStart, segLen)) return 0;
                    groups++;
                    segStart = p + 1;
                }
                p++;
            }
            // last segment before '::'
            if (segStart != dc) {
                int segLen = (int)(dc - segStart);
                if (segLen == 0) return 0;
                if (!is_hex_group(segStart, segLen)) return 0;
                groups++;
            }
        }
        // right side
        p = dc + 2;
        segStart = p;
        if (p == end) {
            // empty right side
        } else {
            while (p < end) {
                if (*p == ':') {
                    int segLen = (int)(p - segStart);
                    if (segLen == 0) return 0;
                    if (!is_hex_group(segStart, segLen)) return 0;
                    groups++;
                    segStart = p + 1;
                }
                p++;
            }
            if (segStart != end) {
                int segLen = (int)(end - segStart);
                if (segLen == 0) return 0;
                if (!is_hex_group(segStart, segLen)) return 0;
                groups++;
            }
        }
        if (groups > 7) return 0; // must compress at least one group
        return 1;
    } else {
        // no '::', must be exactly 8 groups
        const char* p = start;
        const char* segStart = p;
        while (p < end) {
            if (*p == ':') {
                int segLen = (int)(p - segStart);
                if (segLen == 0) return 0;
                if (!is_hex_group(segStart, segLen)) return 0;
                groups++;
                segStart = p + 1;
            }
            p++;
        }
        if (segStart == end) return 0;
        if (!is_hex_group(segStart, (int)(end - segStart))) return 0;
        groups++;
        return groups == 8;
    }
}

const char* validateIPAddressC(const char* ip) {
    if (isValidIPv4C(ip)) return "IPv4";
    if (isValidIPv6C(ip)) return "IPv6";
    return "Neither";
}

int normalizeIPv4C(const char* ip, char* out /* size >= 16 */) {
    if (!isValidIPv4C(ip)) return 0;
    // trim
    const char* s = ip;
    while (*s && isspace((unsigned char)*s)) s++;
    size_t len = strlen(s);
    while (len > 0 && isspace((unsigned char)s[len-1])) len--;
    int i = 0, partIdx = 0;
    char buffer[4] = {0};
    int outPos = 0;
    while (i < (int)len) {
        int start = i, digits = 0;
        while (i < (int)len && s[i] != '.') {
            buffer[digits++] = s[i];
            i++;
        }
        buffer[digits] = '\0';
        int val = atoi(buffer);
        if (partIdx > 0) out[outPos++] = '.';
        outPos += sprintf(out + outPos, "%d", val);
        partIdx++;
        if (i < (int)len && s[i] == '.') i++;
    }
    out[outPos] = '\0';
    return 1;
}

static void pad4_upper_copy(const char* src, int len, char* dst /* must have >=5 */) {
    // src len 1..4
    int pad = 4 - len;
    for (int i = 0; i < pad; ++i) dst[i] = '0';
    for (int i = 0; i < len; ++i) {
        char c = src[i];
        dst[pad + i] = (char)toupper((unsigned char)c);
    }
    dst[4] = '\0';
}

int normalizeIPv6C(const char* ip, char* out /* size >= 64 */) {
    if (!isValidIPv6C(ip)) return 0;
    // trim
    const char* start = ip;
    while (*start && isspace((unsigned char)*start)) start++;
    const char* end = ip + strlen(ip);
    while (end > start && isspace((unsigned char)*(end-1))) end--;

    const char* dc = NULL;
    for (const char* p = start; p + 1 < end; ++p) {
        if (*p == ':' && *(p+1) == ':') { dc = p; break; }
    }

    const char* segs[8];
    int segLens[8];
    int count = 0;

    if (dc) {
        // left side segments
        const char* p = start;
        const char* segStart = p;
        while (p < dc) {
            if (*p == ':') {
                segs[count] = segStart;
                segLens[count] = (int)(p - segStart);
                count++;
                segStart = p + 1;
            }
            p++;
        }
        if (segStart != dc) {
            segs[count] = segStart;
            segLens[count] = (int)(dc - segStart);
            count++;
        }
        // right side segments
        const char* right = dc + 2;
        // determine how many zeros to insert
        int rightCount = 0;
        if (right < end) {
            const char* q = right;
            const char* rSegStart = q;
            while (q < end) {
                if (*q == ':') {
                    rightCount++;
                    q++;
                    rSegStart = q;
                } else {
                    q++;
                }
            }
            if (rSegStart < end) rightCount++;
        } else {
            rightCount = 0;
        }
        int zerosToInsert = 8 - (count + rightCount);
        for (int i = 0; i < zerosToInsert; ++i) {
            segs[count] = "0";
            segLens[count] = 1;
            count++;
        }
        if (right < end) {
            const char* q = right;
            const char* rSegStart = q;
            while (q < end) {
                if (*q == ':') {
                    segs[count] = rSegStart;
                    segLens[count] = (int)(q - rSegStart);
                    count++;
                    rSegStart = q + 1;
                }
                q++;
            }
            if (rSegStart < end) {
                segs[count] = rSegStart;
                segLens[count] = (int)(end - rSegStart);
                count++;
            }
        }
    } else {
        // no '::', exactly 8 segments
        const char* p = start;
        const char* segStart = p;
        while (p < end) {
            if (*p == ':') {
                segs[count] = segStart;
                segLens[count] = (int)(p - segStart);
                count++;
                segStart = p + 1;
            }
            p++;
        }
        if (segStart < end) {
            segs[count] = segStart;
            segLens[count] = (int)(end - segStart);
            count++;
        }
    }

    if (count != 8) return 0;
    int pos = 0;
    for (int i = 0; i < 8; ++i) {
        char buf[5];
        pad4_upper_copy(segs[i], segLens[i], buf);
        if (i > 0) out[pos++] = ':';
        memcpy(out + pos, buf, 4);
        pos += 4;
    }
    out[pos] = '\0';
    return 1;
}

int main(void) {
    const char* tests[5] = {
        "172.16.254.1",
        "2001:0db8:85a3:0000:0000:8A2E:0370:7334",
        "2001:db8:85a3::8a2e:370:7334",
        "256.256.256.256",
        "01.1.1.1"
    };
    for (int i = 0; i < 5; ++i) {
        const char* ip = tests[i];
        const char* kind = validateIPAddressC(ip);
        printf("IP: %s -> %s\n", ip, kind);
        if (strcmp(kind, "IPv4") == 0) {
            char out[32];
            if (normalizeIPv4C(ip, out)) {
                printf("Normalized IPv4: %s\n", out);
            }
        } else if (strcmp(kind, "IPv6") == 0) {
            char out[64];
            if (normalizeIPv6C(ip, out)) {
                printf("Normalized IPv6: %s\n", out);
            }
        }
        printf("\n");
    }
    return 0;
}
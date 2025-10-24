#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <limits.h>

#define MAX_URL_LENGTH 2048

static bool is_hex(char c) {
    return (c>='0' && c<='9') || (c>='A' && c<='F') || (c>='a' && c<='f');
}
static bool is_alnum_c(char c) {
    return (c>='A' && c<='Z') || (c>='a' && c<='z') || (c>='0' && c<='9');
}
static bool is_unreserved(char c) {
    return is_alnum_c(c) || c=='-' || c=='.' || c=='_' || c=='~';
}
static bool is_subdelim(char c) {
    switch(c) {
        case '!': case '$': case '&': case '\'': case '(': case ')':
        case '*': case '+': case ',': case ';': case '=': return true;
        default: return false;
    }
}
static bool is_pchar(char c) {
    return is_unreserved(c) || is_subdelim(c) || c==':' || c=='@';
}
static bool all_digits(const char* s, size_t len) {
    if (len == 0) return false;
    for (size_t i=0;i<len;i++) if (!isdigit((unsigned char)s[i])) return false;
    return true;
}
static int index_of_first(const char* s, int from, char a, char b, char c) {
    int n = (int)strlen(s);
    int res = -1;
    for (int i=from;i<n;i++) {
        if (s[i]==a || s[i]==b || s[i]==c) { res = i; break; }
    }
    return res;
}
static bool validate_percent(const char* s, int i, int n) {
    if (i+2 >= n) return false;
    return is_hex(s[i+1]) && is_hex(s[i+2]);
}
static bool is_valid_path(const char* path, int len) {
    for (int i=0;i<len;i++) {
        char ch = path[i];
        if (ch == '%') {
            if (!validate_percent(path, i, len)) return false;
            i += 2;
        } else if (ch == '/' || is_pchar(ch)) {
            continue;
        } else {
            return false;
        }
    }
    return true;
}
static bool is_valid_query_or_fragment(const char* part, int len) {
    for (int i=0;i<len;i++) {
        char ch = part[i];
        if (ch == '%') {
            if (!validate_percent(part, i, len)) return false;
            i += 2;
        } else if (is_pchar(ch) || ch=='/' || ch=='?') {
            continue;
        } else {
            return false;
        }
    }
    return true;
}
static bool is_valid_ipv4(const char* s, int len) {
    int count = 0;
    int i = 0;
    while (i <= len) {
        int start = i;
        int dot = -1;
        while (i < len && s[i] != '.') i++;
        dot = i;
        int partLen = dot - start;
        if (partLen <= 0 || partLen > 3) return false;
        for (int k=0;k<partLen;k++) if (!isdigit((unsigned char)s[start+k])) return false;
        int val = 0;
        for (int k=0;k<partLen;k++) { val = val*10 + (s[start+k]-'0'); if (val > 255) break; }
        if (val < 0 || val > 255) return false;
        count++;
        if (i == len) break;
        i++; // skip '.'
    }
    return count == 4;
}
static bool is_valid_domain_label(const char* s, int len) {
    if (len <= 0 || len > 63) return false;
    if (!is_alnum_c(s[0]) || !is_alnum_c(s[len-1])) return false;
    for (int i=0;i<len;i++) {
        char ch = s[i];
        if (!(is_alnum_c(ch) || ch=='-')) return false;
    }
    return true;
}
static bool is_valid_host(const char* host, int len) {
    if (len <= 0) return false;
    if (len == 9 && strncasecmp(host, "localhost", 9) == 0) return true;
    if (is_valid_ipv4(host, len)) return true;
    if (len > 253) return false;

    // Split by '.'
    int labelCount = 0;
    int i = 0;
    int lastStart = 0;
    bool ok = true;
    while (i <= len) {
        if (i == len || host[i] == '.') {
            int l = i - lastStart;
            if (!is_valid_domain_label(host + lastStart, l)) { ok = false; break; }
            labelCount++;
            lastStart = i + 1;
        }
        i++;
    }
    if (!ok) return false;
    if (labelCount < 2) return false;

    // TLD checks
    // Extract last label
    int end = len - 1;
    int j = len - 1;
    while (j >= 0 && host[j] != '.') j--;
    int tldStart = j + 1;
    int tldLen = len - tldStart;
    if (tldLen < 2) return false;

    bool alphaTld = true;
    for (int k=0;k<tldLen;k++) {
        char ch = host[tldStart + k];
        if (!((ch>='A' && ch<='Z') || (ch>='a' && ch<='z') || ch=='-')) { alphaTld = false; break; }
    }
    if (!alphaTld) {
        if (!(tldLen >= 4 &&
              (towlower(host[tldStart+0])=='x') &&
              (towlower(host[tldStart+1])=='n') &&
              (towlower(host[tldStart+2])=='-') &&
              (towlower(host[tldStart+3])=='-'))) {
            return false;
        }
    }
    return true;
}
static bool validate_ipv6_parts(const char* s, int start, int end) {
    // parts are separated by ':', none should be empty and each 1..4 hex
    int i = start;
    int segStart = start;
    if (start == end) return true; // empty side is allowed by caller
    while (i <= end) {
        if (i == end || s[i] == ':') {
            int l = i - segStart;
            if (l <= 0 || l > 4) return false;
            for (int k=0;k<l;k++) if (!is_hex(s[segStart+k])) return false;
            segStart = i + 1;
        }
        i++;
    }
    return true;
}
// Simplified IPv6 validation (no IPv4-mapped)
static bool is_valid_ipv6(const char* s, int len) {
    if (len <= 0) return false;
    int colonCount = 0;
    for (int i=0;i<len;i++) if (s[i]==':') colonCount++;
    if (colonCount < 2) return false;

    int ddc = -1;
    for (int i=0;i<len-1;i++) {
        if (s[i]==':' && s[i+1]==':') { ddc = i; break; }
    }
    bool hasDD = ddc != -1;
    if (hasDD) {
        // ensure only one '::'
        for (int i=ddc+2;i<len-1;i++) {
            if (s[i]==':' && s[i+1]==':') return false;
        }
    }
    int leftStart = 0;
    int leftEnd = hasDD ? ddc : len;
    int rightStart = hasDD ? ddc + 2 : len;
    int rightEnd = len;

    // Count parts
    int leftParts = 0;
    if (leftEnd > leftStart) {
        int i = leftStart;
        while (i <= leftEnd) {
            if (i == leftEnd || s[i] == ':') { leftParts++; }
            i++;
        }
    }
    int rightParts = 0;
    if (rightEnd > rightStart) {
        int i = rightStart;
        while (i <= rightEnd) {
            if (i == rightEnd || s[i] == ':') { rightParts++; }
            i++;
        }
    }

    if (!validate_ipv6_parts(s, leftStart, leftEnd)) return false;
    if (!validate_ipv6_parts(s, rightStart, rightEnd)) return false;

    int total = leftParts + rightParts;
    if (hasDD) {
        return total < 8;
    } else {
        return total == 8;
    }
}

bool is_valid_url(const char* s) {
    if (s == NULL) return false;
    int n = (int)strlen(s);
    if (n == 0 || n > MAX_URL_LENGTH) return false;
    for (int i=0;i<n;i++) {
        unsigned char uc = (unsigned char)s[i];
        if (uc < 0x21 || uc > 0x7E) return false;
    }
    // find ://
    const char* schemeSepPtr = strstr(s, "://");
    if (!schemeSepPtr) return false;
    int schemeSep = (int)(schemeSepPtr - s);
    if (schemeSep <= 0) return false;
    // scheme
    bool schemeOk = false;
    if (schemeSep == 4 &&
        tolower(s[0])=='h' && tolower(s[1])=='t' && tolower(s[2])=='t' && tolower(s[3])=='p') schemeOk = true;
    if (schemeSep == 5 &&
        tolower(s[0])=='h' && tolower(s[1])=='t' && tolower(s[2])=='t' && tolower(s[3])=='p' && tolower(s[4])=='s') schemeOk = true;
    if (schemeSep == 3 &&
        tolower(s[0])=='f' && tolower(s[1])=='t' && tolower(s[2])=='p') schemeOk = true;
    if (!schemeOk) return false;

    int pos = schemeSep + 3;
    int authorityEnd = index_of_first(s, pos, '/', '?', '#');
    if (authorityEnd == -1) authorityEnd = n;
    if (authorityEnd <= pos) return false;
    int authLen = authorityEnd - pos;
    const char* authority = s + pos;
    // disallow userinfo
    for (int i=0;i<authLen;i++) if (authority[i]=='@') return false;

    int port = -1;
    // parse host and port
    if (authority[0] == '[') {
        // IPv6
        int close = -1;
        for (int i=1;i<authLen;i++) if (authority[i]==']') { close = i; break; }
        if (close <= 1) return false;
        if (!is_valid_ipv6(authority+1, close-1)) return false;
        if (close + 1 < authLen) {
            if (authority[close+1] != ':') return false;
            int portLen = authLen - (close + 2);
            if (portLen <= 0 || portLen > 5) return false;
            for (int i=0;i<portLen;i++) if (!isdigit((unsigned char)authority[close+2+i])) return false;
            int val = 0;
            for (int i=0;i<portLen;i++) { val = val*10 + (authority[close+2+i]-'0'); if (val > 70000) break; }
            if (val < 1 || val > 65535) return false;
            port = val;
        }
    } else {
        // find last ':'
        int colon = -1;
        for (int i=authLen-1;i>=0;i--) {
            if (authority[i]==':') { colon = i; break; }
        }
        int hostLen = authLen;
        if (colon != -1) hostLen = colon;
        if (hostLen <= 0) return false;
        const char* host = authority;
        if (colon != -1) {
            int portLen = authLen - (colon + 1);
            if (portLen <= 0 || portLen > 5) return false;
            for (int i=0;i<portLen;i++) if (!isdigit((unsigned char)authority[colon+1+i])) return false;
            int val = 0;
            for (int i=0;i<portLen;i++) { val = val*10 + (authority[colon+1+i]-'0'); if (val > 70000) break; }
            if (val < 1 || val > 65535) return false;
            port = val;
        }
        // validate host
        if (hostLen == 9 && strncasecmp(host, "localhost", 9) == 0) {
            // ok
        } else if (is_valid_ipv4(host, hostLen)) {
            // ok
        } else {
            if (hostLen > 253) return false;
            // split by '.'
            int i = 0;
            int lastStart = 0;
            int labelCount = 0;
            while (i <= hostLen) {
                if (i == hostLen || host[i] == '.') {
                    int l = i - lastStart;
                    if (!is_valid_domain_label(host + lastStart, l)) return false;
                    labelCount++;
                    lastStart = i + 1;
                }
                i++;
            }
            if (labelCount < 2) return false;
            int j = hostLen - 1;
            while (j >= 0 && host[j] != '.') j--;
            int tldStart = j + 1;
            int tldLen = hostLen - tldStart;
            if (tldLen < 2) return false;
            bool alphaTld = true;
            for (int k=0;k<tldLen;k++) {
                char ch = host[tldStart + k];
                if (!((ch>='A' && ch<='Z') || (ch>='a' && ch<='z') || ch=='-')) { alphaTld = false; break; }
            }
            if (!alphaTld) {
                if (!(tldLen >= 4 &&
                      tolower(host[tldStart+0])=='x' &&
                      tolower(host[tldStart+1])=='n' &&
                      tolower(host[tldStart+2])=='-' &&
                      tolower(host[tldStart+3])=='-')) {
                    return false;
                }
            }
        }
    }

    // Path
    int idx = authorityEnd;
    int qPos = -1, fPos = -1;
    for (int i=idx;i<n;i++) if (s[i]=='?') { qPos = i; break; }
    for (int i=idx;i<n;i++) if (s[i]=='#') { fPos = i; break; }
    int pathEnd = n;
    if (qPos != -1 && qPos < pathEnd) pathEnd = qPos;
    if (fPos != -1 && fPos < pathEnd) pathEnd = fPos;
    if (!is_valid_path(s + idx, pathEnd - idx)) return false;

    // Query
    if (qPos != -1) {
        int queryEnd = (fPos != -1) ? fPos : n;
        if (!is_valid_query_or_fragment(s + qPos + 1, queryEnd - (qPos + 1))) return false;
    }
    // Fragment
    if (fPos != -1) {
        if (!is_valid_query_or_fragment(s + fPos + 1, n - (fPos + 1))) return false;
    }
    return true;
}

int main(void) {
    const char* tests[5] = {
        "https://example.com",
        "http://sub.example.co.uk:8080/path?query=ok#frag",
        "ftp://[2001:db8::1]/",
        "http://256.256.256.256",
        "https://example.com:70000"
    };
    for (int i=0;i<5;i++) {
        printf("%s -> %s\n", tests[i], is_valid_url(tests[i]) ? "true" : "false");
    }
    return 0;
}
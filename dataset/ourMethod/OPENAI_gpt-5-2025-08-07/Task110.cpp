#include <bits/stdc++.h>
using namespace std;

static const size_t MAX_URL_LENGTH = 2048;

static bool isHex(char c) {
    return (c >= '0' && c <= '9') ||
           (c >= 'A' && c <= 'F') ||
           (c >= 'a' && c <= 'f');
}
static bool isAlphaNum(char c) {
    return (c >= 'A' && c <= 'Z') ||
           (c >= 'a' && c <= 'z') ||
           (c >= '0' && c <= '9');
}
static bool isUnreserved(char c) {
    return isAlphaNum(c) || c=='-' || c=='.' || c=='_' || c=='~';
}
static bool isSubDelim(char c) {
    switch(c) {
        case '!': case '$': case '&': case '\'': case '(': case ')':
        case '*': case '+': case ',': case ';': case '=': return true;
        default: return false;
    }
}
static bool isPchar(char c) {
    return isUnreserved(c) || isSubDelim(c) || c==':' || c=='@';
}
static bool isAllDigits(const string& s) {
    if (s.empty()) return false;
    for (char c : s) if (!isdigit(static_cast<unsigned char>(c))) return false;
    return true;
}

static int indexOfFirst(const string& s, int from, const vector<char>& chars) {
    int res = -1;
    for (char ch : chars) {
        size_t pos = s.find(ch, static_cast<size_t>(from));
        if (pos != string::npos) {
            if (res == -1) res = static_cast<int>(pos);
            else res = min(res, static_cast<int>(pos));
        }
    }
    return res;
}

static bool validatePercent(const string& s, size_t i) {
    if (i + 2 >= s.size()) return false;
    return isHex(s[i+1]) && isHex(s[i+2]);
}

static bool isValidPath(const string& path) {
    for (size_t i = 0; i < path.size(); ++i) {
        char ch = path[i];
        if (ch == '%') {
            if (!validatePercent(path, i)) return false;
            i += 2;
        } else if (ch == '/' || isPchar(ch)) {
            continue;
        } else {
            return false;
        }
    }
    return true;
}

static bool isValidQueryOrFragment(const string& part) {
    for (size_t i = 0; i < part.size(); ++i) {
        char ch = part[i];
        if (ch == '%') {
            if (!validatePercent(part, i)) return false;
            i += 2;
        } else if (isPchar(ch) || ch=='/' || ch=='?') {
            continue;
        } else {
            return false;
        }
    }
    return true;
}

static bool isValidIPv4(const string& s) {
    int count = 0;
    size_t start = 0;
    while (true) {
        size_t dot = s.find('.', start);
        string part = (dot == string::npos) ? s.substr(start) : s.substr(start, dot - start);
        if (part.empty() || part.size() > 3) return false;
        if (!isAllDigits(part)) return false;
        int val = stoi(part);
        if (val < 0 || val > 255) return false;
        ++count;
        if (dot == string::npos) break;
        start = dot + 1;
        if (start > s.size()) return false;
    }
    return count == 4;
}

static bool isValidDomainLabel(const string& label) {
    if (label.empty() || label.size() > 63) return false;
    if (!isAlphaNum(label.front()) || !isAlphaNum(label.back())) return false;
    for (char ch : label) {
        if (!(isAlphaNum(ch) || ch == '-')) return false;
    }
    return true;
}

static bool isValidHost(const string& host) {
    if (host.empty()) return false;
    string lower = host;
    for (char& c : lower) c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
    if (lower == "localhost") return true;
    if (isValidIPv4(host)) return true;
    if (host.size() > 253) return false;

    vector<string> labels;
    size_t start = 0;
    while (true) {
        size_t dot = host.find('.', start);
        string label = (dot == string::npos) ? host.substr(start) : host.substr(start, dot - start);
        if (!isValidDomainLabel(label)) return false;
        labels.push_back(label);
        if (dot == string::npos) break;
        start = dot + 1;
        if (start > host.size()) return false;
    }
    if (labels.size() < 2) return false;
    string tld = labels.back();
    bool alphaTld = true;
    for (char ch : tld) {
        if (!((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || ch=='-')) { alphaTld = false; break; }
    }
    if (!alphaTld) {
        string tldLower = tld;
        for (char& c : tldLower) c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
        if (tldLower.rfind("xn--", 0) != 0) return false;
    }
    if (tld.size() < 2) return false;
    return true;
}

static bool validateIPv6Parts(const vector<string>& parts) {
    for (const string& part : parts) {
        if (part.empty() || part.size() > 4) return false;
        for (char ch : part) if (!isHex(ch)) return false;
    }
    return true;
}

// Simplified IPv6 (no IPv4-mapped)
static bool isValidIPv6(const string& s) {
    if (s.empty()) return false;
    int colonCount = 0;
    for (char ch : s) if (ch == ':') ++colonCount;
    if (colonCount < 2) return false;

    size_t ddc = s.find("::");
    bool hasDD = ddc != string::npos;
    if (hasDD && s.find("::", ddc + 2) != string::npos) return false;

    string left = hasDD ? s.substr(0, ddc) : s;
    string right = hasDD ? s.substr(ddc + 2) : "";

    vector<string> leftParts;
    if (!left.empty()) {
        size_t start = 0;
        while (true) {
            size_t pos = left.find(':', start);
            string seg = (pos == string::npos) ? left.substr(start) : left.substr(start, pos - start);
            leftParts.push_back(seg);
            if (pos == string::npos) break;
            start = pos + 1;
        }
    }
    vector<string> rightParts;
    if (!right.empty()) {
        size_t start = 0;
        while (true) {
            size_t pos = right.find(':', start);
            string seg = (pos == string::npos) ? right.substr(start) : right.substr(start, pos - start);
            rightParts.push_back(seg);
            if (pos == string::npos) break;
            start = pos + 1;
        }
    }

    if (!validateIPv6Parts(leftParts)) return false;
    if (!validateIPv6Parts(rightParts)) return false;

    int total = static_cast<int>(leftParts.size() + rightParts.size());
    if (hasDD) return total < 8;
    return total == 8;
}

bool isValidURL(const string& s) {
    if (s.empty() || s.size() > MAX_URL_LENGTH) return false;
    for (char c : s) {
        unsigned char uc = static_cast<unsigned char>(c);
        if (uc < 0x21 || uc > 0x7E) return false;
    }

    size_t schemeSep = s.find("://");
    if (schemeSep == string::npos || schemeSep == 0) return false;
    string scheme = s.substr(0, schemeSep);
    for (char& c : scheme) c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
    if (!(scheme == "http" || scheme == "https" || scheme == "ftp")) return false;

    size_t pos = schemeSep + 3;
    int authorityEnd = indexOfFirst(s, static_cast<int>(pos), {'/', '?', '#'});
    if (authorityEnd == -1) authorityEnd = static_cast<int>(s.size());
    if (authorityEnd <= static_cast<int>(pos)) return false;
    string authority = s.substr(pos, static_cast<size_t>(authorityEnd) - pos);
    if (authority.find('@') != string::npos) return false;

    int port = -1;
    string host;

    if (!authority.empty() && authority[0] == '[') {
        size_t close = authority.find(']');
        if (close == string::npos || close <= 1) return false;
        host = authority.substr(1, close - 1);
        if (!isValidIPv6(host)) return false;
        string rest = authority.substr(close + 1);
        if (!rest.empty()) {
            if (rest[0] != ':') return false;
            string portStr = rest.substr(1);
            if (portStr.empty() || portStr.size() > 5 || !isAllDigits(portStr)) return false;
            int val = stoi(portStr);
            if (val < 1 || val > 65535) return false;
            port = val;
        }
    } else {
        size_t colon = authority.rfind(':');
        if (colon != string::npos) {
            host = authority.substr(0, colon);
            string portStr = authority.substr(colon + 1);
            if (portStr.empty() || portStr.size() > 5 || !isAllDigits(portStr)) return false;
            int val = stoi(portStr);
            if (val < 1 || val > 65535) return false;
            port = val;
        } else {
            host = authority;
        }
        if (!isValidHost(host)) return false;
    }

    int idx = authorityEnd;
    int qPos = static_cast<int>(s.find('?', static_cast<size_t>(idx)));
    int fPos = static_cast<int>(s.find('#', static_cast<size_t>(idx)));
    int pathEnd = static_cast<int>(s.size());
    if (qPos != -1) pathEnd = min(pathEnd, qPos);
    if (fPos != -1) pathEnd = min(pathEnd, fPos);
    string path = s.substr(static_cast<size_t>(idx), static_cast<size_t>(pathEnd - idx));
    if (!isValidPath(path)) return false;

    if (qPos != -1) {
        int queryEnd = (fPos != -1) ? fPos : static_cast<int>(s.size());
        string query = s.substr(static_cast<size_t>(qPos + 1), static_cast<size_t>(queryEnd - (qPos + 1)));
        if (!isValidQueryOrFragment(query)) return false;
    }
    if (fPos != -1) {
        string frag = s.substr(static_cast<size_t>(fPos + 1));
        if (!isValidQueryOrFragment(frag)) return false;
    }

    return true;
}

int main() {
    vector<string> tests = {
        "https://example.com",
        "http://sub.example.co.uk:8080/path?query=ok#frag",
        "ftp://[2001:db8::1]/",
        "http://256.256.256.256",
        "https://example.com:70000"
    };
    for (const auto& t : tests) {
        cout << t << " -> " << (isValidURL(t) ? "true" : "false") << "\n";
    }
    return 0;
}
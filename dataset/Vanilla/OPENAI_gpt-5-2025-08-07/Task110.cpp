#include <bits/stdc++.h>
using namespace std;

static bool isHex(char c) {
    return (c >= '0' && c <= '9') ||
           (c >= 'a' && c <= 'f') ||
           (c >= 'A' && c <= 'F');
}

static bool isAlnum(char c) {
    return (c >= '0' && c <= '9') ||
           (c >= 'A' && c <= 'Z') ||
           (c >= 'a' && c <= 'z');
}

static bool isValidPort(const string& s) {
    if (s.empty() || s.size() > 5) return false;
    for (char c : s) if (!isdigit((unsigned char)c)) return false;
    int p = stoi(s);
    return p >= 1 && p <= 65535;
}

static bool isValidIPv4(const string& s) {
    int dots = 0;
    for (char c : s) if (c == '.') dots++;
    if (dots != 3) return false;
    stringstream ss(s);
    string part;
    int cnt = 0;
    while (getline(ss, part, '.')) {
        cnt++;
        if (part.empty() || part.size() > 3) return false;
        for (char c : part) if (!isdigit((unsigned char)c)) return false;
        if (part.size() > 1 && part[0] == '0') return false;
        int v = stoi(part);
        if (v < 0 || v > 255) return false;
    }
    return cnt == 4;
}

static bool isValidDomain(const string& host) {
    if (host.empty() || host.size() > 253) return false;
    if (host.front() == '.' || host.back() == '.') return false;
    string label;
    stringstream ss(host);
    while (getline(ss, label, '.')) {
        if (label.size() < 1 || label.size() > 63) return false;
        if (!isAlnum(label.front()) || !isAlnum(label.back())) return false;
        for (char c : label) {
            if (!(isAlnum(c) || c == '-')) return false;
        }
    }
    return true;
}

static bool isValidIPv6(const string& s) {
    if (s.empty()) return false;
    string t = s;
    int maxHextets = 8;

    if (t.find('.') != string::npos) {
        size_t lastColon = t.rfind(':');
        if (lastColon == string::npos) return false;
        string ipv4 = t.substr(lastColon + 1);
        if (!isValidIPv4(ipv4)) return false;
        t = t.substr(0, lastColon);
        maxHextets = 6;
    }

    vector<string> dc;
    size_t pos = 0;
    size_t idx = t.find("::");
    if (idx == string::npos) {
        dc.push_back(t);
    } else {
        // ensure only one "::"
        if (t.find("::", idx + 2) != string::npos) return false;
        dc.push_back(t.substr(0, idx));
        dc.push_back(t.substr(idx + 2));
    }

    int hextetCount = 0;
    for (size_t i = 0; i < dc.size(); ++i) {
        const string& side = dc[i];
        if (side.empty()) continue;
        stringstream ss(side);
        string h;
        while (getline(ss, h, ':')) {
            if (h.size() < 1 || h.size() > 4) return false;
            for (char c : h) if (!isHex(c)) return false;
            hextetCount++;
        }
    }

    if (dc.size() == 2) {
        return hextetCount < maxHextets;
    } else {
        return hextetCount == maxHextets;
    }
}

bool validateURL(const string& url) {
    if (url.empty()) return false;
    for (char c : url) {
        if (isspace((unsigned char)c)) return false;
    }

    string lower = url;
    for (char& c : lower) c = (char)tolower((unsigned char)c);

    size_t schemeEnd = lower.find("://");
    if (schemeEnd == string::npos || schemeEnd == 0) return false;
    string scheme = lower.substr(0, schemeEnd);
    if (!(scheme == "http" || scheme == "https" || scheme == "ftp")) return false;

    size_t pos = schemeEnd + 3;
    size_t n = url.size();
    if (pos >= n) return false;

    string host;
    int port = -1;

    if (url[pos] == '[') {
        size_t close = url.find(']', pos + 1);
        if (close == string::npos) return false;
        host = url.substr(pos + 1, close - (pos + 1));
        if (!isValidIPv6(host)) return false;
        pos = close + 1;
        if (pos < n && url[pos] == ':') {
            size_t start = ++pos;
            while (pos < n && isdigit((unsigned char)url[pos])) pos++;
            if (start == pos) return false;
            string ps = url.substr(start, pos - start);
            if (!isValidPort(ps)) return false;
            port = stoi(ps);
        }
    } else {
        size_t start = pos;
        while (pos < n) {
            char c = url[pos];
            if (c == ':' || c == '/' || c == '?' || c == '#') break;
            pos++;
        }
        if (pos == start) return false;
        host = url.substr(start, pos - start);
        if (host.find(':') != string::npos) return false;
        if (!(isValidIPv4(host) || isValidDomain(host) || strcasecmp(host.c_str(), "localhost") == 0)) return false;

        if (pos < n && url[pos] == ':') {
            size_t startp = ++pos;
            while (pos < n && isdigit((unsigned char)url[pos])) pos++;
            if (startp == pos) return false;
            string ps = url.substr(startp, pos - startp);
            if (!isValidPort(ps)) return false;
            port = stoi(ps);
        }
    }

    // Remainder must be non-whitespace (already checked)
    return true;
}

int main() {
    vector<string> tests = {
        "https://example.com",
        "http://sub.domain.co.uk:8080/path?query=1#frag",
        "ftp://192.168.1.1/resource",
        "https://[2001:db8::1]/index.html",
        "http://-invalid-.com"
    };
    for (const auto& t : tests) {
        cout << t << " -> " << (validateURL(t) ? "true" : "false") << "\n";
    }
    return 0;
}
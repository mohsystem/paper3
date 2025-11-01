#include <bits/stdc++.h>
using namespace std;

static bool contains_ctl(const string& s) {
    for (unsigned char c : s) {
        if ((c < 0x20) || (c == 0x7F)) return true;
    }
    return false;
}

static string to_lower(string s) {
    for (char& c : s) c = (char)tolower((unsigned char)c);
    return s;
}

string redirect_url(const string& url) {
    if (url.empty()) return "404 Not Found";
    // Trim
    auto l = url.find_first_not_of(" \t\r\n");
    auto r = url.find_last_not_of(" \t\r\n");
    if (l == string::npos) return "404 Not Found";
    string u = url.substr(l, r - l + 1);
    if (contains_ctl(u)) return "404 Not Found";

    if (u.find("://") == string::npos) {
        u = string("http://") + u;
    }

    size_t scheme_pos = u.find("://");
    if (scheme_pos == string::npos || scheme_pos == 0) return "404 Not Found";
    string scheme = to_lower(u.substr(0, scheme_pos));
    if (scheme != "http" && scheme != "https") return "404 Not Found";

    size_t auth_start = scheme_pos + 3;
    size_t auth_end = u.find_first_of("/?#", auth_start);
    if (auth_end == string::npos) auth_end = u.size();
    if (auth_end <= auth_start) return "404 Not Found";
    string authority = u.substr(auth_start, auth_end - auth_start);

    // Remove userinfo (last '@')
    size_t at = authority.rfind('@');
    string hostport = (at == string::npos) ? authority : authority.substr(at + 1);

    if (hostport.empty()) return "404 Not Found";

    string host;
    if (!hostport.empty() && hostport[0] == '[') {
        // IPv6 literal [::1] - won't match example.com anyway
        size_t rbr = hostport.find(']');
        if (rbr == string::npos) return "404 Not Found";
        host = hostport.substr(1, rbr - 1);
    } else {
        size_t colon = hostport.find(':');
        host = (colon == string::npos) ? hostport : hostport.substr(0, colon);
    }

    if (host.empty()) return "404 Not Found";
    string lhost = to_lower(host);

    if (lhost == "example.com") {
        return string("Redirected to: ") + u;
    }
    return "404 Not Found";
}

int main() {
    vector<string> tests = {
        "http://example.com/path",
        "https://EXAMPLE.com",
        "http://sub.example.com",
        "http://example.com@evil.com",
        "example.com"
    };
    for (const auto& t : tests) {
        cout << redirect_url(t) << "\n";
    }
    return 0;
}
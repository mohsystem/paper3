#include <iostream>
#include <string>
#include <unordered_map>
#include <cctype>
using namespace std;

static int hexVal(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

static string urlDecode(const string& s) {
    string out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i) {
        char c = s[i];
        if (c == '+') {
            out.push_back(' ');
        } else if (c == '%' && i + 2 < s.size()) {
            int hi = hexVal(s[i+1]);
            int lo = hexVal(s[i+2]);
            if (hi >= 0 && lo >= 0) {
                out.push_back(static_cast<char>((hi << 4) + lo));
                i += 2;
            } else {
                out.push_back(c);
            }
        } else {
            out.push_back(c);
        }
    }
    return out;
}

static pair<string,string> splitPathAndQuery(const string& url) {
    size_t qmark = url.find('?');
    string query = (qmark != string::npos) ? url.substr(qmark + 1) : "";
    size_t scheme = url.find("://");
    string path;
    if (scheme != string::npos) {
        size_t slash = url.find('/', scheme + 3);
        if (slash != string::npos) {
            path = url.substr(slash, (qmark != string::npos ? qmark - slash : string::npos));
        } else {
            path = "/";
        }
    } else {
        path = (qmark != string::npos) ? url.substr(0, qmark) : url;
    }
    return {path, query};
}

string check_mod(const string& url) {
    const string CORRECT_USER = "admin";
    const string CORRECT_PASS = "secret";
    auto pq = splitPathAndQuery(url);
    const string& path = pq.first;
    const string& query = pq.second;
    if (path != "/check_mod") return "false";

    string username, password;
    size_t start = 0;
    while (start <= query.size()) {
        size_t amp = query.find('&', start);
        string pair = query.substr(start, (amp == string::npos ? string::npos : amp - start));
        if (!pair.empty()) {
            size_t eq = pair.find('=');
            string key = urlDecode(pair.substr(0, eq == string::npos ? pair.size() : eq));
            string val = (eq == string::npos) ? "" : urlDecode(pair.substr(eq + 1));
            if (key == "username") username = val;
            else if (key == "password") password = val;
        }
        if (amp == string::npos) break;
        start = amp + 1;
    }
    if (username == CORRECT_USER && password == CORRECT_PASS) return "true";
    return "false";
}

int main() {
    string tests[] = {
        "/check_mod?username=admin&password=secret",
        "/check_mod?username=admin&password=wrong",
        "/other?username=admin&password=secret",
        "http://example.com/check_mod?username=admin&password=sec%72et",
        "/check_mod?password=secret&username=admin"
    };
    for (const auto& t : tests) {
        cout << check_mod(t) << endl;
    }
    return 0;
}
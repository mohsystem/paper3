#include <bits/stdc++.h>
using namespace std;

class Lse149 {
private:
    static inline vector<pair<string,string>> DB;
    static constexpr size_t MAX_USER_LEN = 50;
    static constexpr size_t MAX_MSG_LEN = 500;

    static string sanitize(const string& s, size_t maxLen) {
        string out;
        out.reserve(min(maxLen, s.size()));
        for (char c : s) {
            if (out.size() >= maxLen) break;
            unsigned char uc = static_cast<unsigned char>(c);
            if (uc >= 32 && uc != 127) out.push_back(c);
        }
        // trim
        size_t start = 0, end = out.size();
        while (start < end && isspace(static_cast<unsigned char>(out[start]))) start++;
        while (end > start && isspace(static_cast<unsigned char>(out[end-1]))) end--;
        return out.substr(start, end - start);
    }

    static string htmlEscape(const string& s) {
        string out;
        out.reserve(s.size());
        for (char c : s) {
            switch (c) {
                case '&': out += "&amp;"; break;
                case '<': out += "&lt;"; break;
                case '>': out += "&gt;"; break;
                case '"': out += "&quot;"; break;
                case '\'': out += "&#39;"; break;
                default: out.push_back(c);
            }
        }
        return out;
    }

public:
    static bool insert_user_message_in_db(const string& username, const string& message) {
        string user = sanitize(username, MAX_USER_LEN);
        string msg = sanitize(message, MAX_MSG_LEN);
        if (user.empty() || msg.empty()) return false;
        DB.emplace_back(user, msg);
        return true;
    }

    static string routePost(const string& username, const string& message) {
        insert_user_message_in_db(username, message);
        return "REDIRECT: /";
    }

    static string routeRoot() {
        string html = "<!doctype html><html><head><meta charset=\"utf-8\"><title>Messages</title></head><body>";
        html += "<h1>Messages</h1><ul>";
        for (auto& [u, m] : DB) {
            html += "<li><strong>" + htmlEscape(u) + "</strong>: " + htmlEscape(m) + "</li>";
        }
        html += "</ul></body></html>";
        return html;
    }
};

int main() {
    // 5 test cases
    cout << Lse149::routePost("alice", "Hello, world!") << "\n";
    cout << Lse149::routePost("bob", "Testing <b>bold</b> & \"quotes\"") << "\n";
    cout << Lse149::routePost("charlie", "Line1\nLine2 should be sanitized for control chars.") << "\n";
    cout << Lse149::routePost("dave", string("This is a very long message that will be trimmed if too long.") * 20) << "\n";
    cout << Lse149::routePost(" eve ", "   spaced input   ") << "\n";

    cout << Lse149::routeRoot() << "\n";
    return 0;
}
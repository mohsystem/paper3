#include <bits/stdc++.h>
using namespace std;

/*
 Chain-of-Through (comments):
 1) Implement post(): extracts from request, inserts, redirects, display messages.
 2) Security: validate inputs, escape HTML, strip control chars, bounds.
 3) Secure coding: avoid globals exposure, copy on render, no undefined behavior.
 4) Review: no sensitive data leakage, consistent error handling.
 5) Final: secure functions.
*/

struct Message {
    string userEsc;
    string msgEsc;
    long long ts;
};

static vector<Message> DB;

static bool is_valid_username(const string& u) {
    if (u.empty() || u.size() > 32) return false;
    for (unsigned char c : u) {
        if (!(isalnum(c) || c == '_' || c == '-' || c == '.' || c == ' ')) return false;
    }
    return true;
}

static bool is_valid_message(const string& m) {
    return !m.empty() && m.size() <= 1000;
}

static string strip_controls(const string& s) {
    string out;
    out.reserve(s.size());
    for (unsigned char c : s) {
        if (c == '\n' || c == '\r' || c == '\t' || c >= 32) out.push_back((char)c);
    }
    return out;
}

static string escape_html(const string& s) {
    string out;
    out.reserve((size_t)(s.size() * 1.2));
    for (char c : s) {
        switch (c) {
            case '&': out += "&amp;"; break;
            case '<': out += "&lt;"; break;
            case '>': out += "&gt;"; break;
            case '"': out += "&quot;"; break;
            case '\'': out += "&#39;"; break;
            default: out.push_back(c); break;
        }
    }
    return out;
}

bool insert_user_message_in_db(const string& usernameRaw, const string& messageRaw) {
    string u = usernameRaw;
    string m = messageRaw;
    auto ltrim = [](string& s){
        s.erase(s.begin(), find_if(s.begin(), s.end(), [](unsigned char ch){ return !isspace(ch); }));
    };
    auto rtrim = [](string& s){
        s.erase(find_if(s.rbegin(), s.rend(), [](unsigned char ch){ return !isspace(ch); }).base(), s.end());
    };
    ltrim(u); rtrim(u);
    ltrim(m); rtrim(m);

    if (!is_valid_username(u)) return false;
    if (!is_valid_message(m)) return false;

    string userEsc = escape_html(u);
    string msgEsc = escape_html(strip_controls(m));

    Message msg{userEsc, msgEsc, (long long)time(nullptr)};

    DB.push_back(std::move(msg));
    return true;
}

string post(const unordered_map<string,string>& request) {
    string u, m;
    if (auto it = request.find("username"); it != request.end()) u = it->second; else u = "";
    if (auto it = request.find("message"); it != request.end()) m = it->second; else m = "";
    bool ok = insert_user_message_in_db(u, m);
    return ok ? "REDIRECT: /" : "REDIRECT: /?error=invalid";
}

string display_messages() {
    ostringstream oss;
    oss << "<!DOCTYPE html><html><head><meta charset=\"utf-8\"><title>Messages</title></head><body>";
    oss << "<h1>Messages</h1><ul>";
    for (const auto& msg : DB) {
        // Format time
        time_t tt = (time_t)msg.ts;
        tm tmv{};
        #if defined(_WIN32)
            localtime_s(&tmv, &tt);
        #else
            localtime_r(&tt, &tmv);
        #endif
        char buf[32];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tmv);
        oss << "<li><strong>" << msg.userEsc << "</strong> [" << buf << "]: " << msg.msgEsc << "</li>";
    }
    oss << "</ul></body></html>";
    return oss.str();
}

int main() {
    // 5 test cases
    cout << post({{"username","Alice"},{"message","Hello world!"}}) << "\n";
    cout << post({{"username","Bob_123"},{"message","<script>alert(1)</script> & stuff"}}) << "\n";
    cout << post({{"username","Bad#Name"},{"message","should fail"}}) << "\n";
    cout << post({{"username","Carol"},{"message","Line1\nLine2\tTabbed"}}) << "\n";
    cout << post({{"username","Dave"},{"message", string(1005, 'A')}}) << "\n";
    cout << display_messages() << "\n";
    return 0;
}
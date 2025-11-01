#include <bits/stdc++.h>
using namespace std;

// Simple session structure
struct Session {
    string username; // empty if not authenticated
    string csrf;
};

// In-memory session store
static unordered_map<string, Session> sessions;

// Demo credentials (constant-time compared)
static const string DEMO_USER = "admin";
static const string DEMO_PASS = "s3cureP@ss!";

static bool safe_equals(const string& a, const string& b) {
    if (a.size() != b.size()) return false;
    unsigned char diff = 0;
    for (size_t i = 0; i < a.size(); ++i) {
        diff |= static_cast<unsigned char>(a[i]) ^ static_cast<unsigned char>(b[i]);
    }
    return diff == 0;
}

static string b64url_encode(const vector<unsigned char>& data) {
    static const char* tbl = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
    string out;
    int val = 0, valb = -6;
    for (unsigned char c : data) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            out.push_back(tbl[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) out.push_back(tbl[((val << 8) >> (valb + 8)) & 0x3F]);
    // no padding for urlsafe
    return out;
}

static string gen_token(size_t bytesLen = 24) {
    vector<unsigned char> buf(bytesLen);
    // Use std::random_device for entropy
    std::random_device rd;
    for (size_t i = 0; i < bytesLen; ++i) {
        buf[i] = static_cast<unsigned char>(rd());
    }
    return b64url_encode(buf);
}

static string escape_html(const string& s) {
    string out;
    out.reserve(s.size());
    for (char c : s) {
        switch (c) {
            case '&': out += "&amp;"; break;
            case '<': out += "&lt;"; break;
            case '>': out += "&gt;"; break;
            case '"': out += "&quot;"; break;
            case '\'': out += "&#x27;"; break;
            default: out += c; break;
        }
    }
    return out;
}

string index_page() {
    return "<html><body><h1>Welcome</h1><a href=\"/login\">Login</a></body></html>";
}

string init_session() {
    string sid = gen_token(24);
    Session s;
    s.username.clear();
    s.csrf = gen_token(24);
    sessions[sid] = s;
    return sid;
}

string login_page(const string& session_id) {
    auto it = sessions.find(session_id);
    if (it == sessions.end()) return "ERROR: Invalid session";
    string csrf = it->second.csrf;
    return string("<html><body><h1>Login</h1>"
    "<form method=\"POST\" action=\"/do_login\">") +
    "<input type=\"hidden\" name=\"csrf\" value=\"" + escape_html(csrf) + "\"/>" +
    "Username: <input type=\"text\" name=\"username\"/><br/>"
    "Password: <input type=\"password\" name=\"password\"/><br/>"
    "<button type=\"submit\">Login</button>"
    "</form></body></html>";
}

string do_login(const string& session_id, const string& username, const string& password, const string& csrf_submitted) {
    auto it = sessions.find(session_id);
    if (it == sessions.end()) return "ERROR: Invalid session";
    if (!safe_equals(csrf_submitted, it->second.csrf)) return "ERROR: CSRF validation failed";
    if (username.empty() || password.empty()) return "ERROR: Missing credentials";
    bool okUser = safe_equals(username, DEMO_USER);
    bool okPass = safe_equals(password, DEMO_PASS);
    if (okUser && okPass) {
        it->second.username = username;
        it->second.csrf = gen_token(24); // rotate CSRF
        return "REDIRECT:/secret";
    }
    return "ERROR: Invalid username or password";
}

string secret_page(const string& session_id) {
    auto it = sessions.find(session_id);
    if (it == sessions.end()) return "ERROR: Invalid session";
    if (it->second.username.empty()) return "REDIRECT:/login";
    return "<html><body><h1>Secret settings</h1></body></html>";
}

int main() {
    // Test 1: Index
    cout << "TEST1 Index:\n" << index_page() << "\n";

    // New session
    string sid = init_session();

    // Test 2: Login page
    cout << "\nTEST2 Login Page:\n" << login_page(sid) << "\n";
    string csrf = sessions[sid].csrf;

    // Test 3: Wrong password
    cout << "\nTEST3 Wrong Password:\n" << do_login(sid, "admin", "wrong", csrf) << "\n";

    // Test 4: Wrong CSRF
    cout << "\nTEST4 Wrong CSRF:\n" << do_login(sid, "admin", "s3cureP@ss!", "badtoken") << "\n";

    // Test 5: Success and secret
    cout << "\nTEST5 Successful Login and Secret:\n" << do_login(sid, "admin", "s3cureP@ss!", csrf) << "\n";
    cout << secret_page(sid) << "\n";
    return 0;
}
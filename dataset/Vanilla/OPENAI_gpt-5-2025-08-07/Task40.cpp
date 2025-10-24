#include <bits/stdc++.h>
using namespace std;

struct Session {
    string sid;
    string csrf;
    string username = "default_user";
    string email = "user@example.com";
};

static unordered_map<string, Session> sessions;

static string rand_hex(size_t nbytes) {
    static random_device rd;
    static mt19937_64 gen(rd());
    uniform_int_distribution<unsigned int> dist(0, 255);
    string out;
    out.reserve(nbytes * 2);
    const char* hex = "0123456789abcdef";
    for (size_t i = 0; i < nbytes; ++i) {
        unsigned int b = dist(gen);
        out.push_back(hex[(b >> 4) & 0xF]);
        out.push_back(hex[b & 0xF]);
    }
    return out;
}

static string parse_sid_from_cookie(const string& cookie) {
    if (cookie.empty()) return "";
    stringstream ss(cookie);
    string part;
    while (getline(ss, part, ';')) {
        // trim
        auto s = part.find_first_not_of(' ');
        if (s == string::npos) continue;
        auto token = part.substr(s);
        if (token.rfind("SID=", 0) == 0) {
            return token.substr(4);
        }
    }
    return "";
}

static Session& get_or_create_session(const string& sid) {
    if (!sid.empty()) {
        auto it = sessions.find(sid);
        if (it != sessions.end()) return it->second;
    }
    Session s;
    s.sid = rand_hex(16);
    sessions[s.sid] = s;
    return sessions[s.sid];
}

string process_request(const string& method, const string& path, const string& cookieHeader,
                       const string& form_csrf, const string& form_username, const string& form_email) {
    try {
        if (path != "/settings") {
            return string("HTTP/1.1 404 Not Found\nContent-Type: text/plain; charset=utf-8\n\nNot Found");
        }
        string sid = parse_sid_from_cookie(cookieHeader);
        Session& session = get_or_create_session(sid);

        if (method == "GET") {
            session.csrf = rand_hex(16);
            string body;
            body += "<!doctype html><html><head><title>Settings</title></head><body>";
            body += "<h1>User Settings</h1>";
            body += "<form method=\"POST\" action=\"/settings\">";
            body += "<input type=\"hidden\" name=\"csrf_token\" value=\"" + session.csrf + "\"/>";
            body += "<label>Username: <input type=\"text\" name=\"username\" value=\"" + session.username + "\"/></label><br/>";
            body += "<label>Email: <input type=\"email\" name=\"email\" value=\"" + session.email + "\"/></label><br/>";
            body += "<button type=\"submit\">Update</button>";
            body += "</form></body></html>";

            string resp;
            resp += "HTTP/1.1 200 OK\n";
            resp += "Set-Cookie: SID=" + session.sid + "; HttpOnly; SameSite=Strict\n";
            resp += "Content-Type: text/html; charset=utf-8\n\n";
            resp += body;
            return resp;
        } else if (method == "POST") {
            if (form_csrf == session.csrf && !form_csrf.empty()) {
                Session& s = session;
                if (!form_username.empty()) s.username = form_username;
                if (!form_email.empty()) s.email = form_email;
                s.csrf = rand_hex(16); // rotate
                string body = "Settings updated successfully for " + s.username + " (" + s.email + ")";
                string resp;
                resp += "HTTP/1.1 200 OK\n";
                resp += "Set-Cookie: SID=" + s.sid + "; HttpOnly; SameSite=Strict\n";
                resp += "Content-Type: text/plain; charset=utf-8\n\n";
                resp += body;
                return resp;
            } else {
                string resp;
                resp += "HTTP/1.1 403 Forbidden\n";
                resp += "Set-Cookie: SID=" + session.sid + "; HttpOnly; SameSite=Strict\n";
                resp += "Content-Type: text/plain; charset=utf-8\n\n";
                resp += "Forbidden: invalid CSRF token";
                return resp;
            }
        } else {
            return string("HTTP/1.1 405 Method Not Allowed\nAllow: GET, POST\nContent-Type: text/plain; charset=utf-8\n\nMethod Not Allowed");
        }
    } catch (...) {
        return string("HTTP/1.1 500 Internal Server Error\nContent-Type: text/plain; charset=utf-8\n\nInternal Server Error");
    }
}

static string extract_set_cookie_sid(const string& response) {
    string marker = "Set-Cookie: SID=";
    auto i = response.find(marker);
    if (i == string::npos) return "";
    size_t start = i + marker.size();
    auto end = response.find('\n', start);
    string line = end == string::npos ? response.substr(start) : response.substr(start, end - start);
    auto semi = line.find(';');
    if (semi != string::npos) line = line.substr(0, semi);
    // trim
    while (!line.empty() && isspace(line.back())) line.pop_back();
    while (!line.empty() && isspace(line.front())) line.erase(line.begin());
    return line;
}

static string extract_csrf_token(const string& response) {
    string name = "name=\"csrf_token\"";
    auto n = response.find(name);
    if (n == string::npos) return "";
    string vm = "value=\"";
    auto v = response.find(vm, n);
    if (v == string::npos) return "";
    size_t start = v + vm.size();
    auto end = response.find('"', start);
    if (end == string::npos) return "";
    return response.substr(start, end - start);
}

int main() {
    // Test 1: GET initial
    string r1 = process_request("GET", "/settings", "", "", "", "");
    cout << "=== Test 1: GET /settings (no cookie) ===\n" << r1 << "\n";

    string sid = extract_set_cookie_sid(r1);
    string cookie = "SID=" + sid;
    string csrf1 = extract_csrf_token(r1);

    // Test 2: POST with valid CSRF
    string r2 = process_request("POST", "/settings", cookie, csrf1, "alice", "alice@example.com");
    cout << "=== Test 2: POST /settings (valid CSRF) ===\n" << r2 << "\n";

    // Test 3: POST missing CSRF
    string r3 = process_request("POST", "/settings", cookie, "", "mallory", "mallory@example.com");
    cout << "=== Test 3: POST /settings (missing CSRF) ===\n" << r3 << "\n";

    // Test 4: POST invalid CSRF
    string r4 = process_request("POST", "/settings", cookie, "badtoken", "bob", "bob@example.com");
    cout << "=== Test 4: POST /settings (invalid CSRF) ===\n" << r4 << "\n";

    // Test 5: GET new token then POST with new token
    string r5a = process_request("GET", "/settings", cookie, "", "", "");
    string csrfNew = extract_csrf_token(r5a);
    string r5 = process_request("POST", "/settings", cookie, csrfNew, "charlie", "charlie@example.com");
    cout << "=== Test 5: POST /settings after fresh GET (valid CSRF) ===\n" << r5 << "\n";

    return 0;
}
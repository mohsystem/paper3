#include <bits/stdc++.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>

using namespace std;

// info() called when the route is accessed; accepts session map and returns HTML page
string escape_html(const string& s) {
    string out;
    out.reserve(s.size() * 2);
    for (char c : s) {
        switch (c) {
            case '&': out += "&amp;"; break;
            case '<': out += "&lt;"; break;
            case '>': out += "&gt;"; break;
            case '"': out += "&quot;"; break;
            case '\'': out += "&#39;"; break;
            default: out += c; break;
        }
    }
    return out;
}

string last4_of_ssn(const string& ssn) {
    string digits;
    for (char c : ssn) if (isdigit(static_cast<unsigned char>(c))) digits += c;
    if (digits.empty()) return "N/A";
    if (digits.size() <= 4) return digits;
    return digits.substr(digits.size() - 4);
}

string info(const unordered_map<string, string>& session) {
    string username = "guest";
    string email = "guest@example.com";
    string ssn = "";
    if (session.count("username")) username = session.at("username");
    if (session.count("email")) email = session.at("email");
    if (session.count("ssn")) ssn = session.at("ssn");
    string last4 = last4_of_ssn(ssn);

    string html;
    html += "<!DOCTYPE html><html><head><title>Info</title></head><body>";
    html += "<h1>User Info</h1>";
    html += "<p>Username: " + escape_html(username) + "</p>";
    html += "<p>Email: " + escape_html(email) + "</p>";
    html += "<p>SSN (last 4): " + escape_html(last4) + "</p>";
    html += "</body></html>";
    return html;
}

// Minimal HTTP server with /info route and simple cookie-based session
struct SessionStore {
    unordered_map<string, unordered_map<string,string>> data;
    string cookieName = "SID";

    string parse_sid_from_headers(const string& headers) {
        // Find "Cookie:" header lines, then locate SID=
        string sid;
        stringstream ss(headers);
        string line;
        while (getline(ss, line)) {
            if (line.size() >= 7 && strncasecmp(line.c_str(), "Cookie:", 7) == 0) {
                size_t pos = line.find(':');
                string val = (pos == string::npos) ? "" : line.substr(pos + 1);
                // split by ';'
                string token;
                stringstream sc(val);
                while (getline(sc, token, ';')) {
                    // trim
                    size_t start = token.find_first_not_of(" \t\r\n");
                    size_t end = token.find_last_not_of(" \t\r\n");
                    if (start == string::npos) continue;
                    string kv = token.substr(start, end - start + 1);
                    size_t eq = kv.find('=');
                    if (eq != string::npos) {
                        string k = kv.substr(0, eq);
                        string v = kv.substr(eq + 1);
                        // trim k
                        size_t ks = k.find_first_not_of(" \t");
                        size_t ke = k.find_last_not_of(" \t");
                        if (ks == string::npos) continue;
                        k = k.substr(ks, ke - ks + 1);
                        if (k == cookieName) {
                            // trim v
                            size_t vs = v.find_first_not_of(" \t");
                            size_t ve = v.find_last_not_of(" \t\r\n");
                            if (vs != string::npos)
                                sid = v.substr(vs, ve - vs + 1);
                            return sid;
                        }
                    }
                }
            }
        }
        return "";
    }

    string generate_sid() {
        // Not cryptographically secure; sufficient for demo
        static atomic<uint64_t> counter{0};
        uint64_t c = ++counter;
        auto now = chrono::steady_clock::now().time_since_epoch().count();
        stringstream ss;
        ss << hex << now << "-" << c;
        return ss.str();
    }

    pair<string, unordered_map<string,string>&> get_or_create(const string& headers, bool& isNew) {
        string sid = parse_sid_from_headers(headers);
        if (sid.empty() || !data.count(sid)) {
            sid = generate_sid();
            isNew = true;
            data[sid] = unordered_map<string,string>();
        } else {
            isNew = false;
        }
        auto& sess = data[sid];
        if (!sess.count("username")) sess["username"] = "guest";
        if (!sess.count("email")) sess["email"] = "guest@example.com";
        if (!sess.count("ssn")) sess["ssn"] = "0000";
        return {sid, sess};
    }
};

static void run_server(uint16_t port) {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) return;
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);
    if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        close(server_fd);
        return;
    }
    if (listen(server_fd, 16) < 0) {
        close(server_fd);
        return;
    }

    SessionStore store;

    while (true) {
        int client = accept(server_fd, nullptr, nullptr);
        if (client < 0) {
            continue;
        }
        // Read request (simple, not robust)
        string req;
        char buf[4096];
        ssize_t n;
        // Read until blank line
        while ((n = recv(client, buf, sizeof(buf), 0)) > 0) {
            req.append(buf, buf + n);
            if (req.find("\r\n\r\n") != string::npos) break;
            if (req.size() > 65536) break;
        }
        // Parse first line
        size_t line_end = req.find("\r\n");
        string first_line = (line_end == string::npos) ? req : req.substr(0, line_end);
        string method, path, httpver;
        {
            stringstream fl(first_line);
            fl >> method >> path >> httpver;
        }

        // Extract headers substring
        size_t headers_start = line_end == string::npos ? string::npos : line_end + 2;
        size_t headers_end = req.find("\r\n\r\n");
        string headers_str = (headers_start != string::npos && headers_end != string::npos && headers_end >= headers_start)
                             ? req.substr(headers_start, headers_end - headers_start)
                             : "";

        auto send_text = [&](int code, const string& status, const string& body, const vector<string>& extraHeaders = {}) {
            stringstream resp;
            resp << "HTTP/1.1 " << code << " " << status << "\r\n";
            resp << "Content-Type: text/html; charset=UTF-8\r\n";
            resp << "Content-Length: " << body.size() << "\r\n";
            for (auto& h : extraHeaders) resp << h << "\r\n";
            resp << "Connection: close\r\n\r\n";
            string r = resp.str();
            send(client, r.c_str(), r.size(), 0);
            if (!body.empty()) send(client, body.c_str(), body.size(), 0);
        };

        if (method != "GET") {
            send_text(405, "Method Not Allowed", "Method Not Allowed");
            close(client);
            continue;
        }
        if (path != "/info") {
            send_text(404, "Not Found", "Not Found");
            close(client);
            continue;
        }

        bool isNew = false;
        auto sid_sess = store.get_or_create(headers_str, isNew);
        string sid = sid_sess.first;
        auto& sess = sid_sess.second;
        string body = info(sess);

        vector<string> extra;
        if (isNew) {
            extra.push_back("Set-Cookie: " + store.cookieName + "=" + sid + "; Path=/; HttpOnly");
        }
        send_text(200, "OK", body, extra);

        close(client);
    }
    // close(server_fd); // unreachable
}

int main() {
    // 5 test cases calling info() directly with session-like maps
    vector<unordered_map<string,string>> tests;
    tests.push_back({{"username","alice"},{"email","alice@example.com"},{"ssn","123-45-6789"}});
    tests.push_back({{"username","bob"},{"email","bob@x.com"},{"ssn","0000"}});
    tests.push_back({{"username","carol"},{"email","carol@x.com"},{"ssn","987654321"}});
    tests.push_back({{"username","dave"},{"email","dave@x.com"},{"ssn",""}});
    tests.push_back({{"username","eve"},{"email","eve@x.com"},{"ssn","12-34"}});

    for (size_t i = 0; i < tests.size(); ++i) {
        string out = info(tests[i]);
        cout << "TEST " << (i + 1) << " OUTPUT:\n" << out << "\n-----\n";
    }

    // Start server in background thread (optional)
    // thread srv([](){ run_server(8082); });
    // srv.join();
    return 0;
}
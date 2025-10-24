#include <bits/stdc++.h>
using namespace std;

static const size_t MAX_INPUT_LEN = 1024;
static const size_t MAX_BODY_LEN = 4096;
static const size_t MAX_HTML_LEN = 65536;

string html_escape(const string &in) {
    // Replace &, <, >, ", ' to prevent HTML injection
    string out;
    out.reserve(in.size() + in.size() / 8);
    for (unsigned char ch : in) {
        switch (ch) {
            case '&': out += "&amp;"; break;
            case '<': out += "&lt;"; break;
            case '>': out += "&gt;"; break;
            case '"': out += "&quot;"; break;
            case '\'': out += "&#39;"; break;
            default: out.push_back(static_cast<char>(ch)); break;
        }
        if (out.size() > MAX_HTML_LEN) {
            // Truncate safely if someone tries to blow up output size
            out.resize(MAX_HTML_LEN);
            break;
        }
    }
    return out;
}

bool is_ascii_printable_or_space(unsigned char c) {
    // Allow TAB(9), LF(10), CR(13), and visible ASCII 32..126
    return c == 9 || c == 10 || c == 13 || (c >= 32 && c <= 126);
}

bool validate_input(const string &s) {
    if (s.size() > MAX_INPUT_LEN) return false;
    for (unsigned char c : s) {
        if (!is_ascii_printable_or_space(c)) return false;
    }
    return true;
}

static inline int hexval(unsigned char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
    if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
    return -1;
}

pair<bool, string> url_decode(const string &in) {
    string out;
    out.reserve(in.size());
    for (size_t i = 0; i < in.size(); ++i) {
        unsigned char c = static_cast<unsigned char>(in[i]);
        if (c == '+') {
            out.push_back(' ');
        } else if (c == '%') {
            if (i + 2 >= in.size()) return {false, ""};
            int hi = hexval(static_cast<unsigned char>(in[i + 1]));
            int lo = hexval(static_cast<unsigned char>(in[i + 2]));
            if (hi < 0 || lo < 0) return {false, ""};
            out.push_back(static_cast<char>((hi << 4) | lo));
            i += 2;
        } else {
            out.push_back(static_cast<char>(c));
        }
        if (out.size() > MAX_INPUT_LEN) {
            // Limit decoded message to MAX_INPUT_LEN
            return {false, ""};
        }
    }
    return {true, out};
}

string trim(const string &s) {
    size_t b = 0, e = s.size();
    while (b < e && isspace(static_cast<unsigned char>(s[b]))) ++b;
    while (e > b && isspace(static_cast<unsigned char>(s[e - 1]))) --e;
    return s.substr(b, e - b);
}

string lowercase(const string &s) {
    string t(s);
    for (char &c : t) c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
    return t;
}

bool content_type_is_form_urlencoded(const string &contentType) {
    string ct = contentType;
    size_t sc = ct.find(';');
    if (sc != string::npos) ct = ct.substr(0, sc);
    ct = lowercase(trim(ct));
    return ct == "application/x-www-form-urlencoded";
}

optional<string> get_form_field_message(const string &body) {
    // Parse application/x-www-form-urlencoded and return "message" param if present
    size_t start = 0;
    size_t pairCount = 0;
    while (start <= body.size()) {
        if (++pairCount > 200) break; // prevent excessive pairs
        size_t amp = body.find('&', start);
        if (amp == string::npos) amp = body.size();
        string pair = body.substr(start, amp - start);
        size_t eq = pair.find('=');
        string keyEnc = eq == string::npos ? pair : pair.substr(0, eq);
        string valEnc = eq == string::npos ? "" : pair.substr(eq + 1);
        auto kd = url_decode(keyEnc);
        auto vd = url_decode(valEnc);
        if (kd.first && vd.first) {
            if (kd.second == "message") {
                return vd.second;
            }
        } else {
            // invalid encoding in this pair; skip it
        }
        if (amp == body.size()) break;
        start = amp + 1;
    }
    return nullopt;
}

string build_html_page(const optional<string> &maybeMessage, const optional<string> &maybeError) {
    // Build minimal, safe HTML response
    string title = "Echo Form";
    string body;
    body.reserve(2048);
    body += "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\">";
    body += "<meta http-equiv=\"X-Content-Type-Options\" content=\"nosniff\">";
    body += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
    body += "<title>";
    body += title;
    body += "</title>";
    body += "<style>body{font-family:sans-serif;margin:2rem;} .box{padding:1rem;border:1px solid #ccc;border-radius:8px;max-width:640px;} .err{color:#b00020;} .msg{background:#f6f8fa;padding:.75rem;border-radius:6px;white-space:pre-wrap;}</style>";
    body += "</head><body><div class=\"box\">";
    body += "<h1>Echo your message</h1>";
    if (maybeError.has_value()) {
        body += "<p class=\"err\">Error: ";
        body += html_escape(*maybeError);
        body += "</p>";
    }
    body += "<form method=\"POST\" action=\"\">";
    body += "<label for=\"message\">Enter text (max 1024 ASCII chars):</label><br>";
    body += "<textarea id=\"message\" name=\"message\" rows=\"5\" cols=\"60\" maxlength=\"1024\" required></textarea><br>";
    body += "<button type=\"submit\">Submit</button>";
    body += "</form>";
    if (maybeMessage.has_value()) {
        body += "<h2>Echoed message</h2>";
        body += "<div class=\"msg\">";
        body += html_escape(*maybeMessage);
        body += "</div>";
    }
    body += "</div></body></html>";
    if (body.size() > MAX_HTML_LEN) {
        body.resize(MAX_HTML_LEN);
    }
    return body;
}

string handle_request(const string &method, const string &contentType, const string &bodyRaw) {
    // Dispatch based on method and content type; return full HTML
    if (lowercase(method) != "post") {
        return build_html_page(nullopt, nullopt);
    }
    if (!content_type_is_form_urlencoded(contentType)) {
        return build_html_page(nullopt, optional<string>("Unsupported Content-Type. Use application/x-www-form-urlencoded."));
    }
    if (bodyRaw.size() > MAX_BODY_LEN) {
        return build_html_page(nullopt, optional<string>("Request body too large."));
    }
    optional<string> message = get_form_field_message(bodyRaw);
    if (!message.has_value()) {
        return build_html_page(nullopt, optional<string>("Missing 'message' field."));
    }
    if (!validate_input(*message)) {
        return build_html_page(nullopt, optional<string>("Invalid input. Use ASCII printable characters up to 1024 length."));
    }
    return build_html_page(message, nullopt);
}

// --- CGI entry ---
string read_stdin_body(size_t contentLength) {
    string body;
    if (contentLength > MAX_BODY_LEN) contentLength = MAX_BODY_LEN;
    body.resize(contentLength, '\0');
    cin.read(&body[0], static_cast<streamsize>(contentLength));
    streamsize got = cin.gcount();
    body.resize(static_cast<size_t>(got));
    return body;
}

bool is_cgi_environment() {
    const char* rm = getenv("REQUEST_METHOD");
    const char* gi = getenv("GATEWAY_INTERFACE");
    return (rm != nullptr) || (gi != nullptr);
}

// --- Test harness ---
void run_tests() {
    struct Test {
        string name;
        string method;
        string ctype;
        string body;
    };
    vector<Test> tests = {
        {"GET shows form", "GET", "", ""},
        {"POST ok simple", "POST", "application/x-www-form-urlencoded", "message=Hello+World"},
        {"POST escapes HTML", "POST", "application/x-www-form-urlencoded", "message=%3Cb%3Ebold%3C%2Fb%3E+%26+%22q%22"},
        {"POST too long", "POST", "application/x-www-form-urlencoded", string("message=") + string(1100, 'A')},
        {"POST invalid encoding", "POST", "application/x-www-form-urlencoded", "message=Hello%ZZWorld"}
    };
    for (const auto &t : tests) {
        string html = handle_request(t.method, t.ctype, t.body);
        cout << "==== " << t.name << " ====\n";
        cout << "HTML length: " << html.size() << "\n";
        // Print a small snippet to verify
        size_t pos = html.find("<div class=\"msg\">");
        if (pos != string::npos) {
            size_t end = html.find("</div>", pos);
            string snippet = html.substr(pos, (end != string::npos ? end - pos : 120));
            cout << "Snippet: " << snippet.substr(0, 200) << "\n";
        } else {
            // Maybe an error paragraph
            size_t epos = html.find("<p class=\"err\">");
            if (epos != string::npos) {
                size_t eend = html.find("</p>", epos);
                string snippet = html.substr(epos, (eend != string::npos ? eend - epos : 120));
                cout << "Snippet: " << snippet.substr(0, 200) << "\n";
            } else {
                cout << "Snippet: " << html.substr(0, 200) << "\n";
            }
        }
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (is_cgi_environment()) {
        string method = getenv("REQUEST_METHOD") ? getenv("REQUEST_METHOD") : "";
        string ctype = getenv("CONTENT_TYPE") ? getenv("CONTENT_TYPE") : "";
        string clenStr = getenv("CONTENT_LENGTH") ? getenv("CONTENT_LENGTH") : "";
        size_t clen = 0;
        if (!clenStr.empty()) {
            // Safe parse
            for (char ch : clenStr) {
                if (!isdigit(static_cast<unsigned char>(ch))) { clen = 0; clenStr.clear(); break; }
            }
            if (!clenStr.empty()) {
                try {
                    size_t v = stoul(clenStr);
                    clen = min(v, MAX_BODY_LEN);
                } catch (...) {
                    clen = 0;
                }
            }
        }
        string body = read_stdin_body(clen);
        string html = handle_request(method, ctype, body);
        cout << "Content-Type: text/html; charset=UTF-8\r\n";
        cout << "X-Content-Type-Options: nosniff\r\n";
        cout << "Content-Length: " << html.size() << "\r\n\r\n";
        cout << html;
        return 0;
    } else {
        // Run tests in CLI mode
        run_tests();
        return 0;
    }
}
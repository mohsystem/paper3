// Chain-of-Through Steps (high-level, non-sensitive):
// 1) Provide main page with form and a post() handler.
// 2) Validate inputs, limit sizes, escape HTML when rendering.
// 3) Implement safe data handling with strict checks.
// 4) Review string concatenations; ensure output encoding.
// 5) Final secure code with tests.

#include <iostream>
#include <vector>
#include <string>
#include <regex>
#include <sstream>
#include <cctype>

static std::vector<std::pair<std::string, std::string>> MESSAGES;
static const size_t MAX_MESSAGES = 1000;
static const size_t MAX_MESSAGE_LEN = 500;
static const std::regex USERNAME_RE("^[A-Za-z0-9_\\-]{1,32}$");

std::string post(const std::string& username, const std::string& message);
bool insert_user_message_in_db(const std::string& username, const std::string& message);
std::string main_page();
static std::string sanitize_username(const std::string& username);
static std::string sanitize_message(const std::string& message);
static std::string html_escape(const std::string& s);

std::string post(const std::string& username, const std::string& message) {
    bool ok = insert_user_message_in_db(username, message);
    return ok ? "REDIRECT: /" : "REDIRECT: /?error=invalid";
}

bool insert_user_message_in_db(const std::string& username, const std::string& message) {
    std::string u = sanitize_username(username);
    std::string m = sanitize_message(message);
    if (u.empty() || m.empty()) return false;
    if (MESSAGES.size() >= MAX_MESSAGES) return false;
    MESSAGES.emplace_back(u, m);
    return true;
}

std::string main_page() {
    std::ostringstream out;
    out << "<!doctype html><html><head><meta charset=\"utf-8\"><title>Messages</title></head><body>";
    out << "<h1>Post a message</h1>";
    out << "<form method=\"POST\" action=\"/post\">";
    out << "<label>Username: <input type=\"text\" name=\"username\" maxlength=\"32\" required></label><br>";
    out << "<label>Message: <textarea name=\"message\" maxlength=\"" << MAX_MESSAGE_LEN << "\" required></textarea></label><br>";
    out << "<button type=\"submit\">Send</button>";
    out << "</form>";
    out << "<h2>Recent Messages</h2><ul>";
    for (const auto& pair : MESSAGES) {
        out << "<li><strong>" << html_escape(pair.first) << "</strong>: " << html_escape(pair.second) << "</li>";
    }
    out << "</ul></body></html>";
    return out.str();
}

static std::string sanitize_username(const std::string& username) {
    std::string u;
    // trim
    size_t start = username.find_first_not_of(" \t\r\n");
    size_t end = username.find_last_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    u = username.substr(start, end - start + 1);
    if (u.size() > 32) return "";
    if (!std::regex_match(u, USERNAME_RE)) return "";
    return u;
}

static std::string sanitize_message(const std::string& message) {
    std::string m;
    m.reserve(message.size());
    for (unsigned char ch : message) {
        if (std::isprint(ch) || ch == '\n' || ch == '\t') m.push_back(static_cast<char>(ch));
        else m.push_back(' ');
    }
    // trim
    size_t start = m.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = m.find_last_not_of(" \t\r\n");
    m = m.substr(start, end - start + 1);
    if (m.empty()) return "";
    if (m.size() > MAX_MESSAGE_LEN) m.resize(MAX_MESSAGE_LEN);
    return m;
}

static std::string html_escape(const std::string& s) {
    std::string out;
    out.reserve(s.size() * 2);
    for (char c : s) {
        switch (c) {
            case '&': out += "&amp;"; break;
            case '<': out += "&lt;"; break;
            case '>': out += "&gt;"; break;
            case '"': out += "&quot;"; break;
            case '\'': out += "&#x27;"; break;
            case '/': out += "&#x2F;"; break;
            default: out += c; break;
        }
    }
    return out;
}

int main() {
    // 5 Test cases
    std::cout << "=== Test 1: Initial main page ===\n";
    std::string p1 = main_page();
    std::cout << p1.substr(0, std::min<size_t>(200, p1.size())) << "...\n";

    std::cout << "=== Test 2: Valid post ===\n";
    std::cout << post("Alice", "Hello from C++!") << "\n";

    std::cout << "=== Test 3: XSS attempt ===\n";
    std::cout << post("Bob", "<script>alert(1)</script>") << "\n";

    std::cout << "=== Test 4: Invalid username ===\n";
    std::cout << post("bad user!", "should fail") << "\n";

    std::cout << "=== Test 5: Main page after posts ===\n";
    std::string p2 = main_page();
    std::cout << p2.substr(0, std::min<size_t>(400, p2.size())) << "...\n";
    return 0;
}
#include <iostream>
#include <vector>
#include <string>
#include <regex>
#include <ctime>

struct Message {
    std::string username;
    std::string message;
    std::time_t ts;
};

static std::vector<Message> DB;
static const size_t MAX_MESSAGE_LEN = 500;
static const size_t MAX_USERNAME_LEN = 32;
static const std::regex USERNAME_RE("^[A-Za-z0-9_.-]{1,32}$");

static std::string htmlEscape(const std::string& s) {
    std::string out;
    out.reserve(s.size() + 16);
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

static std::string sanitizeUsername(const std::string& username) {
    std::string u = username;
    // trim
    size_t start = u.find_first_not_of(" \t\r\n");
    size_t end = u.find_last_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    u = u.substr(start, end - start + 1);
    if (u.empty() || u.size() > MAX_USERNAME_LEN) return "";
    if (!std::regex_match(u, USERNAME_RE)) return "";
    return u;
}

static std::string sanitizeMessage(const std::string& message) {
    std::string m = message;
    // normalize newlines
    std::string norm; norm.reserve(m.size());
    for (size_t i = 0; i < m.size(); ++i) {
        char c = m[i];
        if (c == '\r') {
            if (i + 1 < m.size() && m[i+1] == '\n') { /* CRLF -> handled by adding LF once */ }
            norm.push_back('\n');
        } else {
            norm.push_back(c);
        }
    }
    // trim
    size_t start = norm.find_first_not_of(" \t\r\n");
    size_t end = norm.find_last_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    std::string t = norm.substr(start, end - start + 1);
    if (t.empty()) return "";
    if (t.size() > MAX_MESSAGE_LEN) t.resize(MAX_MESSAGE_LEN);
    // remove control chars except 
 and \t
    std::string clean; clean.reserve(t.size());
    for (char c : t) {
        if (c == '\n' || c == '\t' || (static_cast<unsigned char>(c) >= 0x20 && c != 0x7F)) {
            clean.push_back(c);
        }
    }
    return clean;
}

static void insert_user_message_in_db(const std::string& username, const std::string& message) {
    Message m{username, message, std::time(nullptr)};
    DB.push_back(m);
}

std::string post(const std::string& username, const std::string& message) {
    std::string u = sanitizeUsername(username);
    std::string m = sanitizeMessage(message);
    if (!u.empty() && !m.empty()) {
        insert_user_message_in_db(u, m);
    }
    return "redirect:/";
}

std::string displayMessages() {
    std::string html;
    html += "<html><body>\n<h2>Messages</h2>\n<ul>\n";
    for (const auto& msg : DB) {
        html += "<li><strong>";
        html += htmlEscape(msg.username);
        html += ":</strong> ";
        html += htmlEscape(msg.message);
        html += "</li>\n";
    }
    html += "</ul>\n</body></html>";
    return html;
}

std::string renderMainPage() {
    return displayMessages();
}

int main() {
    // 5 test cases
    std::cout << post("alice", "Hello, world!") << "\n";
    std::cout << post("bob_42", "This is a test message.") << "\n";
    std::cout << post("charlie", "<script>alert('xss');</script> Stay safe!") << "\n";
    std::cout << post("invalid name", "Should be rejected due to invalid username.") << "\n";
    std::cout << post("david", "") << "\n"; // empty message rejected

    std::cout << renderMainPage() << "\n";
    return 0;
}
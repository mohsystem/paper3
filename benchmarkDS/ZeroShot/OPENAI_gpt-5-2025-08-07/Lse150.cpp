#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <sstream>
#include <iomanip>
#include <cctype>
#include <algorithm>

static std::vector<std::string> MESSAGES_DB;

std::string htmlEscape(const std::string& s) {
    std::string out;
    out.reserve(s.size());
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

bool validateUsername(const std::string& username) {
    std::string u = username;
    // trim
    auto l = u.find_first_not_of(" \t\r\n");
    auto r = u.find_last_not_of(" \t\r\n");
    if (l == std::string::npos) u.clear();
    else u = u.substr(l, r - l + 1);

    if (u.size() < 3 || u.size() > 32) return false;
    for (char c : u) {
        if (!(std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '-')) return false;
    }
    return true;
}

std::string trim(const std::string& s) {
    auto l = s.find_first_not_of(" \t\r\n");
    auto r = s.find_last_not_of(" \t\r\n");
    if (l == std::string::npos) return "";
    return s.substr(l, r - l + 1);
}

std::string sanitizeMessage(const std::string& message, bool& ok) {
    ok = false;
    std::string m = trim(message);
    std::string out;
    out.reserve(std::min<size_t>(m.size(), 500));
    for (char c : m) {
        unsigned char uc = static_cast<unsigned char>(c);
        if (c == '\n' || (uc >= 0x20 && uc != 0x7F)) {
            out.push_back(c);
            if (out.size() > 500) {
                return "";
            }
        }
    }
    if (out.empty()) return "";
    ok = true;
    return out;
}

bool constantTimeEquals(const std::string& a, const std::string& b) {
    if (a.size() != b.size()) return false;
    unsigned char diff = 0;
    for (size_t i = 0; i < a.size(); ++i) {
        diff |= static_cast<unsigned char>(a[i]) ^ static_cast<unsigned char>(b[i]);
    }
    return diff == 0;
}

std::string generateCsrfToken() {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<unsigned int> dist(0, 255);
    std::ostringstream oss;
    for (int i = 0; i < 32; ++i) {
        unsigned int v = dist(gen);
        oss << std::hex << std::setw(2) << std::setfill('0') << (v & 0xff);
    }
    return oss.str();
}

bool insert_user_message_in_db(const std::string& username, const std::string& message) {
    MESSAGES_DB.push_back(username + ": " + message);
    return true;
}

std::string render_main_page(const std::string& csrfToken) {
    std::string safeToken = htmlEscape(csrfToken);
    std::ostringstream html;
    html << "<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><title>Main</title></head><body>";
    html << "<h1>Submit a message</h1>";
    html << "<form method=\"post\" action=\"/post\" autocomplete=\"off\">";
    html << "<label for=\"username\">Username</label>";
    html << "<input type=\"text\" id=\"username\" name=\"username\" required minlength=\"3\" maxlength=\"32\" pattern=\"[A-Za-z0-9_\\-]+\">";
    html << "<br>";
    html << "<label for=\"message\">Message</label>";
    html << "<textarea id=\"message\" name=\"message\" required maxlength=\"500\"></textarea>";
    html << "<input type=\"hidden\" name=\"csrf\" value=\"" << safeToken << "\">";
    html << "<br><button type=\"submit\">Send</button>";
    html << "</form>";
    html << "</body></html>";
    return html.str();
}

std::string post(const std::string& username, const std::string& message, const std::string& csrfFromReq, const std::string& sessionCsrf) {
    if (!constantTimeEquals(csrfFromReq, sessionCsrf)) {
        return "REDIRECT:/?error=csrf";
    }
    if (!validateUsername(username)) {
        return "REDIRECT:/?error=bad_username";
    }
    bool ok = false;
    std::string sanitized = sanitizeMessage(message, ok);
    if (!ok) {
        return "REDIRECT:/?error=bad_message";
    }
    if (!insert_user_message_in_db(trim(username), sanitized)) {
        return "REDIRECT:/?error=server";
    }
    return "REDIRECT:/";
}

int main() {
    std::string sessionToken = generateCsrfToken();
    std::cout << "Rendered Main Page HTML:\n" << render_main_page(sessionToken) << "\n";

    // Test 1: Valid
    std::cout << "Test1: " << post("alice_01", "Hello world!", sessionToken, sessionToken) << "\n";

    // Test 2: Invalid username
    std::cout << "Test2: " << post("a", "short name", sessionToken, sessionToken) << "\n";

    // Test 3: Script content
    std::cout << "Test3: " << post("bob-2", "<script>alert(1)</script>", sessionToken, sessionToken) << "\n";

    // Test 4: Too long message
    std::string longMsg(600, 'x');
    std::cout << "Test4: " << post("charlie_3", longMsg, sessionToken, sessionToken) << "\n";

    // Test 5: CSRF mismatch
    std::cout << "Test5: " << post("dave_4", "Valid message", "badtoken", sessionToken) << "\n";

    std::cout << "DB size: " << MESSAGES_DB.size() << "\n";
    for (const auto& rec : MESSAGES_DB) {
        std::cout << rec << "\n";
    }
    return 0;
}
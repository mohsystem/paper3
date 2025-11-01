#include <iostream>
#include <string>
#include <unordered_map>
#include <algorithm>

static const std::string EXPECTED_USER = "admin";
static const std::string EXPECTED_PASS = "S3cureP@ss";

std::string sanitize(const std::string& s) {
    std::string out;
    out.reserve(std::min<size_t>(64, s.size()));
    for (char c : s) {
        if (c >= 32 && c <= 126) {
            out.push_back(c);
            if (out.size() >= 64) break;
        }
    }
    // trim
    while (!out.empty() && (out.back() == ' ')) out.pop_back();
    size_t start = 0;
    while (start < out.size() && out[start] == ' ') start++;
    return out.substr(start);
}

bool constantTimeEquals(const std::string& a, const std::string& b) {
    const size_t maxLen = std::max(a.size(), b.size());
    unsigned int diff = static_cast<unsigned int>(a.size() ^ b.size());
    for (size_t i = 0; i < maxLen; ++i) {
        unsigned char ac = i < a.size() ? static_cast<unsigned char>(a[i]) : 0;
        unsigned char bc = i < b.size() ? static_cast<unsigned char>(b[i]) : 0;
        diff |= static_cast<unsigned int>(ac ^ bc);
    }
    return diff == 0;
}

std::string indexPage(std::unordered_map<std::string, std::string>& session) {
    (void)session;
    return "<html><body><h1>Index</h1><a href=\"/login\">Login</a></body></html>";
}

std::string loginPage() {
    return std::string("<html><body><h1>Login</h1>")
        + "<form method=\"POST\" action=\"/do_login\">"
        + "<input type=\"text\" name=\"username\" maxlength=\"64\"/>"
        + "<input type=\"password\" name=\"password\" maxlength=\"64\"/>"
        + "<input type=\"submit\" value=\"Login\"/>"
        + "</form></body></html>";
}

std::string doLogin(const std::string& username, const std::string& password, std::unordered_map<std::string, std::string>& session) {
    std::string u = sanitize(username);
    std::string p = sanitize(password);
    if (u.empty() || p.empty()) {
        return "REDIRECT:/login?error=1";
    }
    bool okUser = constantTimeEquals(u, EXPECTED_USER);
    bool okPass = constantTimeEquals(p, EXPECTED_PASS);
    if (okUser && okPass) {
        session["username"] = u;
        return "REDIRECT:/secret";
    }
    return "REDIRECT:/login?error=1";
}

std::string secretPage(std::unordered_map<std::string, std::string>& session) {
    auto it = session.find("username");
    if (it != session.end() && !it->second.empty()) {
        return "<html><body>Secret settings</body></html>";
    }
    return "REDIRECT:/login";
}

int main() {
    // Test case 1: Index page
    std::unordered_map<std::string, std::string> session1;
    std::cout << "CPP TC1 INDEX: " << indexPage(session1) << std::endl;

    // Test case 2: Secret without login
    std::unordered_map<std::string, std::string> session2;
    std::cout << "CPP TC2 SECRET (no login): " << secretPage(session2) << std::endl;

    // Test case 3: Wrong password
    std::unordered_map<std::string, std::string> session3;
    std::cout << "CPP TC3 LOGIN wrong: " << doLogin("admin", "wrong", session3) << std::endl;
    std::cout << "CPP TC3 SECRET after wrong: " << secretPage(session3) << std::endl;

    // Test case 4: Correct login then secret
    std::unordered_map<std::string, std::string> session4;
    std::cout << "CPP TC4 LOGIN correct: " << doLogin("admin", "S3cureP@ss", session4) << std::endl;
    std::cout << "CPP TC4 SECRET after correct: " << secretPage(session4) << std::endl;

    // Test case 5: Empty credentials
    std::unordered_map<std::string, std::string> session5;
    std::cout << "CPP TC5 LOGIN empty: " << doLogin("", "", session5) << std::endl;
    std::cout << "CPP TC5 SECRET after empty: " << secretPage(session5) << std::endl;

    return 0;
}
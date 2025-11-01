#include <iostream>
#include <string>
#include <cctype>
#include <vector>

class Lse42 {
public:
    struct Session {
        std::string username;
        std::string email;
        std::string ssn;
        bool valid;
    };

    static std::string sanitizeUserOrEmail(const std::string& v, size_t maxLen) {
        std::string out;
        out.reserve(v.size());
        for (char c : v) {
            if (out.size() >= maxLen) break;
            if (std::isalnum(static_cast<unsigned char>(c)) || c=='@' || c=='.' || c=='-' || c=='_' || c=='+' || c==' ') {
                out.push_back(c);
            }
        }
        return out;
    }

    static std::string sanitizeSSN(const std::string& v, size_t maxLen) {
        std::string out;
        out.reserve(v.size());
        for (char c : v) {
            if (out.size() >= maxLen) break;
            if (std::isdigit(static_cast<unsigned char>(c)) || c=='-') {
                out.push_back(c);
            }
        }
        return out;
    }

    // info() function: accepts a session and returns the response string
    static std::string info(const Session* session) {
        if (session == nullptr || !session->valid) {
            return "Unauthorized";
        }
        std::string u = sanitizeUserOrEmail(session->username, 100);
        std::string e = sanitizeUserOrEmail(session->email, 254);
        std::string s = sanitizeSSN(session->ssn, 11);
        return "username=" + u + "; email=" + e + "; ssn=" + s;
    }
};

static std::string maskSSN(const std::string& body) {
    // Simple mask for patterns like 123-45-6789
    std::string out = body;
    for (size_t i = 0; i + 10 <= out.size(); ++i) {
        if (std::isdigit(out[i]) && std::isdigit(out[i+1]) && std::isdigit(out[i+2]) &&
            out[i+3]=='-' &&
            std::isdigit(out[i+4]) && std::isdigit(out[i+5]) &&
            out[i+6]=='-' &&
            std::isdigit(out[i+7]) && std::isdigit(out[i+8]) && std::isdigit(out[i+9]) && std::isdigit(out[i+10])) {
            out[i]='*'; out[i+1]='*'; out[i+2]='*';
            out[i+4]='*'; out[i+5]='*';
            // last 4 kept
        }
    }
    return out;
}

int main() {
    // Five test cases
    Lse42::Session s1{"alice", "alice@example.com", "123-45-6789", true};
    Lse42::Session s2{"bob", "bob.smith+dev@example.org", "000-12-3456", true};
    Lse42::Session s3{"charlie_ze", "charlie.ze@example.net", "999-99-9999", true};
    Lse42::Session s4{"dana-j", "dana-j@example.co", "321-54-9876", true};
    Lse42::Session invalid{"", "", "", false};

    std::vector<const Lse42::Session*> tests = {&s1, &s2, &s3, &s4, &invalid};
    for (size_t i = 0; i < tests.size(); ++i) {
        std::string res = Lse42::info(tests[i]);
        std::cout << "Test " << (i+1) << " -> " << maskSSN(res) << "\n";
    }
    return 0;
}
#include <iostream>
#include <string>
#include <algorithm>
#include <cctype>

struct User {
    std::string name;
    int age;
    std::string email;
};

static inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch){ return !std::isspace(ch); }));
}
static inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch){ return !std::isspace(ch); }).base(), s.end());
}
static inline void trim(std::string &s) { ltrim(s); rtrim(s); }

User deserializeUser(const std::string &input) {
    User user;
    user.age = -1;

    size_t start = 0;
    while (start <= input.size()) {
        size_t end = input.find(';', start);
        std::string token = input.substr(start, (end == std::string::npos) ? std::string::npos : end - start);
        if (!token.empty()) {
            size_t eq = token.find('=');
            if (eq != std::string::npos) {
                std::string key = token.substr(0, eq);
                std::string value = token.substr(eq + 1);
                trim(key);
                trim(value);
                std::transform(key.begin(), key.end(), key.begin(), [](unsigned char c){ return std::tolower(c); });
                if (key == "name") {
                    user.name = value;
                } else if (key == "age") {
                    try {
                        user.age = std::stoi(value);
                    } catch (...) {
                        user.age = -1;
                    }
                } else if (key == "email") {
                    user.email = value;
                } else {
                    // ignore unknown
                }
            }
        }
        if (end == std::string::npos) break;
        start = end + 1;
    }

    return user;
}

int main() {
    std::string tests[5] = {
        "name=Alice; age=30; email=alice@example.com",
        "age=25;name=Bob;email=bob@domain.org",
        "name=Charlie;email=charlie@mail.net",
        "name = Dana ; age = notanumber ; email = dana@x.y",
        "email=eve@ex.com; name=Eve; extra=ignored; age= 42 ;"
    };

    for (const auto &t : tests) {
        User u = deserializeUser(t);
        std::cout << "Input: " << t << "\n";
        std::cout << "Deserialized: User{name='" << u.name << "', age=" << u.age << ", email='" << u.email << "'}\n";
        std::cout << "---\n";
    }
    return 0;
}
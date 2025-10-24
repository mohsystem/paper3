#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>

struct User {
    std::string name;
    std::string email;
    std::string password;
};

static const std::string DB_FILE = "users_cpp.csv";
static const std::string HEADER = "name,email,password";

static std::string sanitize(const std::string& s) {
    std::string out = s;
    for (char& c : out) {
        if (c == '\n' || c == '\r' || c == ',') c = ' ';
    }
    // trim
    while (!out.empty() && (out.front() == ' ' || out.front() == '\t')) out.erase(out.begin());
    while (!out.empty() && (out.back() == ' ' || out.back() == '\t')) out.pop_back();
    return out;
}

static bool file_exists(const std::string& path) {
    std::ifstream f(path);
    return f.good();
}

static void init_db() {
    if (!file_exists(DB_FILE)) {
        std::ofstream out(DB_FILE, std::ios::out);
        out << HEADER << "\n";
        out.close();
    }
}

static bool is_valid_email(const std::string& email) {
    // very basic validation: must contain one '@' and at least one '.' after '@'
    auto at_pos = email.find('@');
    if (at_pos == std::string::npos) return false;
    auto dot_pos = email.find('.', at_pos + 1);
    if (dot_pos == std::string::npos) return false;
    if (at_pos == 0 || dot_pos == email.size() - 1) return false;
    if (email.find(' ') != std::string::npos) return false;
    return true;
}

static bool email_exists(const std::string& email) {
    init_db();
    std::ifstream in(DB_FILE);
    std::string line;
    bool first = true;
    while (std::getline(in, line)) {
        if (first) { first = false; continue; }
        std::stringstream ss(line);
        std::string name, em, pw;
        std::getline(ss, name, ',');
        std::getline(ss, em, ',');
        std::getline(ss, pw, ',');
        if (!em.empty()) {
            std::string emLower = em, emailLower = email;
            std::transform(emLower.begin(), emLower.end(), emLower.begin(), ::tolower);
            std::transform(emailLower.begin(), emailLower.end(), emailLower.begin(), ::tolower);
            if (emLower == emailLower) return true;
        }
    }
    return false;
}

bool registerUser(const std::string& nameIn, const std::string& emailIn, const std::string& passwordIn) {
    init_db();
    std::string name = sanitize(nameIn);
    std::string email = sanitize(emailIn);
    std::string password = sanitize(passwordIn);

    if (name.empty() || email.empty() || password.empty()) return false;
    if (!is_valid_email(email)) return false;
    if (password.size() < 6) return false;
    if (email_exists(email)) return false;

    std::ofstream out(DB_FILE, std::ios::app);
    if (!out.good()) return false;
    out << name << "," << email << "," << password << "\n";
    out.close();
    return true;
}

std::vector<User> getAllUsers() {
    init_db();
    std::vector<User> users;
    std::ifstream in(DB_FILE);
    std::string line;
    bool first = true;
    while (std::getline(in, line)) {
        if (first) { first = false; continue; }
        std::stringstream ss(line);
        std::string name, email, password;
        std::getline(ss, name, ',');
        std::getline(ss, email, ',');
        std::getline(ss, password, ',');
        if (!email.empty()) users.push_back(User{name, email, password});
    }
    return users;
}

int main() {
    std::cout << "Test 1 (valid): " << (registerUser("Alice", "alice@example.com", "pass123") ? "true" : "false") << "\n";
    std::cout << "Test 2 (duplicate email): " << (registerUser("Alicia", "alice@example.com", "differentPass") ? "true" : "false") << "\n";
    std::cout << "Test 3 (invalid email): " << (registerUser("Bob", "bob_at_example.com", "secret12") ? "true" : "false") << "\n";
    std::cout << "Test 4 (short password): " << (registerUser("Charlie", "charlie@example.com", "123") ? "true" : "false") << "\n";
    std::cout << "Test 5 (valid): " << (registerUser("Dana", "dana@example.org", "StrongPass9") ? "true" : "false") << "\n";

    std::cout << "All users:\n";
    for (const auto& u : getAllUsers()) {
        std::cout << "User{name='" << u.name << "', email='" << u.email << "', password='" << u.password << "'}\n";
    }
    return 0;
}
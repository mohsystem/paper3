#include <sodium.h>
#include <iostream>
#include <unordered_map>
#include <string>
#include <cctype>
#include <algorithm>

// Validate allowed username characters and length
bool isValidUsername(const std::string& username) {
    if (username.empty() || username.size() > 64) return false;
    for (char c : username) {
        if (!(std::isalnum(static_cast<unsigned char>(c)) || c=='_' || c=='-' || c=='.' || c=='@')) {
            return false;
        }
    }
    return true;
}

// Simple, strong password policy
bool passwordPolicyValid(const std::string& password, const std::string& username, std::string& errorOut) {
    if (password.size() < 12) { errorOut = "Password too short"; return false; }

    bool hasLower = false, hasUpper = false, hasDigit = false, hasSymbol = false;
    for (char c : password) {
        unsigned char uc = static_cast<unsigned char>(c);
        if (std::islower(uc)) hasLower = true;
        else if (std::isupper(uc)) hasUpper = true;
        else if (std::isdigit(uc)) hasDigit = true;
        else hasSymbol = true;
    }
    if (!hasLower || !hasUpper || !hasDigit || !hasSymbol) {
        errorOut = "Password must include upper, lower, digit, and symbol";
        return false;
    }

    // Avoid containing username (case-insensitive)
    std::string pLower = password;
    std::string uLower = username;
    std::transform(pLower.begin(), pLower.end(), pLower.begin(), ::tolower);
    std::transform(uLower.begin(), uLower.end(), uLower.begin(), ::tolower);
    if (!uLower.empty() && pLower.find(uLower) != std::string::npos) {
        errorOut = "Password must not contain username";
        return false;
    }

    // No 4+ repeated characters
    int run = 1;
    for (size_t i = 1; i < password.size(); ++i) {
        if (password[i] == password[i-1]) {
            run++;
            if (run >= 4) { errorOut = "Too many repeated characters"; return false; }
        } else {
            run = 1;
        }
    }

    errorOut.clear();
    return true;
}

bool createUser(std::unordered_map<std::string, std::string>& db,
                const std::string& username,
                const std::string& password,
                std::string& errorOut) {
    if (!isValidUsername(username)) { errorOut = "Invalid username"; return false; }
    if (db.find(username) != db.end()) { errorOut = "User already exists"; return false; }

    std::string pwdErr;
    if (!passwordPolicyValid(password, username, pwdErr)) {
        errorOut = pwdErr;
        return false;
    }

    char hash[crypto_pwhash_STRBYTES];
    if (crypto_pwhash_str(
            hash,
            password.data(),
            password.size(),
            crypto_pwhash_OPSLIMIT_MODERATE,
            crypto_pwhash_MEMLIMIT_MODERATE) != 0) {
        errorOut = "Out of memory or internal error";
        return false;
    }

    db[username] = std::string(hash);
    // Do not log or store plaintext password
    errorOut.clear();
    return true;
}

bool authenticate(const std::unordered_map<std::string, std::string>& db,
                  const std::string& username,
                  const std::string& password) {
    auto it = db.find(username);
    if (it == db.end()) return false;

    const std::string& stored = it->second;
    if (crypto_pwhash_str_verify(stored.c_str(), password.data(), password.size()) == 0) {
        return true;
    }
    return false;
}

int main() {
    if (sodium_init() < 0) {
        std::cerr << "Failed to initialize sodium\n";
        return 1;
    }

    std::unordered_map<std::string, std::string> db;

    // Test case 1: Create user and authenticate successfully
    {
        std::string err;
        bool created = createUser(db, "alice", "GoodPass!234", err);
        std::cout << "TC1 Create alice success: " << (created ? "true" : "false") << "\n";
        bool authOK = authenticate(db, "alice", "GoodPass!234");
        std::cout << "TC1 Auth alice correct pwd: " << (authOK ? "true" : "false") << "\n";
    }

    // Test case 2: Wrong password
    {
        bool authBad = authenticate(db, "alice", "WrongPass!234");
        std::cout << "TC2 Auth alice wrong pwd: " << (authBad ? "true" : "false") << "\n";
    }

    // Test case 3: Duplicate user
    {
        std::string err;
        bool created = createUser(db, "alice", "AnotherGood!999", err);
        std::cout << "TC3 Create duplicate alice: " << (created ? "true" : "false") << "\n";
    }

    // Test case 4: Weak password policy
    {
        std::string err;
        bool created = createUser(db, "bob", "weakpass", err);
        std::cout << "TC4 Create bob weak password: " << (created ? "true" : "false") << "\n";
    }

    // Test case 5: Another user success
    {
        std::string err;
        bool created = createUser(db, "charlie", "Stronger#Pass123", err);
        std::cout << "TC5 Create charlie success: " << (created ? "true" : "false") << "\n";
        bool authOK = authenticate(db, "charlie", "Stronger#Pass123");
        std::cout << "TC5 Auth charlie correct pwd: " << (authOK ? "true" : "false") << "\n";
    }

    return 0;
}
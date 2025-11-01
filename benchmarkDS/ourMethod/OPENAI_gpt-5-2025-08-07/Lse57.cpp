#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <regex>
#include <optional>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>

struct User {
    std::string email;
    std::vector<unsigned char> salt;
    std::vector<unsigned char> hash;
};

class AuthService {
public:
    AuthService() = default;

    bool registerUser(const std::string& email, const std::string& password) {
        if (!isValidEmail(email) || !isStrongPassword(password)) return false;
        std::string key = toLower(email);
        if (users.count(key)) return false;
        std::vector<unsigned char> salt(SALT_LEN);
        if (RAND_bytes(salt.data(), SALT_LEN) != 1) return false;
        std::vector<unsigned char> h(HASH_LEN);
        if (!pbkdf2(password, salt, h)) return false;
        users.emplace(key, User{email, salt, h});
        return true;
    }

    std::optional<std::string> login(const std::string& email, const std::string& password) {
        if (!isValidEmail(email)) return std::nullopt;
        std::string key = toLower(email);
        auto it = users.find(key);
        if (it == users.end()) return std::nullopt;
        std::vector<unsigned char> cand(HASH_LEN);
        if (!pbkdf2(password, it->second.salt, cand)) return std::nullopt;
        bool ok = CRYPTO_memcmp(cand.data(), it->second.hash.data(), HASH_LEN) == 0;
        cleanse(cand);
        if (!ok) return std::nullopt;
        std::vector<unsigned char> tok(16);
        if (RAND_bytes(tok.data(), (int)tok.size()) != 1) return std::nullopt;
        std::string token = hex(tok);
        sessions[token] = key;
        cleanse(tok);
        return token;
    }

    std::string changeEmail(const std::string& sessionToken, const std::string& oldEmail,
                            const std::string& newEmail, const std::string& confirmPassword) {
        if (sessionToken.empty() || oldEmail.empty() || newEmail.empty() || confirmPassword.empty()) {
            return "Invalid input";
        }
        auto sit = sessions.find(sessionToken);
        if (sit == sessions.end()) {
            return "Not logged in";
        }
        if (!isValidEmail(oldEmail) || !isValidEmail(newEmail)) {
            return "Invalid email";
        }
        if (iequals(oldEmail, newEmail)) {
            return "New email must be different";
        }
        std::string oldKey = toLower(oldEmail);
        if (sit->second != oldKey) {
            return "Session does not match user";
        }
        auto it = users.find(oldKey);
        if (it == users.end()) {
            return "Email not found";
        }
        std::vector<unsigned char> cand(HASH_LEN);
        if (!pbkdf2(confirmPassword, it->second.salt, cand)) return "Invalid credentials";
        bool ok = CRYPTO_memcmp(cand.data(), it->second.hash.data(), HASH_LEN) == 0;
        cleanse(cand);
        if (!ok) return "Invalid credentials";
        std::string newKey = toLower(newEmail);
        if (users.count(newKey)) {
            return "Email already in use";
        }
        User u = it->second;
        users.erase(it);
        u.email = newEmail;
        users.emplace(newKey, std::move(u));
        sessions[sessionToken] = newKey;
        return "Email changed";
    }

private:
    static constexpr int SALT_LEN = 16;
    static constexpr int HASH_LEN = 32;
    static constexpr int PBKDF2_ITERS = 210000;

    std::unordered_map<std::string, User> users;
    std::unordered_map<std::string, std::string> sessions;

    static void cleanse(std::vector<unsigned char>& v) {
        if (!v.empty()) OPENSSL_cleanse(v.data(), v.size());
    }

    static std::string toLower(const std::string& s) {
        std::string r(s);
        for (auto& c : r) c = (char)std::tolower((unsigned char)c);
        return r;
    }

    static bool iequals(const std::string& a, const std::string& b) {
        if (a.size() != b.size()) return false;
        for (size_t i = 0; i < a.size(); ++i) {
            if (std::tolower((unsigned char)a[i]) != std::tolower((unsigned char)b[i])) return false;
        }
        return true;
    }

    static std::string hex(const std::vector<unsigned char>& data) {
        static const char* hexdigits = "0123456789abcdef";
        std::string out;
        out.resize(data.size() * 2);
        for (size_t i = 0; i < data.size(); ++i) {
            out[2 * i] = hexdigits[(data[i] >> 4) & 0xF];
            out[2 * i + 1] = hexdigits[data[i] & 0xF];
        }
        return out;
    }

    static bool isValidEmail(const std::string& email) {
        if (email.size() < 5 || email.size() > 254) return false;
        static const std::regex re(R"(^[A-Za-z0-9._%+-]{1,64}@[A-Za-z0-9.-]{1,190}\.[A-Za-z]{2,24}$)");
        return std::regex_match(email, re);
    }

    static bool isStrongPassword(const std::string& pwd) {
        if (pwd.size() < 10 || pwd.size() > 512) return false;
        bool up = false, lo = false, di = false, sp = false;
        for (char c : pwd) {
            if (std::isupper((unsigned char)c)) up = true;
            else if (std::islower((unsigned char)c)) lo = true;
            else if (std::isdigit((unsigned char)c)) di = true;
            else sp = true;
        }
        return up && lo && di && sp;
    }

    static bool pbkdf2(const std::string& password, const std::vector<unsigned char>& salt, std::vector<unsigned char>& out) {
        if (out.size() != HASH_LEN) out.resize(HASH_LEN);
        int rc = PKCS5_PBKDF2_HMAC(password.c_str(), (int)password.size(),
                                   salt.data(), (int)salt.size(),
                                   PBKDF2_ITERS, EVP_sha256(),
                                   (int)out.size(), out.data());
        return rc == 1;
    }
};

int main() {
    AuthService svc;
    const std::string pass1 = std::getenv("USER1_PASS") ? std::getenv("USER1_PASS") : "Str0ng!Passw0rd";
    const std::string pass2 = std::getenv("USER2_PASS") ? std::getenv("USER2_PASS") : "An0ther#Strong1";
    const std::string pass3 = std::getenv("USER3_PASS") ? std::getenv("USER3_PASS") : "Third#Str0ngP4ss";

    std::cout << "Register user1: " << svc.registerUser("alice@example.com", pass1) << "\n";
    std::cout << "Register user2: " << svc.registerUser("bob@example.com", pass2) << "\n";
    std::cout << "Duplicate user1: " << svc.registerUser("alice@example.com", pass1) << "\n";

    // 1) Success case
    auto t1 = svc.login("alice@example.com", pass1);
    std::cout << "Login user1 ok: " << (t1.has_value()) << "\n";
    std::cout << "Change email success: " << (t1.has_value() ? svc.changeEmail(t1.value(), "alice@example.com", "alice.new@example.com", pass1) : "No token") << "\n";

    // 2) Without login
    std::cout << "Change without login: " << svc.changeEmail("badtoken", "bob@example.com", "bob.new@example.com", pass2) << "\n";

    // 3) Wrong old email vs session
    auto t2 = svc.login("bob@example.com", pass2);
    std::cout << "Wrong old email: " << (t2.has_value() ? svc.changeEmail(t2.value(), "alice.new@example.com", "bob.changed@example.com", pass2) : "No token") << "\n";

    // 4) Wrong confirm password
    std::cout << "Wrong password: " << (t2.has_value() ? svc.changeEmail(t2.value(), "bob@example.com", "robert@example.com", "WrongP@ss1") : "No token") << "\n";

    // 5) New email already used
    std::cout << "Register user3: " << svc.registerUser("carol@example.com", pass3) << "\n";
    std::cout << "Login user3 ok: " << svc.login("carol@example.com", pass3).has_value() << "\n";
    std::cout << "Email already used: " << (t2.has_value() ? svc.changeEmail(t2.value(), "bob@example.com", "alice.new@example.com", pass2) : "No token") << "\n";

    return 0;
}
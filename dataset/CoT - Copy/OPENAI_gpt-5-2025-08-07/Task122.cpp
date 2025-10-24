#include <openssl/evp.h>
#include <openssl/rand.h>
#include <cstring>
#include <string>
#include <unordered_map>
#include <iostream>
#include <vector>
#include <cctype>
#include <sstream>

class SecureUserStoreCPP {
public:
    SecureUserStoreCPP() : iterations(200000), salt_len(16), key_len(32) {}

    bool signup(const std::string& username, const std::string& password) {
        if (!validateUsername(username) || !validatePassword(password)) return false;
        if (users.find(username) != users.end()) return false;

        std::vector<unsigned char> salt(salt_len);
        if (RAND_bytes(salt.data(), salt_len) != 1) return false;

        std::vector<unsigned char> dk(key_len);
        if (!pbkdf2(password, salt, iterations, dk)) return false;

        std::string salt_b64 = b64encode(salt);
        std::string dk_b64 = b64encode(dk);

        std::ostringstream oss;
        oss << "v1$" << iterations << "$" << salt_b64 << "$" << dk_b64;
        users[username] = oss.str();
        return true;
    }

    bool verify(const std::string& username, const std::string& password) {
        auto it = users.find(username);
        if (it == users.end()) return false;

        std::string rec = it->second;
        std::vector<std::string> parts = split(rec, '$');
        if (parts.size() != 4 || parts[0] != "v1") return false;

        int iters = std::stoi(parts[1]);
        std::vector<unsigned char> salt = b64decode(parts[2]);
        std::vector<unsigned char> expected = b64decode(parts[3]);

        std::vector<unsigned char> actual(expected.size());
        if (!pbkdf2(password, salt, iters, actual)) return false;

        return constantTimeEquals(expected, actual);
    }

    std::string getRecord(const std::string& username) {
        auto it = users.find(username);
        if (it == users.end()) return "";
        return it->second;
    }

private:
    std::unordered_map<std::string, std::string> users;
    const int iterations;
    const int salt_len;
    const int key_len;

    static bool validateUsername(const std::string& u) {
        if (u.size() < 3 || u.size() > 32) return false;
        for (unsigned char c : u) {
            if (!(std::isalnum(c) || c == '.' || c == '_' || c == '-')) return false;
        }
        return true;
    }

    static bool validatePassword(const std::string& p) {
        if (p.size() < 10) return false;
        bool hasU=false, hasL=false, hasD=false, hasS=false;
        for (unsigned char c : p) {
            if (std::isspace(c)) return false;
            if (std::isupper(c)) hasU = true;
            else if (std::islower(c)) hasL = true;
            else if (std::isdigit(c)) hasD = true;
            else hasS = true;
        }
        return hasU && hasL && hasD && hasS;
    }

    static bool pbkdf2(const std::string& password, const std::vector<unsigned char>& salt, int iters, std::vector<unsigned char>& out) {
        if (PKCS5_PBKDF2_HMAC(password.c_str(), static_cast<int>(password.size()),
                               salt.data(), static_cast<int>(salt.size()),
                               iters, EVP_sha256(),
                               static_cast<int>(out.size()), out.data()) != 1) {
            return false;
        }
        return true;
    }

    static bool constantTimeEquals(const std::vector<unsigned char>& a, const std::vector<unsigned char>& b) {
        if (a.size() != b.size()) return false;
        unsigned int diff = 0;
        for (size_t i = 0; i < a.size(); ++i) {
            diff |= (a[i] ^ b[i]);
        }
        return diff == 0;
    }

    static std::string b64encode(const std::vector<unsigned char>& data) {
        int out_len = 4 * ((static_cast<int>(data.size()) + 2) / 3);
        std::string out(out_len, '\0');
        int actual = EVP_EncodeBlock(reinterpret_cast<unsigned char*>(&out[0]), data.data(), static_cast<int>(data.size()));
        out.resize(actual);
        return out;
    }

    static std::vector<unsigned char> b64decode(const std::string& s) {
        std::vector<unsigned char> out(3 * (s.size() / 4) + 3);
        int actual = EVP_DecodeBlock(out.data(), reinterpret_cast<const unsigned char*>(s.data()), static_cast<int>(s.size()));
        if (actual < 0) return {};
        // OpenSSL's EVP_DecodeBlock includes padding bytes, adjust length by stripping trailing zeros based on '='
        size_t pad = 0;
        if (!s.empty() && s[s.size()-1] == '=') pad++;
        if (s.size() > 1 && s[s.size()-2] == '=') pad++;
        out.resize(static_cast<size_t>(actual) - pad);
        return out;
    }

    static std::vector<std::string> split(const std::string& s, char delim) {
        std::vector<std::string> parts;
        std::string cur;
        for (char c : s) {
            if (c == delim) {
                parts.push_back(cur);
                cur.clear();
            } else {
                cur.push_back(c);
            }
        }
        parts.push_back(cur);
        return parts;
    }
};

int main() {
    SecureUserStoreCPP store;
    std::cout << "Test1 (valid signup): " << (store.signup("alice", "Str0ngPass!") ? "true" : "false") << "\n";
    std::cout << "Test2 (duplicate username): " << (store.signup("alice", "An0ther#Pass") ? "true" : "false") << "\n";
    std::cout << "Test3 (invalid username): " << (store.signup("bob space", "Str0ngPass!") ? "true" : "false") << "\n";
    std::cout << "Test4 (weak password): " << (store.signup("carol", "weak") ? "true" : "false") << "\n";
    std::cout << "Test5 (another valid signup): " << (store.signup("charlie", "G00d#Password") ? "true" : "false") << "\n";
    std::cout << "Verify alice: " << (store.verify("alice", "Str0ngPass!") ? "true" : "false") << "\n";
    std::cout << "Verify alice wrong: " << (store.verify("alice", "WrongPass!") ? "true" : "false") << "\n";
    return 0;
}
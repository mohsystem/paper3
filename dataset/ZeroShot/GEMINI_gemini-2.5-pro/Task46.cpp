#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <regex>
#include <random>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cstdint>

// --- IMPORTANT SECURITY NOTE ---
// The following SHA-256 implementation is for DEMONSTRATION PURPOSES ONLY
// within a single-file constraint. In a real-world, secure application,
// you MUST use a well-vetted, dedicated cryptography library like OpenSSL,
// libsodium, or a platform-native API. You should also use a key derivation
// function like Argon2, scrypt, or PBKDF2, not a simple salted hash.
// "Don't roll your own crypto."

class SHA256 {
public:
    SHA256() { reset(); }
    void update(const uint8_t* data, size_t length) {
        for (size_t i = 0; i < length; ++i) {
            m_data[m_blocklen++] = data[i];
            if (m_blocklen == 64) {
                transform();
                m_bitlen += 512;
                m_blocklen = 0;
            }
        }
    }
    void update(const std::string& data) { update(reinterpret_cast<const uint8_t*>(data.c_str()), data.length()); }
    std::vector<uint8_t> digest() {
        std::vector<uint8_t> hash(32);
        pad();
        revert(hash.data());
        return hash;
    }
private:
    uint8_t m_data[64];
    uint32_t m_blocklen;
    uint64_t m_bitlen;
    uint32_t m_state[8];
    static const uint32_t K[64];
    void transform() {
        uint32_t w[64], a, b, c, d, e, f, g, h, t1, t2;
        for (int i = 0; i < 16; ++i)
            w[i] = (m_data[4 * i] << 24) | (m_data[4 * i + 1] << 16) | (m_data[4 * i + 2] << 8) | m_data[4 * i + 3];
        for (int i = 16; i < 64; ++i)
            w[i] = SIG1(w[i - 2]) + w[i - 7] + SIG0(w[i - 15]) + w[i - 16];
        a = m_state[0]; b = m_state[1]; c = m_state[2]; d = m_state[3];
        e = m_state[4]; f = m_state[5]; g = m_state[6]; h = m_state[7];
        for (int i = 0; i < 64; ++i) {
            t1 = h + EP1(e) + CH(e, f, g) + K[i] + w[i];
            t2 = EP0(a) + MAJ(a, b, c);
            h = g; g = f; f = e; e = d + t1;
            d = c; c = b; b = a; a = t1 + t2;
        }
        m_state[0] += a; m_state[1] += b; m_state[2] += c; m_state[3] += d;
        m_state[4] += e; m_state[5] += f; m_state[6] += g; m_state[7] += h;
    }
    void pad() {
        uint64_t i = m_blocklen;
        uint8_t end = (m_blocklen < 56) ? (56 - m_blocklen) : (120 - m_blocklen);
        m_data[i++] = 0x80;
        while (i < 64) m_data[i++] = 0;
        if (m_blocklen >= 56) {
            transform();
            for (i = 0; i < 56; ++i) m_data[i] = 0;
        }
        m_bitlen += m_blocklen * 8;
        m_data[63] = m_bitlen;
        m_data[62] = m_bitlen >> 8; m_data[61] = m_bitlen >> 16; m_data[60] = m_bitlen >> 24;
        m_data[59] = m_bitlen >> 32; m_data[58] = m_bitlen >> 40; m_data[57] = m_bitlen >> 48; m_data[56] = m_bitlen >> 56;
        transform();
    }
    void revert(uint8_t* hash) { for (int i = 0; i < 4; ++i) {
            hash[i] = (m_state[0] >> (24 - i * 8)) & 0x000000ff; hash[i + 4] = (m_state[1] >> (24 - i * 8)) & 0x000000ff;
            hash[i + 8] = (m_state[2] >> (24 - i * 8)) & 0x000000ff; hash[i + 12] = (m_state[3] >> (24 - i * 8)) & 0x000000ff;
            hash[i + 16] = (m_state[4] >> (24 - i * 8)) & 0x000000ff; hash[i + 20] = (m_state[5] >> (24 - i * 8)) & 0x000000ff;
            hash[i + 24] = (m_state[6] >> (24 - i * 8)) & 0x000000ff; hash[i + 28] = (m_state[7] >> (24 - i * 8)) & 0x000000ff;
        }
    }
    void reset() {
        m_state[0] = 0x6a09e667; m_state[1] = 0xbb67ae85; m_state[2] = 0x3c6ef372; m_state[3] = 0xa54ff53a;
        m_state[4] = 0x510e527f; m_state[5] = 0x9b05688c; m_state[6] = 0x1f83d9ab; m_state[7] = 0x5be0cd19;
        m_blocklen = 0; m_bitlen = 0;
    }
    uint32_t ROTR(uint32_t x, uint32_t n) { return (x >> n) | (x << (32 - n)); }
    uint32_t CH(uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (~x & z); }
    uint32_t MAJ(uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (x & z) ^ (y & z); }
    uint32_t EP0(uint32_t x) { return ROTR(x, 2) ^ ROTR(x, 13) ^ ROTR(x, 22); }
    uint32_t EP1(uint32_t x) { return ROTR(x, 6) ^ ROTR(x, 11) ^ ROTR(x, 25); }
    uint32_t SIG0(uint32_t x) { return ROTR(x, 7) ^ ROTR(x, 18) ^ (x >> 3); }
    uint32_t SIG1(uint32_t x) { return ROTR(x, 17) ^ ROTR(x, 19) ^ (x >> 10); }
};
const uint32_t SHA256::K[64] = {
	0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
	0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
	0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
	0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
	0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
	0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
	0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
	0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};
// --- END OF SHA-256 IMPLEMENTATION ---

// User data structure
struct User {
    std::string name;
    std::string email;
    std::vector<uint8_t> salt;
    std::string hashedPassword;
};

// In-memory "database"
static std::map<std::string, User> userDatabase;
const size_t SALT_LENGTH_BYTES = 16;

// Hex encoding helper
std::string to_hex(const std::vector<uint8_t>& data) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (uint8_t byte : data) {
        ss << std::setw(2) << static_cast<int>(byte);
    }
    return ss.str();
}

std::vector<uint8_t> generate_salt() {
    std::random_device rd;
    std::vector<uint8_t> salt(SALT_LENGTH_BYTES);
    std::uniform_int_distribution<int> dist(0, 255);
    for (size_t i = 0; i < SALT_LENGTH_BYTES; ++i) {
        salt[i] = static_cast<uint8_t>(dist(rd));
    }
    return salt;
}

std::string hash_password(const std::string& password, const std::vector<uint8_t>& salt) {
    SHA256 sha;
    sha.update(password);
    sha.update(salt.data(), salt.size());
    return to_hex(sha.digest());
}

std::string validateInput(const std::string& name, const std::string& email, const std::string& password) {
    if (name.empty() || name.length() > 50) {
        return "Invalid name. Must be between 1 and 50 characters.";
    }
    const std::regex email_regex("^[a-zA-Z0-9_.+-]+@[a-zA-Z0-9-]+\\.[a-zA-Z0-9-.]+$");
    if (!std::regex_match(email, email_regex)) {
        return "Invalid email format.";
    }
    const std::regex password_regex("^(?=.*[a-z])(?=.*[A-Z])(?=.*\\d).{8,}$");
    if (!std::regex_match(password, password_regex)) {
        return "Password is too weak. Must be at least 8 characters, with one uppercase, one lowercase, and one number.";
    }
    return ""; // Empty string means success
}

std::string registerUser(const std::string& name, const std::string& email, const std::string& password) {
    std::string validationError = validateInput(name, email, password);
    if (!validationError.empty()) {
        return "Registration failed: " + validationError;
    }
    
    std::string normalized_email = email;
    std::transform(normalized_email.begin(), normalized_email.end(), normalized_email.begin(), ::tolower);

    if (userDatabase.count(normalized_email)) {
        return "Registration failed: Email '" + email + "' already exists.";
    }

    auto salt = generate_salt();
    auto hashedPassword = hash_password(password, salt);

    User newUser{name, normalized_email, salt, hashedPassword};
    userDatabase[normalized_email] = newUser;
    
    return "User '" + name + "' registered successfully.";
}

int main() {
    std::cout << "--- User Registration System ---" << std::endl;

    // Test Case 1: Successful registration
    std::cout << "Test 1: " << registerUser("Alice", "alice@example.com", "Password123") << std::endl;

    // Test Case 2: Attempt to register with a duplicate email
    std::cout << "Test 2: " << registerUser("Alice Smith", "alice@example.com", "AnotherPass456") << std::endl;

    // Test Case 3: Registration with an invalid email format
    std::cout << "Test 3: " << registerUser("Bob", "bob-at-example.com", "SecurePass789") << std::endl;

    // Test Case 4: Registration with a weak password
    std::cout << "Test 4: " << registerUser("Charlie", "charlie@example.com", "pass") << std::endl;

    // Test Case 5: Another successful registration
    std::cout << "Test 5: " << registerUser("David", "david@example.com", "MyP@ssw0rd!") << std::endl;

    std::cout << "\n--- Current Database State ---" << std::endl;
    for (const auto& pair : userDatabase) {
        std::cout << "Email: " << pair.first << ", Name: " << pair.second.name
                  << ", Salt: " << to_hex(pair.second.salt)
                  << ", Hash: " << pair.second.hashedPassword << std::endl;
    }

    return 0;
}
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <random>
#include <sstream>
#include <iomanip>
#include <algorithm>

// --- BEGIN Embedded SHA-256 implementation (public domain) ---
// Source: https://github.com/p-m-a/sha256
class SHA256 {
protected:
    typedef unsigned char uint8;
    typedef unsigned int uint32;
    typedef unsigned long long uint64;

    const static uint32 sha256_k[];
    static const unsigned int SHA224_256_BLOCK_SIZE = (512 / 8);
public:
    void init() {
        m_h[0] = 0x6a09e667; m_h[1] = 0xbb67ae85; m_h[2] = 0x3c6ef372;
        m_h[3] = 0xa54ff53a; m_h[4] = 0x510e527f; m_h[5] = 0x9b05688c;
        m_h[6] = 0x1f83d9ab; m_h[7] = 0x5be0cd19;
        m_len = 0; m_tot_len = 0;
    }
    void update(const unsigned char *message, unsigned int len) {
        unsigned int block_nb;
        unsigned int new_len, rem_len, tmp_len;
        const unsigned char *shifted_message;
        tmp_len = SHA224_256_BLOCK_SIZE - m_len;
        rem_len = len < tmp_len ? len : tmp_len;
        memcpy(&m_block[m_len], message, rem_len);
        if (m_len + len < SHA224_256_BLOCK_SIZE) { m_len += len; return; }
        new_len = len - rem_len;
        block_nb = new_len / SHA224_256_BLOCK_SIZE;
        shifted_message = message + rem_len;
        transform(m_block, 1);
        transform(shifted_message, block_nb);
        rem_len = new_len % SHA224_256_BLOCK_SIZE;
        memcpy(m_block, &shifted_message[block_nb << 6], rem_len);
        m_len = rem_len;
        m_tot_len += (block_nb + 1) << 6;
    }
    void final(unsigned char *digest) {
        unsigned int block_nb;
        unsigned int pm_len;
        unsigned int len_b;
        int i;
        block_nb = (1 + ((SHA224_256_BLOCK_SIZE - 9) < (m_len % SHA224_256_BLOCK_SIZE)));
        len_b = (m_tot_len + m_len) << 3;
        pm_len = block_nb << 6;
        memset(m_block + m_len, 0, pm_len - m_len);
        m_block[m_len] = 0x80;
        for (i = 0; i < 4; i++) { m_block[pm_len - 1 - i] = (len_b >> (i * 8)) & 0xFF; }
        transform(m_block, block_nb);
        for (i = 0; i < 8; i++) {
            digest[i * 4] = (m_h[i] >> 24) & 0xFF;
            digest[i * 4 + 1] = (m_h[i] >> 16) & 0xFF;
            digest[i * 4 + 2] = (m_h[i] >> 8) & 0xFF;
            digest[i * 4 + 3] = m_h[i] & 0xFF;
        }
    }
    static std::string hash(const std::string& input) {
        unsigned char digest[32];
        SHA256 ctx;
        ctx.init();
        ctx.update((unsigned char*)input.c_str(), input.length());
        ctx.final(digest);
        std::stringstream ss;
        for(int i = 0; i < 32; ++i)
            ss << std::hex << std::setw(2) << std::setfill('0') << (int)digest[i];
        return ss.str();
    }
private:
    unsigned char m_block[2 * SHA224_256_BLOCK_SIZE];
    uint32 m_h[8];
    uint64 m_len;
    uint64 m_tot_len;
    void transform(const unsigned char *message, unsigned int block_nb);
    static const unsigned int SHA256_DIGEST_SIZE = (256 / 8);
};
const uint32 SHA256::sha256_k[] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};
#define SHFR(x, n)    (x >> n)
#define ROTR(x, n)   ((x >> n) | (x << (32 - n)))
#define CH(x, y, z)  ((x & y) ^ (~x & z))
#define MAJ(x, y, z) ((x & y) ^ (x & z) ^ (y & z))
#define EP0(x) (ROTR(x, 2) ^ ROTR(x, 13) ^ ROTR(x, 22))
#define EP1(x) (ROTR(x, 6) ^ ROTR(x, 11) ^ ROTR(x, 25))
#define SIG0(x) (ROTR(x, 7) ^ ROTR(x, 18) ^ SHFR(x, 3))
#define SIG1(x) (ROTR(x, 17) ^ ROTR(x, 19) ^ SHFR(x, 10))
void SHA256::transform(const unsigned char *message, unsigned int block_nb) {
    uint32 w[64];
    uint32 wv[8];
    uint32 t1, t2;
    const unsigned char *sub_block;
    int i;
    int j;
    for (i = 0; i < (int) block_nb; i++) {
        sub_block = message + (i << 6);
        for (j = 0; j < 16; j++) {
            w[j] = (uint32) sub_block[4 * j + 0] << 24 | (uint32) sub_block[4 * j + 1] << 16 | (uint32) sub_block[4 * j + 2] << 8 | (uint32) sub_block[4 * j + 3];
        }
        for (j = 16; j < 64; j++) { w[j] = SIG1(w[j - 2]) + w[j - 7] + SIG0(w[j - 15]) + w[j - 16]; }
        for (j = 0; j < 8; j++) { wv[j] = m_h[j]; }
        for (j = 0; j < 64; j++) {
            t1 = wv[7] + EP1(wv[4]) + CH(wv[4], wv[5], wv[6]) + sha256_k[j] + w[j];
            t2 = EP0(wv[0]) + MAJ(wv[0], wv[1], wv[2]);
            wv[7] = wv[6]; wv[6] = wv[5]; wv[5] = wv[4]; wv[4] = wv[3] + t1;
            wv[3] = wv[2]; wv[2] = wv[1]; wv[1] = wv[0]; wv[0] = t1 + t2;
        }
        for (j = 0; j < 8; j++) { m_h[j] += wv[j]; }
    }
}
// --- END Embedded SHA-256 implementation ---

const int SALT_LENGTH = 16;

struct User {
    std::string salt;
    std::string hashedPassword;
};

std::map<std::string, User> userDatabase;

std::string generateSalt(int length) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(0, 255);
    std::stringstream ss;
    for (int i = 0; i < length; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << dist(gen);
    }
    return ss.str();
}

bool secure_compare(const std::string& a, const std::string& b) {
    if (a.length() != b.length()) {
        return false;
    }
    int result = 0;
    for (size_t i = 0; i < a.length(); ++i) {
        result |= a[i] ^ b[i];
    }
    return result == 0;
}

bool registerUser(const std::string& username, const std::string& password) {
    if (username.empty() || password.empty()) {
        std::cerr << "Error: Username and password cannot be empty." << std::endl;
        return false;
    }
    if (userDatabase.count(username)) {
        std::cerr << "Error: User '" << username << "' already exists." << std::endl;
        return false;
    }

    std::string salt = generateSalt(SALT_LENGTH);
    std::string hashedPassword = SHA256::hash(salt + password);

    userDatabase[username] = {salt, hashedPassword};
    return true;
}

bool authenticateUser(const std::string& username, const std::string& password) {
    if (!userDatabase.count(username)) {
        return false;
    }

    const User& user = userDatabase.at(username);
    std::string inputHashedPassword = SHA256::hash(user.salt + password);

    return secure_compare(user.hashedPassword, inputHashedPassword);
}

int main() {
    std::cout << "Running 5 test cases for User Authentication..." << std::endl;

    // Test Case 1: Register a new user
    std::cout << "\n--- Test Case 1: Successful Registration ---" << std::endl;
    bool regSuccess = registerUser("alice", "Password123!");
    std::cout << "Registration for 'alice': " << (regSuccess ? "SUCCESS" : "FAIL") << std::endl;

    // Test Case 2: Authenticate with correct password
    std::cout << "\n--- Test Case 2: Successful Authentication ---" << std::endl;
    bool authSuccess = authenticateUser("alice", "Password123!");
    std::cout << "Authentication for 'alice' with correct password: " << (authSuccess ? "SUCCESS" : "FAIL") << std::endl;

    // Test Case 3: Authenticate with incorrect password
    std::cout << "\n--- Test Case 3: Failed Authentication (Wrong Password) ---" << std::endl;
    bool authFailWrongPass = authenticateUser("alice", "WrongPassword");
    std::cout << "Authentication for 'alice' with wrong password: " << (!authFailWrongPass ? "SUCCESS (Correctly Failed)" : "FAIL") << std::endl;

    // Test Case 4: Authenticate a non-existent user
    std::cout << "\n--- Test Case 4: Failed Authentication (Non-Existent User) ---" << std::endl;
    bool authFailNonExistent = authenticateUser("bob", "Password123!");
    std::cout << "Authentication for non-existent 'bob': " << (!authFailNonExistent ? "SUCCESS (Correctly Failed)" : "FAIL") << std::endl;

    // Test Case 5: Registering an existing user
    std::cout << "\n--- Test Case 5: Failed Registration (User Exists) ---" << std::endl;
    bool regFailExists = registerUser("alice", "AnotherPassword");
    std::cout << "Re-registration for 'alice': " << (!regFailExists ? "SUCCESS (Correctly Failed)" : "FAIL") << std::endl;

    // Bonus Test Case: Register another user with the same password
    std::cout << "\n--- Bonus Test Case: Salting Verification ---" << std::endl;
    registerUser("charlie", "Password123!");
    std::string aliceHash = userDatabase.at("alice").hashedPassword;
    std::string charlieHash = userDatabase.at("charlie").hashedPassword;
    bool hashesDiffer = (aliceHash != charlieHash);
    std::cout << "Alice's and Charlie's (same password) hashes are different due to salting: " << (hashesDiffer ? "SUCCESS" : "FAIL") << std::endl;

    return 0;
}
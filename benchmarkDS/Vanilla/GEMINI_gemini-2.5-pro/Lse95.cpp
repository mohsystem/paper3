#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <random>
#include <algorithm>
#include <iterator>
#include <iomanip>
#include <sstream>

// --- Start of SHA-256 implementation (picosha2.h simplified) ---
// Based on public domain implementation by Brad Conte
#define SHA256_BLOCK_SIZE 32

typedef unsigned char BYTE;
typedef unsigned int WORD;

class SHA256 {
public:
    void init();
    void update(const BYTE *message, unsigned int len);
    void final(BYTE *digest);

private:
    static const WORD K[64];
    void transform(const BYTE *message, unsigned int block_nb);
    BYTE m_data[64];
    WORD m_state[8];
    WORD m_bitlen[2];
    unsigned int m_datalen;
};

void SHA256::init() {
    m_state[0] = 0x6a09e667;
    m_state[1] = 0xbb67ae85;
    m_state[2] = 0x3c6ef372;
    m_state[3] = 0xa54ff53a;
    m_state[4] = 0x510e527f;
    m_state[5] = 0x9b05688c;
    m_state[6] = 0x1f83d9ab;
    m_state[7] = 0x5be0cd19;
    m_bitlen[0] = 0;
    m_bitlen[1] = 0;
    m_datalen = 0;
}

#define ROTLEFT(a, b) (((a) << (b)) | ((a) >> (32 - (b))))
#define ROTRIGHT(a, b) (((a) >> (b)) | ((a) << (32 - (b))))
#define CH(x, y, z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define EP0(x) (ROTRIGHT(x, 2) ^ ROTRIGHT(x, 13) ^ ROTRIGHT(x, 22))
#define EP1(x) (ROTRIGHT(x, 6) ^ ROTRIGHT(x, 11) ^ ROTRIGHT(x, 25))
#define SIG0(x) (ROTRIGHT(x, 7) ^ ROTRIGHT(x, 18) ^ ((x) >> 3))
#define SIG1(x) (ROTRIGHT(x, 17) ^ ROTRIGHT(x, 19) ^ ((x) >> 10))

const WORD SHA256::K[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

void SHA256::transform(const BYTE *message, unsigned int block_nb) {
    WORD w[64], wv[8], t1, t2;
    const BYTE *sub_block;
    int i;
    for (i = 0; i < (int)block_nb; i++) {
        sub_block = message + (i << 6);
        for (int j = 0; j < 16; j++)
            w[j] = (sub_block[j * 4] << 24) | (sub_block[j * 4 + 1] << 16) | (sub_block[j * 4 + 2] << 8) | (sub_block[j * 4 + 3]);
        for (int j = 16; j < 64; j++) w[j] = SIG1(w[j - 2]) + w[j - 7] + SIG0(w[j - 15]) + w[j - 16];
        for (int j = 0; j < 8; j++) wv[j] = m_state[j];
        for (int j = 0; j < 64; j++) {
            t1 = wv[7] + EP1(wv[4]) + CH(wv[4], wv[5], wv[6]) + K[j] + w[j];
            t2 = EP0(wv[0]) + MAJ(wv[0], wv[1], wv[2]);
            wv[7] = wv[6]; wv[6] = wv[5]; wv[5] = wv[4];
            wv[4] = wv[3] + t1; wv[3] = wv[2]; wv[2] = wv[1]; wv[1] = wv[0];
            wv[0] = t1 + t2;
        }
        for (int j = 0; j < 8; j++) m_state[j] += wv[j];
    }
}

void SHA256::update(const BYTE *message, unsigned int len) {
    unsigned int i;
    for (i = 0; i < len; i++) {
        m_data[m_datalen] = message[i];
        m_datalen++;
        if (m_datalen == 64) {
            transform(m_data, 1);
            m_bitlen[0] += 512;
            if (m_bitlen[0] == 0) m_bitlen[1]++;
            m_datalen = 0;
        }
    }
}

void SHA256::final(BYTE *digest) {
    unsigned int i = m_datalen;
    if (m_datalen < 56) {
        m_data[i++] = 0x80;
        while (i < 56) m_data[i++] = 0x00;
    } else {
        m_data[i++] = 0x80;
        while (i < 64) m_data[i++] = 0x00;
        transform(m_data, 1);
        for(i = 0; i < 56; ++i) m_data[i] = 0;
    }
    WORD len = (m_datalen * 8) + m_bitlen[0];
    m_data[63] = len & 0xFF;
    m_data[62] = (len >> 8) & 0xFF;
    m_data[61] = (len >> 16) & 0xFF;
    m_data[60] = (len >> 24) & 0xFF;
    len = (m_bitlen[1] << 3) | (m_bitlen[0] >> 29);
    m_data[59] = len & 0xFF;
    m_data[58] = (len >> 8) & 0xFF;
    m_data[57] = (len >> 16) & 0xFF;
    m_data[56] = (len >> 24) & 0xFF;
    transform(m_data, 1);
    for (i = 0; i < 4; i++) {
        digest[i]      = (m_state[0] >> (24 - i * 8)) & 0xff;
        digest[i + 4]  = (m_state[1] >> (24 - i * 8)) & 0xff;
        digest[i + 8]  = (m_state[2] >> (24 - i * 8)) & 0xff;
        digest[i + 12] = (m_state[3] >> (24 - i * 8)) & 0xff;
        digest[i + 16] = (m_state[4] >> (24 - i * 8)) & 0xff;
        digest[i + 20] = (m_state[5] >> (24 - i * 8)) & 0xff;
        digest[i + 24] = (m_state[6] >> (24 - i * 8)) & 0xff;
        digest[i + 28] = (m_state[7] >> (24 - i * 8)) & 0xff;
    }
}
// --- End of SHA-256 implementation ---

const int SALT_LENGTH = 16;

struct UserData {
    std::vector<BYTE> salt;
    std::vector<BYTE> hashedPassword;
};

std::unordered_map<std::string, UserData> database;

std::vector<BYTE> generateSalt() {
    std::vector<BYTE> salt(SALT_LENGTH);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(0, 255);
    std::generate(salt.begin(), salt.end(), [&]() { return dist(gen); });
    return salt;
}

std::vector<BYTE> hashPassword(const std::string& password, const std::vector<BYTE>& salt) {
    std::string combined = password;
    combined.append(salt.begin(), salt.end());
    
    SHA256 sha;
    sha.init();
    sha.update(reinterpret_cast<const BYTE*>(combined.c_str()), combined.length());
    
    std::vector<BYTE> hash(SHA256_BLOCK_SIZE);
    sha.final(hash.data());
    
    return hash;
}

bool registerUser(const std::string& username, const std::string& password) {
    if (database.count(username)) {
        std::cout << "Registration failed: Username '" << username << "' already exists." << std::endl;
        return false;
    }
    auto salt = generateSalt();
    auto hashedPassword = hashPassword(password, salt);
    database[username] = {salt, hashedPassword};
    std::cout << "Registration successful for username: " << username << std::endl;
    return true;
}

bool verifyLogin(const std::string& username, const std::string& password) {
    if (!database.count(username)) {
        std::cout << "Login failed: Username '" << username << "' not found." << std::endl;
        return false;
    }
    const auto& userData = database.at(username);
    auto enteredHashedPassword = hashPassword(password, userData.salt);

    if (userData.hashedPassword == enteredHashedPassword) {
        std::cout << "Login successful for username: " << username << std::endl;
        return true;
    } else {
        std::cout << "Login failed: Incorrect password for username '" << username << "'." << std::endl;
        return false;
    }
}

int main() {
    std::cout << "--- Test Case 1: Successful Registration ---" << std::endl;
    registerUser("alice", "password123");
    
    std::cout << "\n--- Test Case 2: Successful Login ---" << std::endl;
    verifyLogin("alice", "password123");
    
    std::cout << "\n--- Test Case 3: Failed Login (Incorrect Password) ---" << std::endl;
    verifyLogin("alice", "wrongpassword");

    std::cout << "\n--- Test Case 4: Failed Login (User Not Found) ---" << std::endl;
    verifyLogin("bob", "password123");

    std::cout << "\n--- Test Case 5: Failed Registration (Username Exists) ---" << std::endl;
    registerUser("alice", "anotherpassword");

    return 0;
}
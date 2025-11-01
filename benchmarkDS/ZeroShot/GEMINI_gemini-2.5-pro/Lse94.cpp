#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <random>
#include <iomanip>
#include <sstream>

// --- Start of embedded SHA-256 implementation ---
// A simple, header-only style SHA-256 implementation.
// In a production environment, use a well-vetted library like OpenSSL or Crypto++.
class SHA256 {
protected:
    typedef unsigned char uint8;
    typedef unsigned int uint32;
    typedef unsigned long long uint64;

    const static uint32 sha256_k[];
    static const unsigned int SHA224_256_BLOCK_SIZE = (512 / 8);
public:
    void init();
    void update(const unsigned char *message, unsigned int len);
    void final(unsigned char *digest);
    static const unsigned int DIGEST_SIZE = (256 / 8);

protected:
    void transform(const unsigned char *message, unsigned int block_nb);
    unsigned int m_tot_len;
    unsigned int m_len;
    unsigned char m_block[2 * SHA224_256_BLOCK_SIZE];
    uint32 m_h[8];
};

#define SHA2_SHFR(x, n)    (x >> n)
#define SHA2_ROTR(x, n)   ((x >> n) | (x << ((sizeof(x) << 3) - n)))
#define SHA2_ROTL(x, n)   ((x << n) | (x >> ((sizeof(x) << 3) - n)))
#define SHA2_CH(x, y, z)  ((x & y) ^ (~x & z))
#define SHA2_MAJ(x, y, z) ((x & y) ^ (x & z) ^ (y & z))
#define SHA256_F1(x) (SHA2_ROTR(x,  2) ^ SHA2_ROTR(x, 13) ^ SHA2_ROTR(x, 22))
#define SHA256_F2(x) (SHA2_ROTR(x,  6) ^ SHA2_ROTR(x, 11) ^ SHA2_ROTR(x, 25))
#define SHA256_F3(x) (SHA2_ROTR(x,  7) ^ SHA2_ROTR(x, 18) ^ SHA2_SHFR(x,  3))
#define SHA256_F4(x) (SHA2_ROTR(x, 17) ^ SHA2_ROTR(x, 19) ^ SHA2_SHFR(x, 10))
#define SHA2_UNPACK32(x, str)                 \
{                                             \
    *((str) + 3) = (uint8) ((x)      );       \
    *((str) + 2) = (uint8) ((x) >>  8);       \
    *((str) + 1) = (uint8) ((x) >> 16);       \
    *((str) + 0) = (uint8) ((x) >> 24);       \
}
#define SHA2_PACK32(str, x)                   \
{                                             \
    *(x) =   ((uint32) *((str) + 3)      )    \
           | ((uint32) *((str) + 2) <<  8)    \
           | ((uint32) *((str) + 1) << 16)    \
           | ((uint32) *((str) + 0) << 24);   \
}

const unsigned int SHA256::sha256_k[64] =
    {0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
     0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
     0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
     0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
     0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
     0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
     0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
     0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
     0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
     0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
     0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
     0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
     0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
     0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
     0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
     0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};

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
            SHA2_PACK32(&sub_block[j << 2], &w[j]);
        }
        for (j = 16; j < 64; j++) {
            w[j] = SHA256_F4(w[j - 2]) + w[j - 7] + SHA256_F3(w[j - 15]) + w[j - 16];
        }
        for (j = 0; j < 8; j++) {
            wv[j] = m_h[j];
        }
        for (j = 0; j < 64; j++) {
            t1 = wv[7] + SHA256_F2(wv[4]) + SHA2_CH(wv[4], wv[5], wv[6]) + sha256_k[j] + w[j];
            t2 = SHA256_F1(wv[0]) + SHA2_MAJ(wv[0], wv[1], wv[2]);
            wv[7] = wv[6];
            wv[6] = wv[5];
            wv[5] = wv[4];
            wv[4] = wv[3] + t1;
            wv[3] = wv[2];
            wv[2] = wv[1];
            wv[1] = wv[0];
            wv[0] = t1 + t2;
        }
        for (j = 0; j < 8; j++) {
            m_h[j] += wv[j];
        }
    }
}

void SHA256::init() {
    m_h[0] = 0x6a09e667;
    m_h[1] = 0xbb67ae85;
    m_h[2] = 0x3c6ef372;
    m_h[3] = 0xa54ff53a;
    m_h[4] = 0x510e527f;
    m_h[5] = 0x9b05688c;
    m_h[6] = 0x1f83d9ab;
    m_h[7] = 0x5be0cd19;
    m_len = 0;
    m_tot_len = 0;
}

void SHA256::update(const unsigned char *message, unsigned int len) {
    unsigned int block_nb;
    unsigned int new_len, rem_len, tmp_len;
    const unsigned char *shifted_message;
    tmp_len = SHA224_256_BLOCK_SIZE - m_len;
    rem_len = len < tmp_len ? len : tmp_len;
    memcpy(&m_block[m_len], message, rem_len);
    if (m_len + len < SHA224_256_BLOCK_SIZE) {
        m_len += len;
        return;
    }
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

void SHA256::final(unsigned char *digest) {
    unsigned int block_nb;
    unsigned int pm_len;
    unsigned int len_b;
    int i;
    block_nb = (1 + ((SHA224_256_BLOCK_SIZE - 9) < (m_len % SHA224_256_BLOCK_SIZE)));
    len_b = (m_tot_len + m_len) << 3;
    pm_len = block_nb << 6;
    memset(m_block + m_len, 0, pm_len - m_len);
    m_block[m_len] = 0x80;
    SHA2_UNPACK32(len_b, m_block + pm_len - 4);
    transform(m_block, block_nb);
    for (i = 0; i < 8; i++) {
        SHA2_UNPACK32(m_h[i], &digest[i << 2]);
    }
}
// --- End of embedded SHA-256 implementation ---

// A simple struct to represent a user record.
struct User {
    std::string username;
    std::vector<unsigned char> salt;
    std::vector<unsigned char> hashedPassword;
};

const int SALT_LENGTH = 16; // 16 bytes

/**
 * Helper to convert a byte vector to a hex string for printing.
 */
std::string bytes_to_hex(const std::vector<unsigned char>& bytes) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (unsigned char byte : bytes) {
        ss << std::setw(2) << static_cast<int>(byte);
    }
    return ss.str();
}

/**
 * Generates a cryptographically secure random salt.
 * Uses std::random_device which is typically a non-deterministic generator.
 */
std::vector<unsigned char> generate_salt() {
    std::vector<unsigned char> salt(SALT_LENGTH);
    std::random_device rd;
    // Mersenne Twister engine seeded with random_device
    std::mt19937 gen(rd()); 
    std::uniform_int_distribution<> distrib(0, 255);
    for (int i = 0; i < SALT_LENGTH; ++i) {
        salt[i] = static_cast<unsigned char>(distrib(gen));
    }
    return salt;
}

/**
 * Hashes a password with a given salt using the embedded SHA-256 implementation.
 * NOTE: For password hashing, algorithms like Argon2 or bcrypt are recommended
 * over a simple SHA-256 hash. This is for demonstration purposes.
 */
std::vector<unsigned char> hash_password(const std::string& password, const std::vector<unsigned char>& salt) {
    SHA256 sha256;
    sha256.init();

    // Hash the salt first, then the password
    sha256.update(salt.data(), salt.size());
    sha256.update(reinterpret_cast<const unsigned char*>(password.c_str()), password.length());

    std::vector<unsigned char> digest(SHA256::DIGEST_SIZE);
    sha256.final(digest.data());
    
    return digest;
}

/**
 * Registers a user by hashing their password with a new salt and storing it.
 */
bool register_user(const std::string& username, const std::string& password, std::unordered_map<std::string, User>& database) {
    if (username.empty() || password.empty()) {
        std::cerr << "Error: Username and password cannot be empty." << std::endl;
        return false;
    }
    if (database.count(username)) {
        std::cerr << "Error: Username '" << username << "' already exists." << std::endl;
        return false;
    }

    User newUser;
    newUser.username = username;
    newUser.salt = generate_salt();
    newUser.hashedPassword = hash_password(password, newUser.salt);

    database[username] = newUser;
    std::cout << "User '" << username << "' registered successfully." << std::endl;
    return true;
}

int main() {
    // This map simulates a database.
    std::unordered_map<std::string, User> userDatabase;

    std::cout << "--- Running C++ Test Cases ---" << std::endl;
    
    // Test Case 1: Standard registration
    register_user("alice", "Password123!", userDatabase);

    // Test Case 2: Another standard registration
    register_user("bob", "MySecureP@ssw0rd", userDatabase);

    // Test Case 3: Attempt to register a user that already exists
    register_user("alice", "AnotherPassword", userDatabase);

    // Test Case 4: Registration with a simple password
    register_user("charlie", "password", userDatabase);

    // Test Case 5: Registration with special characters in username
    register_user("dave-the-admin", "P@$$w0rd_W1th_Ch@r$", userDatabase);

    std::cout << "\n--- Final Database State ---" << std::endl;
    for (const auto& pair : userDatabase) {
        const User& user = pair.second;
        std::cout << "Username: " << user.username
                  << ", Salt: " << bytes_to_hex(user.salt)
                  << ", Hashed Password: " << bytes_to_hex(user.hashedPassword)
                  << std::endl;
    }

    return 0;
}
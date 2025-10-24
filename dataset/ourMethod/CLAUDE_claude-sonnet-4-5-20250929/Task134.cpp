
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <iomanip>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>

// Magic header for key files
const char KEY_MAGIC[] = "KEY1";
const int KEY_VERSION = 1;
const size_t SALT_SIZE = 16;
const size_t IV_SIZE = 12;
const size_t TAG_SIZE = 16;
const size_t KEY_SIZE = 32; // AES-256
const int PBKDF2_ITERATIONS = 210000;
const size_t MAX_KEY_DATA_SIZE = 1024 * 1024; // 1MB max for key data

// RAII wrapper for OpenSSL cipher context
class CipherContext {
    EVP_CIPHER_CTX* ctx;
public:
    CipherContext() : ctx(EVP_CIPHER_CTX_new()) {
        if (!ctx) throw std::runtime_error("Failed to create cipher context");
    }
    ~CipherContext() { 
        if (ctx) EVP_CIPHER_CTX_free(ctx); 
    }
    EVP_CIPHER_CTX* get() { return ctx; }
    CipherContext(const CipherContext&) = delete;
    CipherContext& operator=(const CipherContext&) = delete;
};

// Securely clear memory before deallocation
void secure_clear(void* ptr, size_t size) {
    if (ptr && size > 0) {
        volatile unsigned char* p = static_cast<volatile unsigned char*>(ptr);
        for (size_t i = 0; i < size; ++i) {
            p[i] = 0;
        }
    }
}

// RAII wrapper for secure memory
class SecureBuffer {
    std::vector<unsigned char> buffer;
public:
    explicit SecureBuffer(size_t size) : buffer(size) {}
    ~SecureBuffer() { secure_clear(buffer.data(), buffer.size()); }
    unsigned char* data() { return buffer.data(); }
    size_t size() const { return buffer.size(); }
    SecureBuffer(const SecureBuffer&) = delete;
    SecureBuffer& operator=(const SecureBuffer&) = delete;
};

// Derive key from passphrase using PBKDF2-HMAC-SHA-256
bool derive_key(const std::string& passphrase, const unsigned char* salt, 
                size_t salt_size, unsigned char* key, size_t key_size) {
    if (!salt || salt_size != SALT_SIZE || !key || key_size != KEY_SIZE) {
        return false;
    }
    
    // Use PBKDF2 with SHA-256 and secure iteration count
    int result = PKCS5_PBKDF2_HMAC(
        passphrase.c_str(), static_cast<int>(passphrase.length()),
        salt, static_cast<int>(salt_size),
        PBKDF2_ITERATIONS,
        EVP_sha256(),
        static_cast<int>(key_size), key
    );
    
    return result == 1;
}

// Encrypt key data using AES-256-GCM
bool encrypt_key_data(const std::string& passphrase, const unsigned char* plaintext,
                     size_t plaintext_len, std::vector<unsigned char>& output) {
    if (!plaintext || plaintext_len == 0 || plaintext_len > MAX_KEY_DATA_SIZE) {
        return false;
    }
    
    try {
        // Generate random salt and IV using secure RNG
        SecureBuffer salt(SALT_SIZE);
        SecureBuffer iv(IV_SIZE);
        
        if (RAND_bytes(salt.data(), SALT_SIZE) != 1 ||
            RAND_bytes(iv.data(), IV_SIZE) != 1) {
            return false;
        }
        
        // Derive encryption key from passphrase
        SecureBuffer key(KEY_SIZE);
        if (!derive_key(passphrase, salt.data(), SALT_SIZE, key.data(), KEY_SIZE)) {
            return false;
        }
        
        // Initialize encryption context
        CipherContext ctx;
        if (EVP_EncryptInit_ex(ctx.get(), EVP_aes_256_gcm(), nullptr, 
                               key.data(), iv.data()) != 1) {
            return false;
        }
        
        // Allocate output buffer: header + salt + iv + ciphertext + tag
        size_t header_size = 8; // 4 bytes magic + 4 bytes version
        size_t total_size = header_size + SALT_SIZE + IV_SIZE + plaintext_len + TAG_SIZE;
        output.resize(total_size);
        
        // Write header
        std::memcpy(output.data(), KEY_MAGIC, 4);
        uint32_t version = KEY_VERSION;
        std::memcpy(output.data() + 4, &version, 4);
        
        // Write salt and IV
        std::memcpy(output.data() + header_size, salt.data(), SALT_SIZE);
        std::memcpy(output.data() + header_size + SALT_SIZE, iv.data(), IV_SIZE);
        
        // Encrypt data
        int len = 0;
        unsigned char* ciphertext_ptr = output.data() + header_size + SALT_SIZE + IV_SIZE;
        if (EVP_EncryptUpdate(ctx.get(), ciphertext_ptr, &len, 
                             plaintext, static_cast<int>(plaintext_len)) != 1) {
            return false;
        }
        int ciphertext_len = len;
        
        // Finalize encryption
        if (EVP_EncryptFinal_ex(ctx.get(), ciphertext_ptr + len, &len) != 1) {
            return false;
        }
        ciphertext_len += len;
        
        // Get authentication tag
        unsigned char* tag_ptr = output.data() + header_size + SALT_SIZE + IV_SIZE + ciphertext_len;
        if (EVP_CIPHER_CTX_ctrl(ctx.get(), EVP_CTRL_GCM_GET_TAG, TAG_SIZE, tag_ptr) != 1) {
            return false;
        }
        
        // Adjust size to actual ciphertext length
        output.resize(header_size + SALT_SIZE + IV_SIZE + ciphertext_len + TAG_SIZE);
        
        return true;
    } catch (...) {
        return false;
    }
}

// Decrypt key data using AES-256-GCM
bool decrypt_key_data(const std::string& passphrase, const unsigned char* encrypted,
                     size_t encrypted_len, std::vector<unsigned char>& plaintext) {
    if (!encrypted || encrypted_len < 8 + SALT_SIZE + IV_SIZE + TAG_SIZE) {
        return false;
    }
    
    try {
        // Validate magic and version
        if (std::memcmp(encrypted, KEY_MAGIC, 4) != 0) {
            return false;
        }
        
        uint32_t version;
        std::memcpy(&version, encrypted + 4, 4);
        if (version != KEY_VERSION) {
            return false;
        }
        
        // Extract salt and IV
        const unsigned char* salt = encrypted + 8;
        const unsigned char* iv = salt + SALT_SIZE;
        const unsigned char* ciphertext = iv + IV_SIZE;
        size_t ciphertext_len = encrypted_len - 8 - SALT_SIZE - IV_SIZE - TAG_SIZE;
        
        if (ciphertext_len > MAX_KEY_DATA_SIZE) {
            return false;
        }
        
        const unsigned char* tag = ciphertext + ciphertext_len;
        
        // Derive decryption key
        SecureBuffer key(KEY_SIZE);
        if (!derive_key(passphrase, salt, SALT_SIZE, key.data(), KEY_SIZE)) {
            return false;
        }
        
        // Initialize decryption context
        CipherContext ctx;
        if (EVP_DecryptInit_ex(ctx.get(), EVP_aes_256_gcm(), nullptr, 
                               key.data(), iv) != 1) {
            return false;
        }
        
        // Decrypt data
        plaintext.resize(ciphertext_len);
        int len = 0;
        if (EVP_DecryptUpdate(ctx.get(), plaintext.data(), &len, 
                             ciphertext, static_cast<int>(ciphertext_len)) != 1) {
            secure_clear(plaintext.data(), plaintext.size());
            return false;
        }
        int plaintext_len = len;
        
        // Set expected tag for verification
        if (EVP_CIPHER_CTX_ctrl(ctx.get(), EVP_CTRL_GCM_SET_TAG, TAG_SIZE, 
                               const_cast<unsigned char*>(tag)) != 1) {
            secure_clear(plaintext.data(), plaintext.size());
            return false;
        }
        
        // Finalize decryption and verify tag
        if (EVP_DecryptFinal_ex(ctx.get(), plaintext.data() + len, &len) != 1) {
            // Tag verification failed - clear plaintext and fail closed
            secure_clear(plaintext.data(), plaintext.size());
            return false;
        }
        plaintext_len += len;
        
        plaintext.resize(plaintext_len);
        return true;
    } catch (...) {
        return false;
    }
}

// Store encrypted key to file with atomic write
bool store_key(const std::string& filename, const std::string& passphrase, 
               const std::string& key_data) {
    if (filename.empty() || passphrase.empty() || key_data.empty()) {
        return false;
    }
    
    // Validate filename doesn't contain path traversal\n    if (filename.find("..") != std::string::npos || \n        filename.find('/') != std::string::npos ||\n        filename.find('\\\\') != std::string::npos) {\n        return false;\n    }\n    \n    std::vector<unsigned char> encrypted;\n    if (!encrypt_key_data(passphrase, \n                         reinterpret_cast<const unsigned char*>(key_data.c_str()),\n                         key_data.length(), encrypted)) {\n        return false;\n    }\n    \n    // Write to temporary file first\n    std::string temp_filename = filename + ".tmp";\n    std::ofstream ofs(temp_filename, std::ios::binary);\n    if (!ofs) {\n        return false;\n    }\n    \n    ofs.write(reinterpret_cast<const char*>(encrypted.data()), encrypted.size());\n    ofs.flush();\n    \n    if (!ofs.good()) {\n        ofs.close();\n        std::remove(temp_filename.c_str());\n        return false;\n    }\n    \n    ofs.close();\n    \n    // Atomic rename to final filename\n    if (std::rename(temp_filename.c_str(), filename.c_str()) != 0) {\n        std::remove(temp_filename.c_str());\n        return false;\n    }\n    \n    return true;\n}\n\n// Load and decrypt key from file\nbool load_key(const std::string& filename, const std::string& passphrase,\n              std::string& key_data) {\n    if (filename.empty() || passphrase.empty()) {\n        return false;\n    }\n    \n    // Validate filename\n    if (filename.find("..") != std::string::npos || \n        filename.find('/') != std::string::npos ||\n        filename.find('\\\\') != std::string::npos) {\n        return false;\n    }\n    \n    std::ifstream ifs(filename, std::ios::binary | std::ios::ate);\n    if (!ifs) {\n        return false;\n    }\n    \n    std::streamsize size = ifs.tellg();\n    if (size <= 0 || size > MAX_KEY_DATA_SIZE + 8 + SALT_SIZE + IV_SIZE + TAG_SIZE) {\n        return false;\n    }\n    \n    ifs.seekg(0, std::ios::beg);\n    \n    std::vector<unsigned char> encrypted(static_cast<size_t>(size));\n    if (!ifs.read(reinterpret_cast<char*>(encrypted.data()), size)) {\n        return false;\n    }\n    \n    std::vector<unsigned char> plaintext;\n    if (!decrypt_key_data(passphrase, encrypted.data(), encrypted.size(), plaintext)) {\n        return false;\n    }\n    \n    key_data.assign(plaintext.begin(), plaintext.end());\n    secure_clear(plaintext.data(), plaintext.size());\n    \n    return true;\n}\n\nint main() {\n    // Test case 1: Store and load a simple key\n    std::cout << "Test 1: Store and load simple key... ";\n    if (store_key("test_key1.enc", "SecurePass123!", "MySecretKey123") &&\n        []() {\n            std::string loaded;\n            return load_key("test_key1.enc", "SecurePass123!", loaded) && \n                   loaded == "MySecretKey123";\n        }()) {\n        std::cout << "PASSED" << std::endl;\n        std::remove("test_key1.enc");\n    } else {\n        std::cout << "FAILED" << std::endl;\n    }\n    \n    // Test case 2: Wrong passphrase should fail\n    std::cout << "Test 2: Wrong passphrase fails... ";\n    store_key("test_key2.enc", "CorrectPass", "SensitiveData");\n    std::string dummy;\n    if (!load_key("test_key2.enc", "WrongPass", dummy)) {\n        std::cout << "PASSED" << std::endl;\n    } else {\n        std::cout << "FAILED" << std::endl;\n    }\n    std::remove("test_key2.enc");\n    \n    // Test case 3: Store and load longer key data\n    std::cout << "Test 3: Store and load longer key... ";\n    std::string long_key(512, 'A');\n    if (store_key("test_key3.enc", "LongDataPass", long_key) &&\n        []() {\n            std::string loaded;\n            std::string expected(512, 'A');
            return load_key("test_key3.enc", "LongDataPass", loaded) && 
                   loaded == expected;
        }()) {
        std::cout << "PASSED" << std::endl;
        std::remove("test_key3.enc");
    } else {
        std::cout << "FAILED" << std::endl;
    }
    
    // Test case 4: Invalid filename with path traversal
    std::cout << "Test 4: Path traversal rejected... ";
    if (!store_key("../test_key4.enc", "Pass", "Data")) {
        std::cout << "PASSED" << std::endl;
    } else {
        std::cout << "FAILED" << std::endl;
    }
    
    // Test case 5: Empty inputs should fail
    std::cout << "Test 5: Empty inputs rejected... ";
    if (!store_key("", "pass", "data") && 
        !store_key("file", "", "data") &&
        !store_key("file", "pass", "")) {
        std::cout << "PASSED" << std::endl;
    } else {
        std::cout << "FAILED" << std::endl;
    }
    
    return 0;
}

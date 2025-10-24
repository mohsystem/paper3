// Note: This code requires the OpenSSL library.
// Compile with: g++ -o your_program_name your_source_file.cpp -lssl -lcrypto
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <stdexcept>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>

// Constants
constexpr int KEY_SIZE = 32; // 256-bit
constexpr int SALT_SIZE = 16;
constexpr int IV_SIZE = 12; // GCM recommended
constexpr int TAG_SIZE = 16; // GCM standard
constexpr int PBKDF2_ITERATIONS = 65536;
constexpr int BUFFER_SIZE = 4096;

void handle_openssl_errors() {
    throw std::runtime_error("OpenSSL error occurred.");
}

bool getKeyFromPassword(const std::string& password, const std::vector<unsigned char>& salt, std::vector<unsigned char>& key) {
    key.resize(KEY_SIZE);
    if (!PKCS5_PBKDF2_HMAC(password.c_str(), password.length(), salt.data(), salt.size(), PBKDF2_ITERATIONS, EVP_sha256(), KEY_SIZE, key.data())) {
        return false;
    }
    return true;
}

bool encryptFile(const std::string& inputFile, const std::string& outputFile, const std::string& password) {
    std::ifstream inFile(inputFile, std::ios::binary);
    if (!inFile) return false;
    std::ofstream outFile(outputFile, std::ios::binary);
    if (!outFile) return false;

    std::vector<unsigned char> salt(SALT_SIZE);
    if (!RAND_bytes(salt.data(), SALT_SIZE)) return false;

    std::vector<unsigned char> key;
    if (!getKeyFromPassword(password, salt, key)) return false;
    
    std::vector<unsigned char> iv(IV_SIZE);
    if (!RAND_bytes(iv.data(), IV_SIZE)) return false;

    outFile.write(reinterpret_cast<const char*>(salt.data()), SALT_SIZE);
    outFile.write(reinterpret_cast<const char*>(iv.data()), IV_SIZE);
    
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return false;

    bool success = true;
    try {
        if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL)) handle_openssl_errors();
        if (1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, IV_SIZE, NULL)) handle_openssl_errors();
        if (1 != EVP_EncryptInit_ex(ctx, NULL, NULL, key.data(), iv.data())) handle_openssl_errors();

        std::vector<unsigned char> inBuffer(BUFFER_SIZE);
        std::vector<unsigned char> outBuffer(BUFFER_SIZE + EVP_MAX_BLOCK_LENGTH);
        int len = 0;

        while (inFile.read(reinterpret_cast<char*>(inBuffer.data()), BUFFER_SIZE)) {
            if (1 != EVP_EncryptUpdate(ctx, outBuffer.data(), &len, inBuffer.data(), inFile.gcount())) handle_openssl_errors();
            outFile.write(reinterpret_cast<const char*>(outBuffer.data()), len);
        }
        if (inFile.gcount() > 0) {
             if (1 != EVP_EncryptUpdate(ctx, outBuffer.data(), &len, inBuffer.data(), inFile.gcount())) handle_openssl_errors();
             outFile.write(reinterpret_cast<const char*>(outBuffer.data()), len);
        }

        if (1 != EVP_EncryptFinal_ex(ctx, outBuffer.data(), &len)) handle_openssl_errors();
        outFile.write(reinterpret_cast<const char*>(outBuffer.data()), len);

        std::vector<unsigned char> tag(TAG_SIZE);
        if (1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, TAG_SIZE, tag.data())) handle_openssl_errors();
        outFile.write(reinterpret_cast<const char*>(tag.data()), TAG_SIZE);

    } catch (...) {
        success = false;
    }
    
    EVP_CIPHER_CTX_free(ctx);
    return success;
}

bool decryptFile(const std::string& inputFile, const std::string& outputFile, const std::string& password) {
    std::ifstream inFile(inputFile, std::ios::binary);
    if (!inFile) return false;
    std::ofstream outFile(outputFile, std::ios::binary);
    if (!outFile) return false;

    std::vector<unsigned char> salt(SALT_SIZE);
    inFile.read(reinterpret_cast<char*>(salt.data()), SALT_SIZE);
    if (inFile.gcount() != SALT_SIZE) return false;

    std::vector<unsigned char> iv(IV_SIZE);
    inFile.read(reinterpret_cast<char*>(iv.data()), IV_SIZE);
    if (inFile.gcount() != IV_SIZE) return false;

    // Get tag from the end of the file
    inFile.seekg(-TAG_SIZE, std::ios::end);
    std::vector<unsigned char> tag(TAG_SIZE);
    inFile.read(reinterpret_cast<char*>(tag.data()), TAG_SIZE);
    if (inFile.gcount() != TAG_SIZE) return false;
    
    // Position back to start of ciphertext
    inFile.seekg(SALT_SIZE + IV_SIZE, std::ios::beg);
    long long ciphertext_size = static_cast<long long>(inFile.tellg()); // current pos
    inFile.seekg(-TAG_SIZE, std::ios::end);
    ciphertext_size = static_cast<long long>(inFile.tellg()) - ciphertext_size;
    inFile.seekg(SALT_SIZE + IV_SIZE, std::ios::beg);

    std::vector<unsigned char> key;
    if (!getKeyFromPassword(password, salt, key)) return false;

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return false;

    bool success = true;
    try {
        if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL)) handle_openssl_errors();
        if (1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, IV_SIZE, NULL)) handle_openssl_errors();
        if (1 != EVP_DecryptInit_ex(ctx, NULL, NULL, key.data(), iv.data())) handle_openssl_errors();
       
        std::vector<unsigned char> inBuffer(BUFFER_SIZE);
        std::vector<unsigned char> outBuffer(BUFFER_SIZE + EVP_MAX_BLOCK_LENGTH);
        int len = 0;
        long long read_count = 0;

        while (read_count < ciphertext_size) {
            int to_read = (ciphertext_size - read_count > BUFFER_SIZE) ? BUFFER_SIZE : (ciphertext_size - read_count);
            inFile.read(reinterpret_cast<char*>(inBuffer.data()), to_read);
            if (inFile.gcount() != to_read) handle_openssl_errors();
            read_count += to_read;

            if (1 != EVP_DecryptUpdate(ctx, outBuffer.data(), &len, inBuffer.data(), to_read)) handle_openssl_errors();
            outFile.write(reinterpret_cast<const char*>(outBuffer.data()), len);
        }

        if (1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, TAG_SIZE, tag.data())) handle_openssl_errors();
        
        if (1 != EVP_DecryptFinal_ex(ctx, outBuffer.data(), &len)) {
            // This is where authentication failure is detected
            throw std::runtime_error("Authentication failed: wrong password or tampered file.");
        }
        outFile.write(reinterpret_cast<const char*>(outBuffer.data()), len);

    } catch (...) {
        success = false;
        remove(outputFile.c_str());
    }

    EVP_CIPHER_CTX_free(ctx);
    return success;
}

// Helper to check if files are identical
bool files_are_equal(const std::string& path1, const std::string& path2) {
    std::ifstream f1(path1, std::ios::binary | std::ios::ate);
    std::ifstream f2(path2, std::ios::binary | std::ios::ate);

    if (f1.fail() || f2.fail() || f1.tellg() != f2.tellg()) {
        return false;
    }
    f1.seekg(0, std::ios::beg);
    f2.seekg(0, std::ios::beg);
    return std::equal(std::istreambuf_iterator<char>(f1.rdbuf()),
                      std::istreambuf_iterator<char>(),
                      std::istreambuf_iterator<char>(f2.rdbuf()));
}

void run_test(int num, const std::string& name, bool condition) {
    std::cout << "Test Case " << num << " (" << name << "): " << (condition ? "PASSED" : "FAILED") << std::endl;
}

void cleanup(const std::vector<std::string>& files) {
    for (const auto& file : files) {
        remove(file.c_str());
    }
}

int main() {
    std::cout << "\nRunning C++ Tests..." << std::endl;
    std::string password = "my-very-secret-password-123";

    // Test Case 1: Simple text
    {
        std::ofstream("plain1.txt") << "This is a test file for encryption.";
        bool success = encryptFile("plain1.txt", "encrypted1.enc", password) &&
                       decryptFile("encrypted1.enc", "decrypted1.txt", password);
        run_test(1, "Simple Text", success && files_are_equal("plain1.txt", "decrypted1.txt"));
        cleanup({"plain1.txt", "encrypted1.enc", "decrypted1.txt"});
    }

    // Test Case 2: Larger binary
    {
        std::vector<char> data(1024 * 100);
        std::ofstream("plain2.bin", std::ios::binary).write(data.data(), data.size());
        bool success = encryptFile("plain2.bin", "encrypted2.enc", password) &&
                       decryptFile("encrypted2.enc", "decrypted2.bin", password);
        run_test(2, "Large File", success && files_are_equal("plain2.bin", "decrypted2.bin"));
        cleanup({"plain2.bin", "encrypted2.enc", "decrypted2.bin"});
    }
    
    // Test Case 3: Empty file
    {
        std::ofstream("plain3.txt") << "";
        bool success = encryptFile("plain3.txt", "encrypted3.enc", password) &&
                       decryptFile("encrypted3.enc", "decrypted3.txt", password);
        run_test(3, "Empty File", success && files_are_equal("plain3.txt", "decrypted3.txt"));
        cleanup({"plain3.txt", "encrypted3.enc", "decrypted3.txt"});
    }

    // Test Case 4: Wrong password
    {
        std::ofstream("plain4.txt") << "Test wrong password.";
        bool encrypted = encryptFile("plain4.txt", "encrypted4.enc", password);
        bool decrypted = decryptFile("encrypted4.enc", "decrypted4.txt", "wrong-password");
        run_test(4, "Wrong Password", encrypted && !decrypted);
        cleanup({"plain4.txt", "encrypted4.enc", "decrypted4.txt"});
    }
    
    // Test Case 5: Tampered file
    {
        std::ofstream("plain5.txt") << "Test tampered file.";
        bool encrypted = encryptFile("plain5.txt", "encrypted5.enc", password);
        
        // Tamper the file
        std::fstream file("encrypted5.enc", std::ios::in | std::ios::out | std::ios::binary);
        file.seekp(-5, std::ios::end);
        char byte;
        file.read(&byte, 1);
        file.seekp(-1, std::ios::cur);
        byte ^= 1;
        file.write(&byte, 1);
        file.close();

        bool decrypted = decryptFile("encrypted5.enc", "decrypted5.txt", password);
        run_test(5, "Tampered File", encrypted && !decrypted);
        cleanup({"plain5.txt", "encrypted5.enc", "decrypted5.txt"});
    }

    return 0;
}
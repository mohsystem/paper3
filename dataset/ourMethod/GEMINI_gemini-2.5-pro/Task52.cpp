#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <stdexcept>
#include <memory>
#include <cstdio> // For std::rename

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <openssl/kdf.h>

// To compile: g++ your_file.cpp -o your_app -lssl -lcrypto

// Constants based on security requirements
constexpr int KEY_SIZE = 32;       // 256 bits for AES-256
constexpr int IV_SIZE = 12;        // 96 bits for GCM, a standard size
constexpr int SALT_SIZE = 16;      // 128 bits
constexpr int TAG_SIZE = 16;       // 128 bits for GCM
constexpr int PBKDF2_ITER = 310000; // Recommended minimum is increasing

// Helper to print OpenSSL errors and throw an exception
void handle_openssl_errors() {
    unsigned long err_code;
    std::string err_details;
    while ((err_code = ERR_get_error())) {
        char err_msg[256];
        ERR_error_string_n(err_code, err_msg, sizeof(err_msg));
        err_details += std::string(err_msg) + "\n";
    }
    throw std::runtime_error("An OpenSSL error occurred: " + err_details);
}

// RAII wrapper for EVP_CIPHER_CTX
using EVP_CIPHER_CTX_ptr = std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)>;

// Securely clears a vector's memory
template<typename T>
void secure_zero_vector(std::vector<T>& vec) {
    if (!vec.empty()) {
        OPENSSL_cleanse(vec.data(), vec.size() * sizeof(T));
    }
    vec.clear();
}

bool encrypt_file(const std::string& input_path, const std::string& output_path, const std::string& password) {
    try {
        std::ifstream input_file(input_path, std::ios::binary);
        if (!input_file) {
            std::cerr << "Error: Cannot open input file: " << input_path << std::endl;
            return false;
        }
        
        std::vector<unsigned char> plaintext(
            (std::istreambuf_iterator<char>(input_file)),
            std::istreambuf_iterator<char>()
        );
        input_file.close();

        std::vector<unsigned char> salt(SALT_SIZE);
        if (RAND_bytes(salt.data(), SALT_SIZE) != 1) handle_openssl_errors();

        std::vector<unsigned char> key(KEY_SIZE);
        if (PKCS5_PBKDF2_HMAC(password.c_str(), password.length(), salt.data(), salt.size(), PBKDF2_ITER, EVP_sha256(), KEY_SIZE, key.data()) != 1) handle_openssl_errors();

        std::vector<unsigned char> iv(IV_SIZE);
        if (RAND_bytes(iv.data(), IV_SIZE) != 1) handle_openssl_errors();
        
        EVP_CIPHER_CTX_ptr ctx(EVP_CIPHER_CTX_new(), &EVP_CIPHER_CTX_free);
        if (!ctx) handle_openssl_errors();

        if (EVP_EncryptInit_ex(ctx.get(), EVP_aes_256_gcm(), nullptr, key.data(), iv.data()) != 1) handle_openssl_errors();

        std::vector<unsigned char> ciphertext(plaintext.size() + TAG_SIZE);
        int len = 0;
        if (EVP_EncryptUpdate(ctx.get(), ciphertext.data(), &len, plaintext.data(), plaintext.size()) != 1) handle_openssl_errors();
        int ciphertext_len = len;

        if (EVP_EncryptFinal_ex(ctx.get(), ciphertext.data() + len, &len) != 1) handle_openssl_errors();
        ciphertext_len += len;
        ciphertext.resize(ciphertext_len);

        std::vector<unsigned char> tag(TAG_SIZE);
        if (EVP_CIPHER_CTX_ctrl(ctx.get(), EVP_CTRL_GCM_GET_TAG, TAG_SIZE, tag.data()) != 1) handle_openssl_errors();

        std::string temp_output_path = output_path + ".tmp";
        std::ofstream output_file(temp_output_path, std::ios::binary | std::ios::trunc);
        if (!output_file) {
            std::cerr << "Error: Cannot create temporary output file." << std::endl;
            secure_zero_vector(key);
            return false;
        }

        output_file.write(reinterpret_cast<const char*>(salt.data()), SALT_SIZE);
        output_file.write(reinterpret_cast<const char*>(iv.data()), IV_SIZE);
        output_file.write(reinterpret_cast<const char*>(ciphertext.data()), ciphertext.size());
        output_file.write(reinterpret_cast<const char*>(tag.data()), TAG_SIZE);
        output_file.close();

        if (std::rename(temp_output_path.c_str(), output_path.c_str()) != 0) {
            std::cerr << "Error: Failed to rename temporary file." << std::endl;
            std::remove(temp_output_path.c_str());
            secure_zero_vector(key);
            return false;
        }

        secure_zero_vector(key);
        return true;
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return false;
    }
}

bool decrypt_file(const std::string& input_path, const std::string& output_path, const std::string& password) {
    try {
        std::ifstream input_file(input_path, std::ios::binary);
        if (!input_file) {
            std::cerr << "Error: Cannot open input file: " << input_path << std::endl;
            return false;
        }

        input_file.seekg(0, std::ios::end);
        long long file_size = input_file.tellg();
        if (file_size < SALT_SIZE + IV_SIZE + TAG_SIZE) {
            std::cerr << "Error: Invalid encrypted file format or size." << std::endl;
            return false;
        }
        input_file.seekg(0, std::ios::beg);

        std::vector<unsigned char> salt(SALT_SIZE);
        input_file.read(reinterpret_cast<char*>(salt.data()), SALT_SIZE);
        
        std::vector<unsigned char> iv(IV_SIZE);
        input_file.read(reinterpret_cast<char*>(iv.data()), IV_SIZE);

        long long ctext_size = file_size - SALT_SIZE - IV_SIZE - TAG_SIZE;
        std::vector<unsigned char> ciphertext(ctext_size);
        input_file.read(reinterpret_cast<char*>(ciphertext.data()), ctext_size);

        std::vector<unsigned char> tag(TAG_SIZE);
        input_file.read(reinterpret_cast<char*>(tag.data()), TAG_SIZE);
        input_file.close();
        
        std::vector<unsigned char> key(KEY_SIZE);
        if (PKCS5_PBKDF2_HMAC(password.c_str(), password.length(), salt.data(), salt.size(), PBKDF2_ITER, EVP_sha256(), KEY_SIZE, key.data()) != 1) handle_openssl_errors();

        EVP_CIPHER_CTX_ptr ctx(EVP_CIPHER_CTX_new(), &EVP_CIPHER_CTX_free);
        if (!ctx) handle_openssl_errors();

        if (EVP_DecryptInit_ex(ctx.get(), EVP_aes_256_gcm(), nullptr, key.data(), iv.data()) != 1) handle_openssl_errors();

        std::vector<unsigned char> plaintext(ciphertext.size());
        int len = 0;
        if (EVP_DecryptUpdate(ctx.get(), plaintext.data(), &len, ciphertext.data(), ciphertext.size()) != 1) handle_openssl_errors();
        int plaintext_len = len;

        if (EVP_CIPHER_CTX_ctrl(ctx.get(), EVP_CTRL_GCM_SET_TAG, TAG_SIZE, tag.data()) != 1) handle_openssl_errors();
        
        if (EVP_DecryptFinal_ex(ctx.get(), plaintext.data() + len, &len) <= 0) {
            std::cerr << "Error: Decryption failed. The data is corrupt or password is wrong." << std::endl;
            secure_zero_vector(key);
            return false;
        }
        plaintext_len += len;
        plaintext.resize(plaintext_len);

        std::string temp_output_path = output_path + ".tmp";
        std::ofstream output_file(temp_output_path, std::ios::binary | std::ios::trunc);
        if (!output_file) {
            std::cerr << "Error: Cannot create temporary output file." << std::endl;
            secure_zero_vector(key);
            return false;
        }

        output_file.write(reinterpret_cast<const char*>(plaintext.data()), plaintext.size());
        output_file.close();

        if (std::rename(temp_output_path.c_str(), output_path.c_str()) != 0) {
            std::cerr << "Error: Failed to rename temporary file." << std::endl;
            std::remove(temp_output_path.c_str());
            secure_zero_vector(key);
            return false;
        }
        
        secure_zero_vector(key);
        return true;
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return false;
    }
}

// Test helpers
void create_file(const std::string& path, const std::string& content) {
    std::ofstream file(path, std::ios::binary);
    file << content;
}

bool compare_files(const std::string& path1, const std::string& path2) {
    std::ifstream f1(path1, std::ios::binary | std::ios::ate);
    std::ifstream f2(path2, std::ios::binary | std::ios::ate);
    if (f1.fail() || f2.fail()) return false;
    if (f1.tellg() != f2.tellg()) return false;
    f1.seekg(0, std::ios::beg);
    f2.seekg(0, std::ios::beg);
    return std::equal(std::istreambuf_iterator<char>(f1.rdbuf()),
                      std::istreambuf_iterator<char>(),
                      std::istreambuf_iterator<char>(f2.rdbuf()));
}

int main() {
    // NOTE: In a real application, never hardcode passwords.
    // Use a secure method like environment variables or a key vault.
    const std::string password = "a_very_strong_password_123!@#";

    std::cout << "--- Running C++ Test Cases ---" << std::endl;

    // Test Case 1: Encrypt and decrypt a simple text file
    std::cout << "Test Case 1: Simple text file encryption/decryption... ";
    create_file("test1.txt", "Hello, this is a secret message.");
    if (encrypt_file("test1.txt", "test1.enc", password) &&
        decrypt_file("test1.enc", "test1.dec", password) &&
        compare_files("test1.txt", "test1.dec")) {
        std::cout << "PASSED" << std::endl;
    } else {
        std::cout << "FAILED" << std::endl;
    }

    // Test Case 2: Decryption with wrong password
    std::cout << "Test Case 2: Decryption with wrong password... ";
    if (!decrypt_file("test1.enc", "test2.dec", "wrongpassword")) {
        std::cout << "PASSED (Decryption failed as expected)" << std::endl;
    } else {
        std::cout << "FAILED (Decryption succeeded unexpectedly)" << std::endl;
    }

    // Test Case 3: Encrypt and decrypt an empty file
    std::cout << "Test Case 3: Empty file... ";
    create_file("empty.txt", "");
    if (encrypt_file("empty.txt", "empty.enc", password) &&
        decrypt_file("empty.enc", "empty.dec", password) &&
        compare_files("empty.txt", "empty.dec")) {
        std::cout << "PASSED" << std::endl;
    } else {
        std::cout << "FAILED" << std::endl;
    }
    
    // Test Case 4: File with binary data (a few non-text bytes)
    std::cout << "Test Case 4: Binary data... ";
    std::vector<char> binary_data = {'\x01', '\x02', '\x00', '\xFE', '\xFF', '\x80'};
    std::ofstream("binary.dat", std::ios::binary).write(binary_data.data(), binary_data.size());
    if (encrypt_file("binary.dat", "binary.enc", password) &&
        decrypt_file("binary.enc", "binary.dec", password) &&
        compare_files("binary.dat", "binary.dec")) {
        std::cout << "PASSED" << std::endl;
    } else {
        std::cout << "FAILED" << std::endl;
    }
    
    // Test Case 5: A slightly larger file (10KB)
    std::cout << "Test Case 5: Larger file (10KB)... ";
    std::string large_content(10 * 1024, 'X');
    create_file("large.txt", large_content);
    if (encrypt_file("large.txt", "large.enc", password) &&
        decrypt_file("large.enc", "large.dec", password) &&
        compare_files("large.txt", "large.dec")) {
        std::cout << "PASSED" << std::endl;
    } else {
        std::cout << "FAILED" << std::endl;
    }

    // Cleanup
    std::remove("test1.txt"); std::remove("test1.enc"); std::remove("test1.dec");
    std::remove("test2.dec");
    std::remove("empty.txt"); std::remove("empty.enc"); std::remove("empty.dec");
    std::remove("binary.dat"); std::remove("binary.enc"); std::remove("binary.dec");
    std::remove("large.txt"); std::remove("large.enc"); std::remove("large.dec");
    
    return 0;
}
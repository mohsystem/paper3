/*
 * NOTE: This code requires the OpenSSL library.
 * To compile on Linux/macOS:
 * g++ your_file_name.cpp -o your_program_name -lssl -lcrypto
 *
 * On Windows, you will need to install OpenSSL and link the libraries
 * (libssl.lib, libcrypto.lib) in your compiler settings.
 */
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <stdexcept>
#include <memory>

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>

// Constants
constexpr int AES_256_GCM_KEY_SIZE = 32;
constexpr int AES_256_GCM_IV_SIZE = 12;
constexpr int AES_256_GCM_TAG_SIZE = 16;

void handle_openssl_errors() {
    unsigned long err_code;
    while ((err_code = ERR_get_error())) {
        char err_buf[256];
        ERR_error_string_n(err_code, err_buf, sizeof(err_buf));
        std::cerr << "OpenSSL Error: " << err_buf << std::endl;
    }
    throw std::runtime_error("An OpenSSL error occurred.");
}

bool encryptFile(const std::string& inputFile, const std::string& outputFile, const std::vector<unsigned char>& key) {
    if (key.size() != AES_256_GCM_KEY_SIZE) {
        std::cerr << "Error: Invalid key size." << std::endl;
        return false;
    }

    std::ifstream inFile(inputFile, std::ios::binary);
    if (!inFile) {
        std::cerr << "Error: Cannot open input file " << inputFile << std::endl;
        return false;
    }

    std::vector<unsigned char> plaintext(
        (std::istreambuf_iterator<char>(inFile)),
        std::istreambuf_iterator<char>()
    );
    inFile.close();

    std::vector<unsigned char> iv(AES_256_GCM_IV_SIZE);
    if (RAND_bytes(iv.data(), iv.size()) != 1) {
        handle_openssl_errors();
    }

    std::vector<unsigned char> ciphertext(plaintext.size());
    std::vector<unsigned char> tag(AES_256_GCM_TAG_SIZE);

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) handle_openssl_errors();
    
    std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)> ctx_ptr(ctx, &EVP_CIPHER_CTX_free);

    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL) != 1) handle_openssl_errors();
    if (EVP_EncryptInit_ex(ctx, NULL, NULL, key.data(), iv.data()) != 1) handle_openssl_errors();

    int len = 0;
    if (EVP_EncryptUpdate(ctx, ciphertext.data(), &len, plaintext.data(), plaintext.size()) != 1) handle_openssl_errors();

    int ciphertext_len = len;

    if (EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len) != 1) handle_openssl_errors();
    ciphertext_len += len;
    
    if (ciphertext_len != plaintext.size()) {
        std::cerr << "Error: Ciphertext length mismatch." << std::endl;
        return false;
    }
    ciphertext.resize(ciphertext_len);

    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, AES_256_GCM_TAG_SIZE, tag.data()) != 1) handle_openssl_errors();

    std::ofstream outFile(outputFile, std::ios::binary);
    if (!outFile) {
        std::cerr << "Error: Cannot open output file " << outputFile << std::endl;
        return false;
    }
    outFile.write(reinterpret_cast<const char*>(iv.data()), iv.size());
    outFile.write(reinterpret_cast<const char*>(tag.data()), tag.size());
    outFile.write(reinterpret_cast<const char*>(ciphertext.data()), ciphertext.size());
    outFile.close();

    return true;
}

bool decryptFile(const std::string& inputFile, const std::string& outputFile, const std::vector<unsigned char>& key) {
    if (key.size() != AES_256_GCM_KEY_SIZE) {
        std::cerr << "Error: Invalid key size." << std::endl;
        return false;
    }

    std::ifstream inFile(inputFile, std::ios::binary);
    if (!inFile) {
        std::cerr << "Error: Cannot open input file " << inputFile << std::endl;
        return false;
    }

    std::vector<unsigned char> iv(AES_256_GCM_IV_SIZE);
    std::vector<unsigned char> tag(AES_256_GCM_TAG_SIZE);
    
    inFile.read(reinterpret_cast<char*>(iv.data()), iv.size());
    inFile.read(reinterpret_cast<char*>(tag.data()), tag.size());

    if (!inFile) {
        std::cerr << "Error: Invalid encrypted file format or size." << std::endl;
        return false;
    }

    std::vector<unsigned char> ciphertext(
        (std::istreambuf_iterator<char>(inFile)),
        std::istreambuf_iterator<char>()
    );
    inFile.close();

    std::vector<unsigned char> plaintext(ciphertext.size());
    int plaintext_len = 0;
    int len = 0;

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) handle_openssl_errors();
    std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)> ctx_ptr(ctx, &EVP_CIPHER_CTX_free);

    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL) != 1) handle_openssl_errors();
    if (EVP_DecryptInit_ex(ctx, NULL, NULL, key.data(), iv.data()) != 1) handle_openssl_errors();
    if (EVP_DecryptUpdate(ctx, plaintext.data(), &len, ciphertext.data(), ciphertext.size()) != 1) handle_openssl_errors();
    
    plaintext_len = len;

    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, AES_256_GCM_TAG_SIZE, tag.data()) != 1) handle_openssl_errors();
    
    int ret = EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &len);

    if (ret <= 0) {
        std::cerr << "Decryption failed: Incorrect key or tampered data." << std::endl;
        return false;
    }

    plaintext_len += len;
    plaintext.resize(plaintext_len);

    std::ofstream outFile(outputFile, std::ios::binary);
    if (!outFile) {
        std::cerr << "Error: Cannot open output file " << outputFile << std::endl;
        return false;
    }
    outFile.write(reinterpret_cast<const char*>(plaintext.data()), plaintext.size());
    outFile.close();

    return true;
}

// Helper to create a file
void createFile(const std::string& path, const std::string& content) {
    std::ofstream file(path, std::ios::binary);
    file << content;
}

// Helper to read file content
std::string readFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

// Helper to tamper a file
void tamperFile(const std::string& path) {
    std::fstream file(path, std::ios::binary | std::ios::in | std::ios::out);
    file.seekg(0, std::ios::end);
    long size = file.tellg();
    if (size > AES_256_GCM_IV_SIZE + AES_256_GCM_TAG_SIZE) {
        file.seekp(size - 1);
        char c;
        file.read(&c, 1);
        c ^= 1; // Flip a bit
        file.seekp(size - 1);
        file.write(&c, 1);
    }
}

int main() {
    const std::string ORIGINAL_FILE = "original.txt";
    const std::string ENCRYPTED_FILE = "encrypted.bin";
    const std::string DECRYPTED_FILE = "decrypted.txt";
    const std::string TAMPERED_FILE = "tampered.bin";
    const std::string EMPTY_FILE = "empty.txt";
    const std::string EMPTY_ENC_FILE = "empty.enc";
    const std::string EMPTY_DEC_FILE = "empty.dec";
    const std::string NON_EXISTENT_FILE = "non_existent.txt";

    std::vector<unsigned char> correctKey(AES_256_GCM_KEY_SIZE);
    std::vector<unsigned char> wrongKey(AES_256_GCM_KEY_SIZE);
    RAND_bytes(correctKey.data(), correctKey.size());
    RAND_bytes(wrongKey.data(), wrongKey.size());

    const std::string originalContent = "This is a secret message for testing AES-GCM file encryption.";
    
    try {
        // --- Test Case 1: Successful Encryption and Decryption ---
        std::cout << "--- Test Case 1: Successful Encryption/Decryption ---" << std::endl;
        createFile(ORIGINAL_FILE, originalContent);
        bool encryptSuccess = encryptFile(ORIGINAL_FILE, ENCRYPTED_FILE, correctKey);
        bool decryptSuccess = decryptFile(ENCRYPTED_FILE, DECRYPTED_FILE, correctKey);
        if (encryptSuccess && decryptSuccess && originalContent == readFile(DECRYPTED_FILE)) {
            std::cout << "Test 1 Passed: Content matches after decryption." << std::endl;
        } else {
            std::cout << "Test 1 Failed." << std::endl;
        }
        std::cout << "--------------------------------------------------\n" << std::endl;

        // --- Test Case 2: Decryption with Wrong Key ---
        std::cout << "--- Test Case 2: Decryption with Wrong Key ---" << std::endl;
        bool decryptWrongKey = decryptFile(ENCRYPTED_FILE, "decrypted_wrong.txt", wrongKey);
        if (!decryptWrongKey) {
            std::cout << "Test 2 Passed: Decryption failed as expected." << std::endl;
        } else {
            std::cout << "Test 2 Failed: Decryption succeeded with a wrong key." << std::endl;
        }
        std::cout << "--------------------------------------------------\n" << std::endl;

        // --- Test Case 3: Decryption of Tampered File ---
        std::cout << "--- Test Case 3: Decryption of Tampered File ---" << std::endl;
        std::ifstream src(ENCRYPTED_FILE, std::ios::binary);
        std::ofstream dst(TAMPERED_FILE, std::ios::binary);
        dst << src.rdbuf();
        src.close();
        dst.close();
        tamperFile(TAMPERED_FILE);
        bool decryptTampered = decryptFile(TAMPERED_FILE, "decrypted_tampered.txt", correctKey);
        if (!decryptTampered) {
            std::cout << "Test 3 Passed: Decryption of tampered file failed as expected." << std::endl;
        } else {
            std::cout << "Test 3 Failed: Decryption of tampered file succeeded." << std::endl;
        }
        std::cout << "--------------------------------------------------\n" << std::endl;

        // --- Test Case 4: Encrypting and Decrypting an Empty File ---
        std::cout << "--- Test Case 4: Encrypt/Decrypt Empty File ---" << std::endl;
        createFile(EMPTY_FILE, "");
        bool encryptEmpty = encryptFile(EMPTY_FILE, EMPTY_ENC_FILE, correctKey);
        bool decryptEmpty = decryptFile(EMPTY_ENC_FILE, EMPTY_DEC_FILE, correctKey);
        if (encryptEmpty && decryptEmpty && readFile(EMPTY_DEC_FILE).empty()) {
            std::cout << "Test 4 Passed: Empty file handled correctly." << std::endl;
        } else {
            std::cout << "Test 4 Failed." << std::endl;
        }
        std::cout << "--------------------------------------------------\n" << std::endl;

        // --- Test Case 5: Handling Non-Existent Input File ---
        std::cout << "--- Test Case 5: Handling Non-Existent Input File ---" << std::endl;
        bool encryptNonExistent = encryptFile(NON_EXISTENT_FILE, "out.enc", correctKey);
        if (!encryptNonExistent) {
            std::cout << "Test 5 Passed: Handled non-existent input file correctly." << std::endl;
        } else {
            std::cout << "Test 5 Failed." << std::endl;
        }
        std::cout << "--------------------------------------------------\n" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "A critical error occurred: " << e.what() << std::endl;
    }

    // --- Cleanup ---
    remove(ORIGINAL_FILE.c_str());
    remove(ENCRYPTED_FILE.c_str());
    remove(DECRYPTED_FILE.c_str());
    remove("decrypted_wrong.txt");
    remove(TAMPERED_FILE.c_str());
    remove("decrypted_tampered.txt");
    remove(EMPTY_FILE.c_str());
    remove(EMPTY_ENC_FILE.c_str());
    remove(EMPTY_DEC_FILE.c_str());
    
    return 0;
}
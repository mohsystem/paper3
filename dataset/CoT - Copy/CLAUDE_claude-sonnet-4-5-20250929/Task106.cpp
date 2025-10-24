
#include <iostream>
#include <string>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <cstring>
#include <memory>

class Task106 {
private:
    EVP_PKEY* keypair;

    std::string base64_encode(const unsigned char* buffer, size_t length) {
        BIO *bio, *b64;
        BUF_MEM *bufferPtr;

        b64 = BIO_new(BIO_f_base64());
        bio = BIO_new(BIO_s_mem());
        bio = BIO_push(b64, bio);

        BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
        BIO_write(bio, buffer, length);
        BIO_flush(bio);
        BIO_get_mem_ptr(bio, &bufferPtr);

        std::string result(bufferPtr->data, bufferPtr->length);
        BIO_free_all(bio);

        return result;
    }

    std::string base64_decode(const std::string& encoded_string) {
        BIO *bio, *b64;
        int decodeLen = encoded_string.length();
        unsigned char* buffer = (unsigned char*)malloc(decodeLen);

        bio = BIO_new_mem_buf(encoded_string.c_str(), -1);
        b64 = BIO_new(BIO_f_base64());
        bio = BIO_push(b64, bio);

        BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
        int length = BIO_read(bio, buffer, decodeLen);
        BIO_free_all(bio);

        std::string result((char*)buffer, length);
        free(buffer);

        return result;
    }

public:
    Task106() {
        EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL);
        EVP_PKEY_keygen_init(ctx);
        EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, 2048);
        EVP_PKEY_keygen(ctx, &keypair);
        EVP_PKEY_CTX_free(ctx);
    }

    ~Task106() {
        if (keypair) {
            EVP_PKEY_free(keypair);
        }
    }

    std::string encrypt(const std::string& plainText) {
        if (plainText.empty()) {
            throw std::invalid_argument("Input cannot be empty");
        }

        EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(keypair, NULL);
        EVP_PKEY_encrypt_init(ctx);
        EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING);

        size_t outlen;
        EVP_PKEY_encrypt(ctx, NULL, &outlen, 
                        (const unsigned char*)plainText.c_str(), 
                        plainText.length());

        unsigned char* encrypted = (unsigned char*)malloc(outlen);
        EVP_PKEY_encrypt(ctx, encrypted, &outlen, 
                        (const unsigned char*)plainText.c_str(), 
                        plainText.length());

        std::string result = base64_encode(encrypted, outlen);
        free(encrypted);
        EVP_PKEY_CTX_free(ctx);

        return result;
    }

    std::string decrypt(const std::string& encryptedText) {
        if (encryptedText.empty()) {
            throw std::invalid_argument("Input cannot be empty");
        }

        std::string decoded = base64_decode(encryptedText);

        EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(keypair, NULL);
        EVP_PKEY_decrypt_init(ctx);
        EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING);

        size_t outlen;
        EVP_PKEY_decrypt(ctx, NULL, &outlen, 
                        (const unsigned char*)decoded.c_str(), 
                        decoded.length());

        unsigned char* decrypted = (unsigned char*)malloc(outlen);
        EVP_PKEY_decrypt(ctx, decrypted, &outlen, 
                        (const unsigned char*)decoded.c_str(), 
                        decoded.length());

        std::string result((char*)decrypted, outlen);
        free(decrypted);
        EVP_PKEY_CTX_free(ctx);

        return result;
    }
};

int main() {
    try {
        // Test Case 1: Simple text encryption
        Task106 rsa1;
        std::string text1 = "Hello World";
        std::string encrypted1 = rsa1.encrypt(text1);
        std::string decrypted1 = rsa1.decrypt(encrypted1);
        std::cout << "Test 1:" << std::endl;
        std::cout << "Original: " << text1 << std::endl;
        std::cout << "Decrypted: " << decrypted1 << std::endl;
        std::cout << "Match: " << (text1 == decrypted1 ? "true" : "false") << std::endl << std::endl;

        // Test Case 2: Numbers and special characters
        Task106 rsa2;
        std::string text2 = "12345!@#$%";
        std::string encrypted2 = rsa2.encrypt(text2);
        std::string decrypted2 = rsa2.decrypt(encrypted2);
        std::cout << "Test 2:" << std::endl;
        std::cout << "Original: " << text2 << std::endl;
        std::cout << "Decrypted: " << decrypted2 << std::endl;
        std::cout << "Match: " << (text2 == decrypted2 ? "true" : "false") << std::endl << std::endl;

        // Test Case 3: Longer text
        Task106 rsa3;
        std::string text3 = "RSA Algorithm Test with longer message";
        std::string encrypted3 = rsa3.encrypt(text3);
        std::string decrypted3 = rsa3.decrypt(encrypted3);
        std::cout << "Test 3:" << std::endl;
        std::cout << "Original: " << text3 << std::endl;
        std::cout << "Decrypted: " << decrypted3 << std::endl;
        std::cout << "Match: " << (text3 == decrypted3 ? "true" : "false") << std::endl << std::endl;

        // Test Case 4: Unicode characters (basic ASCII for C++)
        Task106 rsa4;
        std::string text4 = "Test Message 123";
        std::string encrypted4 = rsa4.encrypt(text4);
        std::string decrypted4 = rsa4.decrypt(encrypted4);
        std::cout << "Test 4:" << std::endl;
        std::cout << "Original: " << text4 << std::endl;
        std::cout << "Decrypted: " << decrypted4 << std::endl;
        std::cout << "Match: " << (text4 == decrypted4 ? "true" : "false") << std::endl << std::endl;

        // Test Case 5: Single character
        Task106 rsa5;
        std::string text5 = "X";
        std::string encrypted5 = rsa5.encrypt(text5);
        std::string decrypted5 = rsa5.decrypt(encrypted5);
        std::cout << "Test 5:" << std::endl;
        std::cout << "Original: " << text5 << std::endl;
        std::cout << "Decrypted: " << decrypted5 << std::endl;
        std::cout << "Match: " << (text5 == decrypted5 ? "true" : "false") << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}

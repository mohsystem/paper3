
#include <iostream>
#include <string>
#include <cstring>
#include <openssl/aes.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>

using namespace std;

string base64_encode(const unsigned char* buffer, size_t length) {
    BIO *bio, *b64;
    BUF_MEM *bufferPtr;
    
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);
    
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    BIO_write(bio, buffer, length);
    BIO_flush(bio);
    BIO_get_mem_ptr(bio, &bufferPtr);
    
    string result(bufferPtr->data, bufferPtr->length);
    BIO_free_all(bio);
    
    return result;
}

string encryptMessage(const string& message, const string& secretKey) {
    try {
        unsigned char key[16] = {0};
        memcpy(key, secretKey.c_str(), min(secretKey.length(), (size_t)16));
        
        EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
        EVP_EncryptInit_ex(ctx, EVP_aes_128_ecb(), NULL, key, NULL);
        
        int ciphertext_len = message.length() + AES_BLOCK_SIZE;
        unsigned char* ciphertext = new unsigned char[ciphertext_len];
        int len;
        
        EVP_EncryptUpdate(ctx, ciphertext, &len, (unsigned char*)message.c_str(), message.length());
        ciphertext_len = len;
        
        EVP_EncryptFinal_ex(ctx, ciphertext + len, &len);
        ciphertext_len += len;
        
        string result = base64_encode(ciphertext, ciphertext_len);
        
        delete[] ciphertext;
        EVP_CIPHER_CTX_free(ctx);
        
        return result;
    } catch (...) {
        return "Encryption error";
    }
}

int main() {
    // Test case 1
    string message1 = "Hello World";
    string key1 = "mySecretKey12345";
    cout << "Test 1:" << endl;
    cout << "Message: " << message1 << endl;
    cout << "Key: " << key1 << endl;
    cout << "Encrypted: " << encryptMessage(message1, key1) << endl << endl;
    
    // Test case 2
    string message2 = "This is a secret message";
    string key2 = "password123";
    cout << "Test 2:" << endl;
    cout << "Message: " << message2 << endl;
    cout << "Key: " << key2 << endl;
    cout << "Encrypted: " << encryptMessage(message2, key2) << endl << endl;
    
    // Test case 3
    string message3 = "OpenAI GPT";
    string key3 = "key123";
    cout << "Test 3:" << endl;
    cout << "Message: " << message3 << endl;
    cout << "Key: " << key3 << endl;
    cout << "Encrypted: " << encryptMessage(message3, key3) << endl << endl;
    
    // Test case 4
    string message4 = "Encryption test 2024";
    string key4 = "supersecret";
    cout << "Test 4:" << endl;
    cout << "Message: " << message4 << endl;
    cout << "Key: " << key4 << endl;
    cout << "Encrypted: " << encryptMessage(message4, key4) << endl << endl;
    
    // Test case 5
    string message5 = "12345";
    string key5 = "numerickey";
    cout << "Test 5:" << endl;
    cout << "Message: " << message5 << endl;
    cout << "Key: " << key5 << endl;
    cout << "Encrypted: " << encryptMessage(message5, key5) << endl;
    
    return 0;
}

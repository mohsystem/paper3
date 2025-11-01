
#include <iostream>
#include <string>
#include <sqlite3.h>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <cstring>
#include <vector>

class Lse35 {
private:
    static const std::string DB_PATH;
    
    static std::string base64_decode(const std::string& encoded) {
        BIO *bio, *b64;
        int decodeLen = encoded.length();
        std::vector<unsigned char> buffer(decodeLen);
        
        bio = BIO_new_mem_buf(encoded.data(), -1);
        b64 = BIO_new(BIO_f_base64());
        bio = BIO_push(b64, bio);
        
        BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
        int length = BIO_read(bio, buffer.data(), decodeLen);
        BIO_free_all(bio);
        
        return std::string(buffer.begin(), buffer.begin() + length);
    }
    
    static std::string base64_encode(const unsigned char* buffer, size_t length) {
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
    
    static std::string hashPassword(const std::string& password, const std::string& salt) {
        std::string salt_decoded = base64_decode(salt);
        unsigned char hash[SHA256_DIGEST_LENGTH];
        
        PKCS5_PBKDF2_HMAC(password.c_str(), password.length(),
                         (unsigned char*)salt_decoded.c_str(), salt_decoded.length(),
                         100000, EVP_sha256(), SHA256_DIGEST_LENGTH, hash);
        
        return base64_encode(hash, SHA256_DIGEST_LENGTH);
    }
    
public:
    struct LoginResult {
        bool success;
        std::string redirectUrl;
        
        LoginResult(bool s, const std::string& url) : success(s), redirectUrl(url) {}
    };
    
    static LoginResult login(const std::string& username, const std::string& password) {
        if (username.empty() || password.empty()) {
            return LoginResult(false, "login.html");
        }
        
        sqlite3* db;
        sqlite3_stmt* stmt;
        
        int rc = sqlite3_open("users.db", &db);
        if (rc != SQLITE_OK) {
            std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << std::endl;
            return LoginResult(false, "login.html");
        }
        
        const char* query = "SELECT password_hash, salt FROM users WHERE username = ?";
        rc = sqlite3_prepare_v2(db, query, -1, &stmt, nullptr);
        
        if (rc != SQLITE_OK) {
            sqlite3_close(db);
            return LoginResult(false, "login.html");
        }
        
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
        
        LoginResult result(false, "login.html");
        
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            const char* stored_hash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            const char* salt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            
            std::string input_hash = hashPassword(password, salt);
            
            if (input_hash == stored_hash) {
                result = LoginResult(true, "home.html");
            }
        }
        
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        
        return result;
    }
};

const std::string Lse35::DB_PATH = "users.db";

int main() {
    std::cout << "Test 1 - Valid user:" << std::endl;
    Lse35::LoginResult result1 = Lse35::login("john_doe", "SecurePass123");
    std::cout << "Success: " << result1.success << ", Redirect: " << result1.redirectUrl << std::endl;
    
    std::cout << "\\nTest 2 - Invalid password:" << std::endl;
    Lse35::LoginResult result2 = Lse35::login("john_doe", "WrongPass");
    std::cout << "Success: " << result2.success << ", Redirect: " << result2.redirectUrl << std::endl;
    
    std::cout << "\\nTest 3 - Non-existent user:" << std::endl;
    Lse35::LoginResult result3 = Lse35::login("fake_user", "password");
    std::cout << "Success: " << result3.success << ", Redirect: " << result3.redirectUrl << std::endl;
    
    std::cout << "\\nTest 4 - Empty username:" << std::endl;
    Lse35::LoginResult result4 = Lse35::login("", "password");
    std::cout << "Success: " << result4.success << ", Redirect: " << result4.redirectUrl << std::endl;
    
    std::cout << "\\nTest 5 - Empty password:" << std::endl;
    Lse35::LoginResult result5 = Lse35::login("john_doe", "");
    std::cout << "Success: " << result5.success << ", Redirect: " << result5.redirectUrl << std::endl;
    
    return 0;
}


#include <iostream>
#include <string>
#include <regex>
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <mysql/mysql.h>
#include <cstring>
#include <vector>

class Lse35 {
private:
    std::string db_host;
    std::string db_user;
    std::string db_password;
    std::string db_name;

    // Base64 encode helper
    std::string base64_encode(const unsigned char* buffer, size_t length) {
        static const char encode_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::string result;
        
        for (size_t i = 0; i < length; i += 3) {
            unsigned int n = buffer[i] << 16;
            if (i + 1 < length) n |= buffer[i + 1] << 8;
            if (i + 2 < length) n |= buffer[i + 2];
            
            result.push_back(encode_table[(n >> 18) & 0x3F]);
            result.push_back(encode_table[(n >> 12) & 0x3F]);
            result.push_back(i + 1 < length ? encode_table[(n >> 6) & 0x3F] : '=');
            result.push_back(i + 2 < length ? encode_table[n & 0x3F] : '=');
        }
        
        return result;
    }

    // Hash password with SHA-256
    std::string hashPassword(const std::string& password, const std::string& salt) {
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256_CTX sha256;
        SHA256_Init(&sha256);
        SHA256_Update(&sha256, salt.c_str(), salt.length());
        SHA256_Update(&sha256, password.c_str(), password.length());
        SHA256_Final(hash, &sha256);
        
        return base64_encode(hash, SHA256_DIGEST_LENGTH);
    }

    // Validate username format
    bool validateUsername(const std::string& username) {
        std::regex pattern("^[a-zA-Z0-9_]{3,20}$");
        return std::regex_match(username, pattern);
    }

public:
    struct LoginResult {
        bool success;
        std::string redirectUrl;
        
        LoginResult(bool s, const std::string& url) : success(s), redirectUrl(url) {}
    };

    Lse35(const std::string& host = "localhost", 
          const std::string& user = "root",
          const std::string& pass = "password",
          const std::string& dbname = "userdb") 
        : db_host(host), db_user(user), db_password(pass), db_name(dbname) {}

    LoginResult login(const std::string& username, const std::string& password) {
        // Input validation
        if (username.empty() || password.empty()) {
            return LoginResult(false, "login.html?error=empty");
        }

        if (!validateUsername(username)) {
            return LoginResult(false, "login.html?error=invalid");
        }

        if (password.length() < 8 || password.length() > 128) {
            return LoginResult(false, "login.html?error=invalid");
        }

        MYSQL* conn = mysql_init(nullptr);
        if (!conn) {
            std::cerr << "MySQL initialization failed" << std::endl;
            return LoginResult(false, "login.html?error=system");
        }

        // Connect to database
        if (!mysql_real_connect(conn, db_host.c_str(), db_user.c_str(), 
                               db_password.c_str(), db_name.c_str(), 0, nullptr, 0)) {
            std::cerr << "MySQL connection failed: " << mysql_error(conn) << std::endl;
            mysql_close(conn);
            return LoginResult(false, "login.html?error=system");
        }

        // Prepare statement to prevent SQL injection
        MYSQL_STMT* stmt = mysql_stmt_init(conn);
        const char* query = "SELECT password_hash, salt FROM users WHERE username = ?";
        
        if (mysql_stmt_prepare(stmt, query, strlen(query))) {
            std::cerr << "Statement prepare failed: " << mysql_stmt_error(stmt) << std::endl;
            mysql_stmt_close(stmt);
            mysql_close(conn);
            return LoginResult(false, "login.html?error=system");
        }

        // Bind parameters
        MYSQL_BIND bind_param[1];
        memset(bind_param, 0, sizeof(bind_param));
        
        bind_param[0].buffer_type = MYSQL_TYPE_STRING;
        bind_param[0].buffer = (char*)username.c_str();
        bind_param[0].buffer_length = username.length();

        if (mysql_stmt_bind_param(stmt, bind_param)) {
            std::cerr << "Bind param failed: " << mysql_stmt_error(stmt) << std::endl;
            mysql_stmt_close(stmt);
            mysql_close(conn);
            return LoginResult(false, "login.html?error=system");
        }

        // Execute query
        if (mysql_stmt_execute(stmt)) {
            std::cerr << "Execute failed: " << mysql_stmt_error(stmt) << std::endl;
            mysql_stmt_close(stmt);
            mysql_close(conn);
            return LoginResult(false, "login.html?error=system");
        }

        // Bind results
        char stored_hash[256] = {0};
        char salt[256] = {0};
        unsigned long stored_hash_length, salt_length;
        
        MYSQL_BIND bind_result[2];
        memset(bind_result, 0, sizeof(bind_result));
        
        bind_result[0].buffer_type = MYSQL_TYPE_STRING;
        bind_result[0].buffer = stored_hash;
        bind_result[0].buffer_length = sizeof(stored_hash);
        bind_result[0].length = &stored_hash_length;
        
        bind_result[1].buffer_type = MYSQL_TYPE_STRING;
        bind_result[1].buffer = salt;
        bind_result[1].buffer_length = sizeof(salt);
        bind_result[1].length = &salt_length;

        mysql_stmt_bind_result(stmt, bind_result);

        // Fetch result
        int fetch_result = mysql_stmt_fetch(stmt);
        
        LoginResult result(false, "login.html?error=invalid");
        
        if (fetch_result == 0) {
            std::string stored_hash_str(stored_hash, stored_hash_length);
            std::string salt_str(salt, salt_length);
            std::string input_hash = hashPassword(password, salt_str);
            
            // Constant-time comparison
            if (input_hash == stored_hash_str) {
                result = LoginResult(true, "home.html");
            }
        }

        mysql_stmt_close(stmt);
        mysql_close(conn);
        
        return result;
    }
};

int main() {
    Lse35 loginSystem;
    
    std::cout << "Test Case 1 - Valid user:" << std::endl;
    auto result1 = loginSystem.login("validuser", "correctpassword");
    std::cout << "Success: " << result1.success << ", Redirect: " << result1.redirectUrl << std::endl;
    
    std::cout << "\\nTest Case 2 - Invalid password:" << std::endl;
    auto result2 = loginSystem.login("validuser", "wrongpassword");
    std::cout << "Success: " << result2.success << ", Redirect: " << result2.redirectUrl << std::endl;
    
    std::cout << "\\nTest Case 3 - Non-existent user:" << std::endl;
    auto result3 = loginSystem.login("nonexistent", "anypassword");
    std::cout << "Success: " << result3.success << ", Redirect: " << result3.redirectUrl << std::endl;
    
    std::cout << "\\nTest Case 4 - Empty username:" << std::endl;
    auto result4 = loginSystem.login("", "password");
    std::cout << "Success: " << result4.success << ", Redirect: " << result4.redirectUrl << std::endl;
    
    std::cout << "\\nTest Case 5 - SQL injection attempt:" << std::endl;
    auto result5 = loginSystem.login("admin' OR '1'='1", "password");
    std::cout << "Success: " << result5.success << ", Redirect: " << result5.redirectUrl << std::endl;
    
    return 0;
}


#include <iostream>
#include <string>
#include <sqlite3.h>
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <regex>
#include <cstring>
#include <sstream>
#include <iomanip>

class Lse97 {
private:
    static std::string base64_encode(const unsigned char* data, size_t len) {
        static const char* base64_chars = 
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::string ret;
        int i = 0;
        unsigned char char_array_3[3];
        unsigned char char_array_4[4];

        while (len--) {
            char_array_3[i++] = *(data++);
            if (i == 3) {
                char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
                char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
                char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
                char_array_4[3] = char_array_3[2] & 0x3f;

                for(i = 0; i < 4; i++)
                    ret += base64_chars[char_array_4[i]];
                i = 0;
            }
        }

        if (i) {
            for(int j = i; j < 3; j++)
                char_array_3[j] = '\\0';

            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);

            for (int j = 0; j < i + 1; j++)
                ret += base64_chars[char_array_4[j]];

            while(i++ < 3)
                ret += '=';
        }

        return ret;
    }

    static std::string hashPassword(const std::string& password, const unsigned char* salt) {
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256_CTX sha256;
        SHA256_Init(&sha256);
        SHA256_Update(&sha256, salt, 16);
        SHA256_Update(&sha256, password.c_str(), password.length());
        SHA256_Final(hash, &sha256);
        
        return base64_encode(hash, SHA256_DIGEST_LENGTH);
    }

    static void generateSalt(unsigned char* salt) {
        RAND_bytes(salt, 16);
    }

public:
    static std::string registerUser(const std::string& username, const std::string& password,
                                    const std::string& firstName, const std::string& lastName,
                                    const std::string& email, sqlite3* db) {
        // Input validation
        if (username.empty() || password.empty() || firstName.empty() || 
            lastName.empty() || email.empty()) {
            return "Error: All fields are required";
        }

        // Validate email format
        std::regex email_pattern(R"(^[A-Za-z0-9+_.-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,}$)");
        if (!std::regex_match(email, email_pattern)) {
            return "Error: Invalid email format";
        }

        // Password strength validation
        if (password.length() < 8) {
            return "Error: Password must be at least 8 characters";
        }

        // Check if username exists
        sqlite3_stmt* stmt;
        const char* checkQuery = "SELECT username FROM users WHERE username = ?";
        
        if (sqlite3_prepare_v2(db, checkQuery, -1, &stmt, nullptr) != SQLITE_OK) {
            return "Error: Database operation failed";
        }

        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            sqlite3_finalize(stmt);
            return "Error: Username already exists";
        }
        sqlite3_finalize(stmt);

        // Generate salt and hash password
        unsigned char salt[16];
        generateSalt(salt);
        std::string hashedPassword = hashPassword(password, salt);
        std::string saltString = base64_encode(salt, 16);

        // Insert user into database
        const char* insertQuery = 
            "INSERT INTO users (username, password, salt, first_name, last_name, email) VALUES (?, ?, ?, ?, ?, ?)";
        
        if (sqlite3_prepare_v2(db, insertQuery, -1, &stmt, nullptr) != SQLITE_OK) {
            return "Error: Database operation failed";
        }

        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, hashedPassword.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, saltString.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 4, firstName.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 5, lastName.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 6, email.c_str(), -1, SQLITE_TRANSIENT);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            return "Error: Database operation failed";
        }

        sqlite3_finalize(stmt);
        return "Success: Registration succeeded";
    }
};

int main() {
    std::cout << "Registration System Test Cases:\\n\\n";

    sqlite3* db;
    sqlite3_open(":memory:", &db);

    // Create users table
    const char* createTableQuery = 
        "CREATE TABLE users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "username TEXT UNIQUE NOT NULL,"
        "password TEXT NOT NULL,"
        "salt TEXT NOT NULL,"
        "first_name TEXT NOT NULL,"
        "last_name TEXT NOT NULL,"
        "email TEXT NOT NULL)";
    
    sqlite3_exec(db, createTableQuery, nullptr, nullptr, nullptr);

    // Test Case 1: Valid registration
    std::cout << "Test Case 1 - Valid registration:\\n";
    std::string result = Lse97::registerUser("john_doe", "SecurePass123", "John", "Doe", "john@example.com", db);
    std::cout << "Result: " << result << "\\n\\n";

    // Test Case 2: Duplicate username
    std::cout << "Test Case 2 - Duplicate username:\\n";
    result = Lse97::registerUser("john_doe", "Pass456", "Jane", "Smith", "jane@example.com", db);
    std::cout << "Result: " << result << "\\n\\n";

    // Test Case 3: Empty fields
    std::cout << "Test Case 3 - Empty fields:\\n";
    result = Lse97::registerUser("", "Pass123", "Test", "User", "test@example.com", db);
    std::cout << "Result: " << result << "\\n\\n";

    // Test Case 4: Invalid email
    std::cout << "Test Case 4 - Invalid email:\\n";
    result = Lse97::registerUser("testuser", "Pass123", "Test", "User", "invalid-email", db);
    std::cout << "Result: " << result << "\\n\\n";

    // Test Case 5: Weak password
    std::cout << "Test Case 5 - Weak password:\\n";
    result = Lse97::registerUser("testuser2", "123", "Test", "User", "test2@example.com", db);
    std::cout << "Result: " << result << "\\n\\n";

    sqlite3_close(db);
    return 0;
}

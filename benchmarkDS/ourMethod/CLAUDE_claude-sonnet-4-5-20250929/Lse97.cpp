
#include <iostream>
#include <string>
#include <regex>
#include <cstring>
#include <memory>
#include <random>
#include <sstream>
#include <iomanip>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <sqlite3.h>

// Rules#34: Validate input patterns
const std::regex USERNAME_PATTERN("^[a-zA-Z0-9_]{3,30}$");
const std::regex EMAIL_PATTERN("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\\\.[a-zA-Z]{2,}$");
const std::regex NAME_PATTERN("^[a-zA-Z\\\\s'-]{1,50}$");
const size_t MIN_PASSWORD_LENGTH = 8;
const size_t MAX_PASSWORD_LENGTH = 128;
const int PBKDF2_ITERATIONS = 210000;
const size_t SALT_LENGTH = 16;

struct RegistrationResult {
    bool success;
    std::string message;
};

// Rules#2: Input validation functions
bool validate_username(const std::string& username) {
    if (username.empty()) return false;
    return std::regex_match(username, USERNAME_PATTERN);
}

bool validate_password(const std::string& password) {
    if (password.empty()) return false;
    size_t length = password.length();
    return length >= MIN_PASSWORD_LENGTH && length <= MAX_PASSWORD_LENGTH;
}

bool validate_name(const std::string& name) {
    if (name.empty()) return false;
    return std::regex_match(name, NAME_PATTERN);
}

bool validate_email(const std::string& email) {
    if (email.empty()) return false;
    return std::regex_match(email, EMAIL_PATTERN) && email.length() <= 254;
}

// Rules#5: Generate salt with CSPRNG
std::string generate_salt() {
    unsigned char salt[SALT_LENGTH];
    // Use OpenSSL CSPRNG
    if (RAND_bytes(salt, SALT_LENGTH) != 1) {
        return "";
    }
    
    std::stringstream ss;
    for (size_t i = 0; i < SALT_LENGTH; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(salt[i]);
    }
    return ss.str();
}

// Rules#5: Hash password with PBKDF2-HMAC-SHA-256 with 210000 iterations
std::string hash_password(const std::string& password, const std::string& salt_hex) {
    // Convert hex salt to bytes
    std::vector<unsigned char> salt_bytes;
    for (size_t i = 0; i < salt_hex.length(); i += 2) {
        std::string byte_str = salt_hex.substr(i, 2);
        unsigned char byte = static_cast<unsigned char>(std::stoi(byte_str, nullptr, 16));
        salt_bytes.push_back(byte);
    }
    
    unsigned char hash[32];
    
    // Rules#5: PBKDF2-HMAC-SHA-256 with proper iterations
    if (PKCS5_PBKDF2_HMAC(password.c_str(), password.length(),
                          salt_bytes.data(), salt_bytes.size(),
                          PBKDF2_ITERATIONS, EVP_sha256(),
                          32, hash) != 1) {
        return "";
    }
    
    std::stringstream ss;
    for (int i = 0; i < 32; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    
    return ss.str();
}

// Rules#3: Get DB path from environment
std::string get_db_path() {
    const char* env_path = std::getenv("DB_PATH");
    return env_path ? std::string(env_path) : "users.db";
}

// Rules#34-42: Validate all inputs and use parameterized queries
RegistrationResult register_user(const std::string& username, const std::string& password,
                                  const std::string& first_name, const std::string& last_name,
                                  const std::string& email) {
    // Rules#34: Validate all inputs at trust boundary
    if (!validate_username(username)) {
        return {false, "Registration failed"};
    }
    if (!validate_password(password)) {
        return {false, "Registration failed"};
    }
    if (!validate_name(first_name)) {
        return {false, "Registration failed"};
    }
    if (!validate_name(last_name)) {
        return {false, "Registration failed"};
    }
    if (!validate_email(email)) {
        return {false, "Registration failed"};
    }
    
    sqlite3* db = nullptr;
    sqlite3_stmt* check_stmt = nullptr;
    sqlite3_stmt* insert_stmt = nullptr;
    
    try {
        // Rules#3: Get DB path from environment
        std::string db_path = get_db_path();
        
        if (sqlite3_open(db_path.c_str(), &db) != SQLITE_OK) {
            if (db) sqlite3_close(db);
            return {false, "Registration failed"};
        }
        
        // Rules#42: Use parameterized query to prevent SQL injection
        const char* check_sql = "SELECT COUNT(*) FROM users WHERE username = ?";
        if (sqlite3_prepare_v2(db, check_sql, -1, &check_stmt, nullptr) != SQLITE_OK) {
            sqlite3_close(db);
            return {false, "Registration failed"};
        }
        
        sqlite3_bind_text(check_stmt, 1, username.c_str(), -1, SQLITE_STATIC);
        
        int count = 0;
        if (sqlite3_step(check_stmt) == SQLITE_ROW) {
            count = sqlite3_column_int(check_stmt, 0);
        }
        sqlite3_finalize(check_stmt);
        check_stmt = nullptr;
        
        if (count > 0) {
            // Rules#11: Generic error - no info leaked about existing username
            sqlite3_close(db);
            return {false, "Registration failed"};
        }
        
        // Rules#5: Generate salt and hash password
        std::string salt = generate_salt();
        if (salt.empty()) {
            sqlite3_close(db);
            return {false, "Registration failed"};
        }
        
        std::string password_hash = hash_password(password, salt);
        if (password_hash.empty()) {
            sqlite3_close(db);
            return {false, "Registration failed"};
        }
        
        // Rules#42: Parameterized insert prevents SQL injection
        const char* insert_sql = "INSERT INTO users (username, password_hash, salt, first_name, last_name, email) VALUES (?, ?, ?, ?, ?, ?)";
        if (sqlite3_prepare_v2(db, insert_sql, -1, &insert_stmt, nullptr) != SQLITE_OK) {
            sqlite3_close(db);
            return {false, "Registration failed"};
        }
        
        sqlite3_bind_text(insert_stmt, 1, username.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(insert_stmt, 2, password_hash.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(insert_stmt, 3, salt.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(insert_stmt, 4, first_name.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(insert_stmt, 5, last_name.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(insert_stmt, 6, email.c_str(), -1, SQLITE_STATIC);
        
        int result = sqlite3_step(insert_stmt);
        sqlite3_finalize(insert_stmt);
        insert_stmt = nullptr;
        sqlite3_close(db);
        
        if (result == SQLITE_DONE) {
            return {true, "Registration succeeded"};
        } else {
            return {false, "Registration failed"};
        }
        
    } catch (...) {
        // Rules#11: Generic error message
        if (check_stmt) sqlite3_finalize(check_stmt);
        if (insert_stmt) sqlite3_finalize(insert_stmt);
        if (db) sqlite3_close(db);
        return {false, "Registration failed"};
    }
}

void init_database() {
    sqlite3* db = nullptr;
    std::string db_path = get_db_path();
    
    if (sqlite3_open(db_path.c_str(), &db) == SQLITE_OK) {
        const char* create_table = R"(\n            CREATE TABLE IF NOT EXISTS users (\n                id INTEGER PRIMARY KEY AUTOINCREMENT,\n                username TEXT UNIQUE NOT NULL,\n                password_hash TEXT NOT NULL,\n                salt TEXT NOT NULL,\n                first_name TEXT NOT NULL,\n                last_name TEXT NOT NULL,\n                email TEXT NOT NULL\n            )\n        )";
        sqlite3_exec(db, create_table, nullptr, nullptr, nullptr);
        sqlite3_close(db);
    }
}

int main() {
    init_database();
    
    // Test case 1: Valid registration
    auto r1 = register_user("john_doe", "SecurePass123!", "John", "Doe", "john@example.com");
    std::cout << "Test 1 - Valid registration: " << r1.message << std::endl;
    
    // Test case 2: Duplicate username
    auto r2 = register_user("john_doe", "AnotherPass456!", "Jane", "Smith", "jane@example.com");
    std::cout << "Test 2 - Duplicate username: " << r2.message << std::endl;
    
    // Test case 3: Invalid username (too short)
    auto r3 = register_user("ab", "ValidPass789!", "Bob", "Johnson", "bob@example.com");
    std::cout << "Test 3 - Invalid username: " << r3.message << std::endl;
    
    // Test case 4: Invalid password (too short)
    auto r4 = register_user("alice_w", "short", "Alice", "Williams", "alice@example.com");
    std::cout << "Test 4 - Invalid password: " << r4.message << std::endl;
    
    // Test case 5: Invalid email format
    auto r5 = register_user("charlie_b", "ValidPass999!", "Charlie", "Brown", "invalid-email");
    std::cout << "Test 5 - Invalid email: " << r5.message << std::endl;
    
    return 0;
}

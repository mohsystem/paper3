
#include <iostream>
#include <string>
#include <regex>
#include <random>
#include <sstream>
#include <iomanip>
#include <sqlite3.h>
#include <openssl/sha.h>

class Task46 {
private:
    static const std::string DB_NAME;
    static const std::regex EMAIL_PATTERN;
    
    static std::string generateSalt() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 255);
        
        std::stringstream ss;
        for (int i = 0; i < 16; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') << dis(gen);
        }
        return ss.str();
    }
    
    static std::string hashPassword(const std::string& password, const std::string& salt) {
        std::string salted = salt + password;
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256(reinterpret_cast<const unsigned char*>(salted.c_str()), salted.length(), hash);
        
        std::stringstream ss;
        for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
        }
        return ss.str();
    }
    
    static bool validateName(const std::string& name) {
        size_t len = name.length();
        return !name.empty() && len >= 2 && len <= 100;
    }
    
    static bool validateEmail(const std::string& email) {
        return !email.empty() && std::regex_match(email, EMAIL_PATTERN);
    }
    
    static bool validatePassword(const std::string& password) {
        size_t len = password.length();
        return !password.empty() && len >= 8 && len <= 128;
    }
    
    static std::string trim(const std::string& str) {
        size_t first = str.find_first_not_of(' ');
        if (first == std::string::npos) return "";
        size_t last = str.find_last_not_of(' ');
        return str.substr(first, (last - first + 1));
    }
    
    static std::string toLower(const std::string& str) {
        std::string result = str;
        for (char& c : result) {
            c = std::tolower(c);
        }
        return result;
    }

public:
    static void initializeDatabase() {
        sqlite3* db;
        char* errMsg = nullptr;
        
        int rc = sqlite3_open(DB_NAME.c_str(), &db);
        if (rc) {
            std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
            return;
        }
        
        const char* sql = "CREATE TABLE IF NOT EXISTS users ("
                         "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                         "name TEXT NOT NULL,"
                         "email TEXT UNIQUE NOT NULL,"
                         "password_hash TEXT NOT NULL,"
                         "salt TEXT NOT NULL)";
        
        rc = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
        if (rc != SQLITE_OK) {
            std::cerr << "SQL error: " << errMsg << std::endl;
            sqlite3_free(errMsg);
        }
        
        sqlite3_close(db);
    }
    
    static std::pair<bool, std::string> registerUser(const std::string& name, 
                                                      const std::string& email, 
                                                      const std::string& password) {
        // Input validation
        if (!validateName(name)) {
            return {false, "Invalid name. Must be 2-100 characters."};
        }
        
        if (!validateEmail(email)) {
            return {false, "Invalid email format."};
        }
        
        if (!validatePassword(password)) {
            return {false, "Invalid password. Must be 8-128 characters."};
        }
        
        // Generate salt and hash password
        std::string salt = generateSalt();
        std::string passwordHash = hashPassword(password, salt);
        
        // Store in database
        sqlite3* db;
        int rc = sqlite3_open(DB_NAME.c_str(), &db);
        if (rc) {
            return {false, "Database connection failed."};
        }
        
        const char* sql = "INSERT INTO users (name, email, password_hash, salt) VALUES (?, ?, ?, ?)";
        sqlite3_stmt* stmt;
        
        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            sqlite3_close(db);
            return {false, "Failed to prepare statement."};
        }
        
        std::string trimmedName = trim(name);
        std::string trimmedEmail = toLower(trim(email));
        
        sqlite3_bind_text(stmt, 1, trimmedName.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, trimmedEmail.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, passwordHash.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 4, salt.c_str(), -1, SQLITE_TRANSIENT);
        
        rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        
        if (rc == SQLITE_DONE) {
            return {true, "User registered successfully."};
        } else if (rc == SQLITE_CONSTRAINT) {
            return {false, "Email already registered."};
        } else {
            return {false, "Registration failed."};
        }
    }
};

const std::string Task46::DB_NAME = "users.db";
const std::regex Task46::EMAIL_PATTERN("^[A-Za-z0-9+_.-]+@[A-Za-z0-9.-]+\\\\.[A-Za-z]{2,}$");

int main() {
    Task46::initializeDatabase();
    
    std::cout << "=== User Registration System Test Cases ===" << std::endl << std::endl;
    
    // Test Case 1: Valid registration
    auto result1 = Task46::registerUser("John Doe", "john.doe@example.com", "SecurePass123");
    std::cout << "Test 1 - Valid registration: " << result1.second << std::endl;
    
    // Test Case 2: Duplicate email
    auto result2 = Task46::registerUser("Jane Doe", "john.doe@example.com", "AnotherPass456");
    std::cout << "Test 2 - Duplicate email: " << result2.second << std::endl;
    
    // Test Case 3: Invalid email format
    auto result3 = Task46::registerUser("Bob Smith", "invalid-email", "Password789");
    std::cout << "Test 3 - Invalid email: " << result3.second << std::endl;
    
    // Test Case 4: Weak password
    auto result4 = Task46::registerUser("Alice Brown", "alice@example.com", "123");
    std::cout << "Test 4 - Weak password: " << result4.second << std::endl;
    
    // Test Case 5: Invalid name
    auto result5 = Task46::registerUser("A", "test@example.com", "ValidPass123");
    std::cout << "Test 5 - Invalid name: " << result5.second << std::endl;
    
    return 0;
}

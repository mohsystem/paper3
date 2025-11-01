
#include <iostream>
#include <string>
#include <regex>
#include <sqlite3.h>

class Lse143 {
private:
    static const std::string DB_PATH;
    static const std::regex EMAIL_REGEX;
    
public:
    static std::string unsubscribe(const std::string& email) {
        // Input validation
        if (email.empty()) {
            return "Invalid email parameter";
        }
        
        std::string trimmedEmail = trim(email);
        
        // Validate email format
        if (!std::regex_match(trimmedEmail, EMAIL_REGEX)) {
            return "Invalid email format";
        }
        
        sqlite3* db = nullptr;
        sqlite3_stmt* stmt = nullptr;
        int rc;
        
        // Open database connection
        rc = sqlite3_open(DB_PATH.c_str(), &db);
        if (rc != SQLITE_OK) {
            std::string error = "Database error: " + std::string(sqlite3_errmsg(db));
            sqlite3_close(db);
            return error;
        }
        
        // Check if email exists
        const char* checkQuery = "SELECT email FROM subscribers WHERE email = ?";
        rc = sqlite3_prepare_v2(db, checkQuery, -1, &stmt, nullptr);
        
        if (rc != SQLITE_OK) {
            std::string error = "Database error: " + std::string(sqlite3_errmsg(db));
            sqlite3_close(db);
            return error;
        }
        
        sqlite3_bind_text(stmt, 1, trimmedEmail.c_str(), -1, SQLITE_TRANSIENT);
        rc = sqlite3_step(stmt);
        
        if (rc == SQLITE_ROW) {
            // Email exists, delete the user
            sqlite3_finalize(stmt);
            
            const char* deleteQuery = "DELETE FROM subscribers WHERE email = ?";
            rc = sqlite3_prepare_v2(db, deleteQuery, -1, &stmt, nullptr);
            
            if (rc != SQLITE_OK) {
                std::string error = "Database error: " + std::string(sqlite3_errmsg(db));
                sqlite3_close(db);
                return error;
            }
            
            sqlite3_bind_text(stmt, 1, trimmedEmail.c_str(), -1, SQLITE_TRANSIENT);
            rc = sqlite3_step(stmt);
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            
            if (rc == SQLITE_DONE) {
                return "Successfully unsubscribed: " + trimmedEmail;
            } else {
                return "Failed to unsubscribe";
            }
        } else {
            // Email not found
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            return "User is not subscribed";
        }
    }
    
    static void initTestDatabase() {
        sqlite3* db = nullptr;
        char* errMsg = nullptr;
        
        int rc = sqlite3_open(DB_PATH.c_str(), &db);
        if (rc != SQLITE_OK) {
            std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << std::endl;
            return;
        }
        
        const char* createTable = "CREATE TABLE IF NOT EXISTS subscribers (id INTEGER PRIMARY KEY AUTOINCREMENT, email TEXT UNIQUE NOT NULL)";
        sqlite3_exec(db, createTable, nullptr, nullptr, &errMsg);
        
        sqlite3_exec(db, "DELETE FROM subscribers", nullptr, nullptr, &errMsg);
        sqlite3_exec(db, "INSERT INTO subscribers (email) VALUES ('test1@example.com')", nullptr, nullptr, &errMsg);
        sqlite3_exec(db, "INSERT INTO subscribers (email) VALUES ('test2@example.com')", nullptr, nullptr, &errMsg);
        sqlite3_exec(db, "INSERT INTO subscribers (email) VALUES ('user@domain.com')", nullptr, nullptr, &errMsg);
        
        sqlite3_close(db);
    }
    
private:
    static std::string trim(const std::string& str) {
        size_t first = str.find_first_not_of(' ');
        if (std::string::npos == first) {
            return str;
        }
        size_t last = str.find_last_not_of(' ');
        return str.substr(first, (last - first + 1));
    }
};

const std::string Lse143::DB_PATH = "mailing_list.db";
const std::regex Lse143::EMAIL_REGEX("^[A-Za-z0-9+_.-]+@[A-Za-z0-9.-]+\\\\.[A-Za-z]{2,}$");

int main() {
    // Initialize test database
    Lse143::initTestDatabase();
    
    // Test case 1: Valid unsubscribe
    std::cout << "Test 1: " << Lse143::unsubscribe("test1@example.com") << std::endl;
    
    // Test case 2: User not subscribed
    std::cout << "Test 2: " << Lse143::unsubscribe("notfound@example.com") << std::endl;
    
    // Test case 3: Another valid unsubscribe
    std::cout << "Test 3: " << Lse143::unsubscribe("test2@example.com") << std::endl;
    
    // Test case 4: Invalid email format
    std::cout << "Test 4: " << Lse143::unsubscribe("invalid-email") << std::endl;
    
    // Test case 5: Empty email
    std::cout << "Test 5: " << Lse143::unsubscribe("") << std::endl;
    
    return 0;
}

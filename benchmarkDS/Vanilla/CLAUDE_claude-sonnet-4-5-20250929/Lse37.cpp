
#include <iostream>
#include <string>
#include <map>
#include <sqlite3.h>

class Lse37 {
private:
    static std::map<std::string, std::string> mockDatabase;
    
public:
    static void initMockDatabase() {
        mockDatabase["admin"] = "password123";
        mockDatabase["user1"] = "pass1";
        mockDatabase["user2"] = "pass2";
        mockDatabase["testuser"] = "testpass";
        mockDatabase["john"] = "doe123";
    }
    
    static std::string loginUser(const std::string& username, const std::string& password) {
        try {
            // Check if username and password are not empty
            if (username.empty() || password.empty()) {
                return "cant_login";
            }
            
            // Check credentials in database
            auto it = mockDatabase.find(username);
            if (it != mockDatabase.end() && it->second == password) {
                return "success";
            } else {
                return "cant_login";
            }
            
        } catch (...) {
            return "internal_server_error";
        }
    }
    
    static std::string loginUserWithDB(const std::string& username, const std::string& password, 
                                       const std::string& dbPath = "users.db") {
        sqlite3* db = nullptr;
        sqlite3_stmt* stmt = nullptr;
        
        try {
            // Open database connection
            int rc = sqlite3_open(dbPath.c_str(), &db);
            if (rc != SQLITE_OK) {
                sqlite3_close(db);
                return "internal_server_error";
            }
            
            // Prepare SQL query with parameterized statement
            const char* sql = "SELECT * FROM users WHERE username = ? AND password = ?";
            rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
            if (rc != SQLITE_OK) {
                sqlite3_finalize(stmt);
                sqlite3_close(db);
                return "internal_server_error";
            }
            
            // Bind parameters
            sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_STATIC);
            
            // Execute query
            rc = sqlite3_step(stmt);
            
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            
            if (rc == SQLITE_ROW) {
                return "success";
            } else {
                return "cant_login";
            }
            
        } catch (...) {
            if (stmt) sqlite3_finalize(stmt);
            if (db) sqlite3_close(db);
            return "internal_server_error";
        }
    }
};

std::map<std::string, std::string> Lse37::mockDatabase;

int main() {
    Lse37::initMockDatabase();
    
    // Test case 1: Valid credentials
    std::cout << "Test 1 - Valid login (admin/password123): " 
              << Lse37::loginUser("admin", "password123") << std::endl;
    
    // Test case 2: Invalid username
    std::cout << "Test 2 - Invalid username: " 
              << Lse37::loginUser("wronguser", "password123") << std::endl;
    
    // Test case 3: Invalid password
    std::cout << "Test 3 - Invalid password: " 
              << Lse37::loginUser("admin", "wrongpass") << std::endl;
    
    // Test case 4: Empty credentials
    std::cout << "Test 4 - Empty credentials: " 
              << Lse37::loginUser("", "") << std::endl;
    
    // Test case 5: Valid user2
    std::cout << "Test 5 - Valid login (user2/pass2): " 
              << Lse37::loginUser("user2", "pass2") << std::endl;
    
    return 0;
}

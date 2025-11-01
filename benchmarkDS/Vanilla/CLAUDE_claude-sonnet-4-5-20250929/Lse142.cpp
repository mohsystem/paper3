
#include <iostream>
#include <string>
#include <sqlite3.h>

class Lse142 {
public:
    static std::string unsubscribe(const std::string& email) {
        sqlite3* db;
        sqlite3_stmt* stmt;
        int rc;
        
        rc = sqlite3_open("emaildb.db", &db);
        if (rc != SQLITE_OK) {
            std::string error = "Cannot open database: " + std::string(sqlite3_errmsg(db));
            sqlite3_close(db);
            return error;
        }
        
        // Check if email exists
        const char* checkQuery = "SELECT email FROM subscribers WHERE email = ?";
        rc = sqlite3_prepare_v2(db, checkQuery, -1, &stmt, nullptr);
        
        if (rc != SQLITE_OK) {
            std::string error = "Failed to prepare statement: " + std::string(sqlite3_errmsg(db));
            sqlite3_close(db);
            return error;
        }
        
        sqlite3_bind_text(stmt, 1, email.c_str(), -1, SQLITE_STATIC);
        rc = sqlite3_step(stmt);
        
        if (rc == SQLITE_ROW) {
            sqlite3_finalize(stmt);
            
            // Email exists, delete it
            const char* deleteQuery = "DELETE FROM subscribers WHERE email = ?";
            rc = sqlite3_prepare_v2(db, deleteQuery, -1, &stmt, nullptr);
            
            if (rc != SQLITE_OK) {
                std::string error = "Failed to prepare delete statement: " + std::string(sqlite3_errmsg(db));
                sqlite3_close(db);
                return error;
            }
            
            sqlite3_bind_text(stmt, 1, email.c_str(), -1, SQLITE_STATIC);
            rc = sqlite3_step(stmt);
            
            if (rc == SQLITE_DONE) {
                sqlite3_finalize(stmt);
                sqlite3_close(db);
                return "Successfully unsubscribed: " + email;
            } else {
                sqlite3_finalize(stmt);
                sqlite3_close(db);
                return "Error occurred while unsubscribing";
            }
        } else {
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            return "Email not found in database: " + email;
        }
    }
};

int main() {
    // Test cases
    std::cout << "Test Case 1:" << std::endl;
    std::cout << Lse142::unsubscribe("user1@example.com") << std::endl;
    
    std::cout << "\\nTest Case 2:" << std::endl;
    std::cout << Lse142::unsubscribe("user2@example.com") << std::endl;
    
    std::cout << "\\nTest Case 3:" << std::endl;
    std::cout << Lse142::unsubscribe("notindb@example.com") << std::endl;
    
    std::cout << "\\nTest Case 4:" << std::endl;
    std::cout << Lse142::unsubscribe("user3@example.com") << std::endl;
    
    std::cout << "\\nTest Case 5:" << std::endl;
    std::cout << Lse142::unsubscribe("invalid@test.com") << std::endl;
    
    return 0;
}


#include <iostream>
#include <string>
#include <sqlite3.h>
#include <cstring>

class Lse143 {
private:
    static const char* DB_PATH;
    
public:
    static std::string unsubscribe(const std::string& email) {
        if (email.empty()) {
            return "Invalid email address";
        }
        
        sqlite3* db;
        sqlite3_stmt* stmt;
        int rc;
        
        // Open database connection
        rc = sqlite3_open(DB_PATH, &db);
        if (rc != SQLITE_OK) {
            std::string error = "Cannot open database: " + std::string(sqlite3_errmsg(db));
            sqlite3_close(db);
            return error;
        }
        
        // Check if email exists
        const char* selectQuery = "SELECT email FROM subscribers WHERE email = ?";
        rc = sqlite3_prepare_v2(db, selectQuery, -1, &stmt, nullptr);
        
        if (rc != SQLITE_OK) {
            std::string error = "SQL error: " + std::string(sqlite3_errmsg(db));
            sqlite3_close(db);
            return error;
        }
        
        sqlite3_bind_text(stmt, 1, email.c_str(), -1, SQLITE_STATIC);
        rc = sqlite3_step(stmt);
        
        std::string result;
        
        if (rc == SQLITE_ROW) {
            // Email exists, delete it
            sqlite3_finalize(stmt);
            
            const char* deleteQuery = "DELETE FROM subscribers WHERE email = ?";
            rc = sqlite3_prepare_v2(db, deleteQuery, -1, &stmt, nullptr);
            
            if (rc == SQLITE_OK) {
                sqlite3_bind_text(stmt, 1, email.c_str(), -1, SQLITE_STATIC);
                rc = sqlite3_step(stmt);
                
                if (rc == SQLITE_DONE) {
                    result = "Successfully unsubscribed: " + email;
                } else {
                    result = "Error deleting user: " + std::string(sqlite3_errmsg(db));
                }
            } else {
                result = "SQL error: " + std::string(sqlite3_errmsg(db));
            }
        } else {
            // Email not found
            result = "User is not subscribed: " + email;
        }
        
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        
        return result;
    }
    
    static void initializeDatabase() {
        sqlite3* db;
        char* errMsg = nullptr;
        
        int rc = sqlite3_open(DB_PATH, &db);
        if (rc != SQLITE_OK) {
            std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << std::endl;
            return;
        }
        
        // Create table
        const char* createTableSQL = 
            "CREATE TABLE IF NOT EXISTS subscribers ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "email TEXT UNIQUE NOT NULL)";
        
        rc = sqlite3_exec(db, createTableSQL, nullptr, nullptr, &errMsg);
        if (rc != SQLITE_OK) {
            std::cerr << "SQL error: " << errMsg << std::endl;
            sqlite3_free(errMsg);
        }
        
        // Insert test data
        const char* testEmails[] = {
            "user1@example.com",
            "user2@example.com",
            "user3@example.com"
        };
        
        for (const char* email : testEmails) {
            std::string insertSQL = "INSERT OR IGNORE INTO subscribers (email) VALUES ('" + 
                                   std::string(email) + "')";
            sqlite3_exec(db, insertSQL.c_str(), nullptr, nullptr, &errMsg);
        }
        
        sqlite3_close(db);
    }
};

const char* Lse143::DB_PATH = "mailing_list.db";

int main() {
    // Initialize database with test data
    Lse143::initializeDatabase();
    
    // Test case 1: Unsubscribe existing user
    std::cout << "Test 1: " << Lse143::unsubscribe("user1@example.com") << std::endl;
    
    // Test case 2: Try to unsubscribe same user again
    std::cout << "Test 2: " << Lse143::unsubscribe("user1@example.com") << std::endl;
    
    // Test case 3: Unsubscribe another existing user
    std::cout << "Test 3: " << Lse143::unsubscribe("user2@example.com") << std::endl;
    
    // Test case 4: Try to unsubscribe non-existent user
    std::cout << "Test 4: " << Lse143::unsubscribe("nonexistent@example.com") << std::endl;
    
    // Test case 5: Try with empty email
    std::cout << "Test 5: " << Lse143::unsubscribe("") << std::endl;
    
    return 0;
}

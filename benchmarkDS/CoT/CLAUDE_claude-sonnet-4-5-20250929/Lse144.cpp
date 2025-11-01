
#include <iostream>
#include <string>
#include <regex>
#include <sqlite3.h>

class Lse144 {
private:
    static const std::regex EMAIL_REGEX;
    
    static int unsubscribeEmail(const std::string& email) {
        // Simulated unsubscribe script call
        return 0; // Success
    }
    
public:
    static int processUnsubscribe(const std::string& email, sqlite3* db) {
        if (email.empty() || db == nullptr) {
            return 0;
        }
        
        if (!std::regex_match(email, EMAIL_REGEX)) {
            return 0;
        }
        
        sqlite3_stmt* stmt = nullptr;
        const char* query = "SELECT COUNT(*) FROM users WHERE email = ?";
        
        int rc = sqlite3_prepare_v2(db, query, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            return 0;
        }
        
        sqlite3_bind_text(stmt, 1, email.c_str(), -1, SQLITE_TRANSIENT);
        
        int result = 0;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            int count = sqlite3_column_int(stmt, 0);
            if (count > 0) {
                int scriptResult = unsubscribeEmail(email);
                result = (scriptResult == 0) ? 1 : 0;
            }
        }
        
        sqlite3_finalize(stmt);
        return result;
    }
    
    static void runTests() {
        sqlite3* db = nullptr;
        sqlite3_open(":memory:", &db);
        
        sqlite3_exec(db, "CREATE TABLE users (id INTEGER PRIMARY KEY, email TEXT)", 
                     nullptr, nullptr, nullptr);
        sqlite3_exec(db, "INSERT INTO users (email) VALUES ('test@example.com')", 
                     nullptr, nullptr, nullptr);
        
        std::cout << "Test 1: " << processUnsubscribe("test@example.com", db) << std::endl;
        std::cout << "Test 2: " << processUnsubscribe("nonexistent@example.com", db) << std::endl;
        std::cout << "Test 3: " << processUnsubscribe("invalid-email", db) << std::endl;
        std::cout << "Test 4: " << processUnsubscribe("", db) << std::endl;
        std::cout << "Test 5: " << processUnsubscribe("user@test.org", db) << std::endl;
        
        sqlite3_close(db);
    }
};

const std::regex Lse144::EMAIL_REGEX("^[A-Za-z0-9+_.-]+@[A-Za-z0-9.-]+\\\\.[A-Za-z]{2,}$");

int main() {
    Lse144::runTests();
    return 0;
}


#include <iostream>
#include <string>
#include <vector>
#include <sqlite3.h>
#include <ctime>

class Lse149 {
private:
    sqlite3* db;
    std::string dbName;

public:
    Lse149(const std::string& dbName = "messages.db") : dbName(dbName), db(nullptr) {
        initializeDatabase();
    }

    ~Lse149() {
        if (db) {
            sqlite3_close(db);
        }
    }

    void initializeDatabase() {
        int rc = sqlite3_open(dbName.c_str(), &db);
        if (rc) {
            std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
            return;
        }

        const char* sql = "CREATE TABLE IF NOT EXISTS messages ("
                         "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                         "username TEXT NOT NULL, "
                         "message TEXT NOT NULL, "
                         "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP);";
        
        char* errMsg = nullptr;
        rc = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
        if (rc != SQLITE_OK) {
            std::cerr << "SQL error: " << errMsg << std::endl;
            sqlite3_free(errMsg);
        }
    }

    void insertUserMessageInDb(const std::string& username, const std::string& message) {
        const char* sql = "INSERT INTO messages (username, message) VALUES (?, ?);";
        sqlite3_stmt* stmt;
        
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 2, message.c_str(), -1, SQLITE_STATIC);
            
            if (sqlite3_step(stmt) != SQLITE_DONE) {
                std::cerr << "Execution failed: " << sqlite3_errmsg(db) << std::endl;
            }
            sqlite3_finalize(stmt);
        }
    }

    std::vector<std::tuple<std::string, std::string, std::string>> getMessages() {
        std::vector<std::tuple<std::string, std::string, std::string>> messages;
        const char* sql = "SELECT username, message, timestamp FROM messages ORDER BY timestamp DESC;";
        sqlite3_stmt* stmt;
        
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                std::string username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
                std::string message = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
                std::string timestamp = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
                messages.push_back(std::make_tuple(username, message, timestamp));
            }
            sqlite3_finalize(stmt);
        }
        return messages;
    }

    void displayMessages() {
        auto messages = getMessages();
        std::cout << "=== Message Board ===" << std::endl;
        for (const auto& msg : messages) {
            std::cout << std::get<0>(msg) << ": " << std::get<1>(msg) 
                     << " (" << std::get<2>(msg) << ")" << std::endl;
        }
    }

    void handlePost(const std::string& username, const std::string& message) {
        if (!username.empty() && !message.empty()) {
            insertUserMessageInDb(username, message);
        }
    }
};

int main() {
    Lse149 app;
    
    std::cout << "Test Case 1: Insert message from user1" << std::endl;
    app.handlePost("user1", "Hello World!");
    
    std::cout << "Test Case 2: Insert message from user2" << std::endl;
    app.handlePost("user2", "This is a test message");
    
    std::cout << "Test Case 3: Insert message from user3" << std::endl;
    app.handlePost("user3", "C++ example");
    
    std::cout << "Test Case 4: Insert message from user4" << std::endl;
    app.handlePost("user4", "Another test message");
    
    std::cout << "Test Case 5: Display all messages" << std::endl;
    app.displayMessages();
    
    return 0;
}


#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <sqlite3.h>
#include <cstring>

class Task49 {
private:
    static const char* DB_NAME;
    
public:
    static void initDatabase() {
        sqlite3* db;
        char* errMsg = nullptr;
        
        int rc = sqlite3_open(DB_NAME, &db);
        if (rc) {
            std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
            return;
        }
        
        const char* sql = "CREATE TABLE IF NOT EXISTS users ("
                         "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                         "name TEXT NOT NULL,"
                         "email TEXT NOT NULL,"
                         "age INTEGER);";
        
        rc = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
        if (rc != SQLITE_OK) {
            std::cerr << "SQL error: " << errMsg << std::endl;
            sqlite3_free(errMsg);
        }
        
        sqlite3_close(db);
    }
    
    static std::string addUser(const std::string& name, const std::string& email, int age) {
        sqlite3* db;
        sqlite3_stmt* stmt;
        
        int rc = sqlite3_open(DB_NAME, &db);
        if (rc) {
            return "{\\"status\\":\\"error\\",\\"message\\":\\"Can't open database\\"}";
        }
        
        const char* sql = "INSERT INTO users (name, email, age) VALUES (?, ?, ?);";
        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
        
        if (rc != SQLITE_OK) {
            sqlite3_close(db);
            return "{\\"status\\":\\"error\\",\\"message\\":\\"Failed to prepare statement\\"}";
        }
        
        sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, email.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 3, age);
        
        rc = sqlite3_step(stmt);
        
        std::ostringstream oss;
        if (rc == SQLITE_DONE) {
            long long id = sqlite3_last_insert_rowid(db);
            oss << "{\\"status\\":\\"success\\",\\"id\\":" << id << "}";
        } else {
            oss << "{\\"status\\":\\"error\\",\\"message\\":\\"Failed to insert user\\"}";
        }
        
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        
        return oss.str();
    }
    
    static std::string getUsers() {
        sqlite3* db;
        sqlite3_stmt* stmt;
        
        int rc = sqlite3_open(DB_NAME, &db);
        if (rc) {
            return "{\\"status\\":\\"error\\",\\"message\\":\\"Can't open database\\"}";
        }
        
        const char* sql = "SELECT * FROM users;";
        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
        
        if (rc != SQLITE_OK) {
            sqlite3_close(db);
            return "{\\"status\\":\\"error\\",\\"message\\":\\"Failed to prepare statement\\"}";
        }
        
        std::ostringstream oss;
        oss << "[";
        bool first = true;
        
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            if (!first) oss << ",";
            
            int id = sqlite3_column_int(stmt, 0);
            const unsigned char* name = sqlite3_column_text(stmt, 1);
            const unsigned char* email = sqlite3_column_text(stmt, 2);
            int age = sqlite3_column_int(stmt, 3);
            
            oss << "{\\"id\\":" << id 
                << ",\\"name\\":\\"" << name 
                << "\\",\\"email\\":\\"" << email 
                << "\\",\\"age\\":" << age << "}";
            
            first = false;
        }
        
        oss << "]";
        
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        
        return oss.str();
    }
};

const char* Task49::DB_NAME = "users.db";

int main() {
    Task49::initDatabase();
    
    std::cout << "Test Case 1: Add user John" << std::endl;
    std::cout << Task49::addUser("John Doe", "john@example.com", 30) << std::endl;
    
    std::cout << "\\nTest Case 2: Add user Jane" << std::endl;
    std::cout << Task49::addUser("Jane Smith", "jane@example.com", 25) << std::endl;
    
    std::cout << "\\nTest Case 3: Add user Bob" << std::endl;
    std::cout << Task49::addUser("Bob Johnson", "bob@example.com", 35) << std::endl;
    
    std::cout << "\\nTest Case 4: Add user Alice" << std::endl;
    std::cout << Task49::addUser("Alice Brown", "alice@example.com", 28) << std::endl;
    
    std::cout << "\\nTest Case 5: Get all users" << std::endl;
    std::cout << Task49::getUsers() << std::endl;
    
    return 0;
}

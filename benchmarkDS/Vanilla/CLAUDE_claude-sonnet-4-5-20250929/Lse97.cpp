
#include <iostream>
#include <string>
#include <sqlite3.h>
#include <openssl/sha.h>
#include <sstream>
#include <iomanip>

using namespace std;

class Lse97 {
private:
    static sqlite3* db;
    
public:
    static string hashPassword(const string& password) {
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256((unsigned char*)password.c_str(), password.length(), hash);
        
        stringstream ss;
        for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
            ss << hex << setw(2) << setfill('0') << (int)hash[i];
        }
        return ss.str();
    }
    
    static string registerUser(const string& username, const string& password, 
                              const string& firstName, const string& lastName, 
                              const string& email) {
        // Initialize database if not exists
        if (db == nullptr) {
            if (sqlite3_open("users.db", &db) != SQLITE_OK) {
                return "Database connection failed";
            }
            
            const char* createTable = "CREATE TABLE IF NOT EXISTS users (username TEXT PRIMARY KEY, password TEXT, firstName TEXT, lastName TEXT, email TEXT)";
            char* errMsg;
            if (sqlite3_exec(db, createTable, nullptr, nullptr, &errMsg) != SQLITE_OK) {
                string error = errMsg;
                sqlite3_free(errMsg);
                return "Table creation failed: " + error;
            }
        }
        
        // Check if username exists
        sqlite3_stmt* stmt;
        const char* checkQuery = "SELECT username FROM users WHERE username = ?";
        
        if (sqlite3_prepare_v2(db, checkQuery, -1, &stmt, nullptr) != SQLITE_OK) {
            return "Query preparation failed";
        }
        
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
        
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            sqlite3_finalize(stmt);
            return "Username already exists";
        }
        sqlite3_finalize(stmt);
        
        // Hash password
        string hashedPassword = hashPassword(password);
        
        // Insert user
        const char* insertQuery = "INSERT INTO users (username, password, firstName, lastName, email) VALUES (?, ?, ?, ?, ?)";
        
        if (sqlite3_prepare_v2(db, insertQuery, -1, &stmt, nullptr) != SQLITE_OK) {
            return "Insert preparation failed";
        }
        
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, hashedPassword.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, firstName.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 4, lastName.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 5, email.c_str(), -1, SQLITE_STATIC);
        
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            return "Registration failed";
        }
        
        sqlite3_finalize(stmt);
        return "Registration succeeded";
    }
    
    static void closeDatabase() {
        if (db != nullptr) {
            sqlite3_close(db);
            db = nullptr;
        }
    }
};

sqlite3* Lse97::db = nullptr;

int main() {
    // Test cases
    cout << Lse97::registerUser("john_doe", "password123", "John", "Doe", "john@example.com") << endl;
    cout << Lse97::registerUser("jane_smith", "securePass456", "Jane", "Smith", "jane@example.com") << endl;
    cout << Lse97::registerUser("john_doe", "anotherPass", "John", "Doe", "john2@example.com") << endl;
    cout << Lse97::registerUser("bob_wilson", "bobPass789", "Bob", "Wilson", "bob@example.com") << endl;
    cout << Lse97::registerUser("alice_brown", "alicePass321", "Alice", "Brown", "alice@example.com") << endl;
    
    Lse97::closeDatabase();
    return 0;
}

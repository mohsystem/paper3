
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <openssl/sha.h>
#include <sqlite3.h>

using namespace std;

string hashPassword(const string& password) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)password.c_str(), password.length(), hash);
    
    stringstream ss;
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << hex << setw(2) << setfill('0') << (int)hash[i];
    }
    return ss.str();
}

bool insertUser(const string& username, const string& password) {
    string hashedPassword = hashPassword(password);
    
    sqlite3* db;
    char* errMsg = 0;
    int rc = sqlite3_open("users.db", &db);
    
    if(rc) {
        cout << "Can't open database: " << sqlite3_errmsg(db) << endl;
        return false;
    }
    
    const char* createTableSQL = "CREATE TABLE IF NOT EXISTS users (id INTEGER PRIMARY KEY AUTOINCREMENT, username TEXT UNIQUE, password TEXT);";
    rc = sqlite3_exec(db, createTableSQL, 0, 0, &errMsg);
    
    if(rc != SQLITE_OK) {
        cout << "SQL error: " << errMsg << endl;
        sqlite3_free(errMsg);
        sqlite3_close(db);
        return false;
    }
    
    sqlite3_stmt* stmt;
    const char* insertSQL = "INSERT INTO users (username, password) VALUES (?, ?);";
    rc = sqlite3_prepare_v2(db, insertSQL, -1, &stmt, 0);
    
    if(rc != SQLITE_OK) {
        cout << "Failed to prepare statement" << endl;
        sqlite3_close(db);
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, hashedPassword.c_str(), -1, SQLITE_TRANSIENT);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    return rc == SQLITE_DONE;
}

int main() {
    // Test cases
    cout << "Test Case 1:" << endl;
    bool result1 = insertUser("user1", "password123");
    cout << "User1 inserted: " << (result1 ? "true" : "false") << endl;
    
    cout << "\\nTest Case 2:" << endl;
    bool result2 = insertUser("user2", "securePass456");
    cout << "User2 inserted: " << (result2 ? "true" : "false") << endl;
    
    cout << "\\nTest Case 3:" << endl;
    bool result3 = insertUser("user3", "mySecret789");
    cout << "User3 inserted: " << (result3 ? "true" : "false") << endl;
    
    cout << "\\nTest Case 4:" << endl;
    bool result4 = insertUser("user4", "testPass000");
    cout << "User4 inserted: " << (result4 ? "true" : "false") << endl;
    
    cout << "\\nTest Case 5:" << endl;
    bool result5 = insertUser("user5", "finalTest111");
    cout << "User5 inserted: " << (result5 ? "true" : "false") << endl;
    
    return 0;
}

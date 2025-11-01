
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <sqlite3.h>
#include <openssl/sha.h>

using namespace std;

class Lse99 {
public:
    static string hashPassword(const string& password) {
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256((unsigned char*)password.c_str(), password.length(), hash);
        
        stringstream ss;
        for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
            ss << hex << setw(2) << setfill('0') << (int)hash[i];
        }
        return ss.str();
    }
    
    static bool insertUser(const string& username, const string& password) {
        string hashedPassword = hashPassword(password);
        sqlite3* db;
        char* errMsg = 0;
        
        int rc = sqlite3_open("users.db", &db);
        if(rc) {
            cout << "Error opening database: " << sqlite3_errmsg(db) << endl;
            return false;
        }
        
        string createTableSQL = "CREATE TABLE IF NOT EXISTS users (id INTEGER PRIMARY KEY AUTOINCREMENT, username TEXT UNIQUE, password TEXT);";
        rc = sqlite3_exec(db, createTableSQL.c_str(), 0, 0, &errMsg);
        
        string insertSQL = "INSERT INTO users (username, password) VALUES ('" + username + "', '" + hashedPassword + "');";
        rc = sqlite3_exec(db, insertSQL.c_str(), 0, 0, &errMsg);
        
        if(rc != SQLITE_OK) {
            cout << "Error: " << errMsg << endl;
            sqlite3_free(errMsg);
            sqlite3_close(db);
            return false;
        }
        
        sqlite3_close(db);
        return true;
    }
};

int main() {
    cout << "Test Case 1:" << endl;
    cout << (Lse99::insertUser("user1", "password123") ? "Success" : "Failed") << endl;
    
    cout << "\\nTest Case 2:" << endl;
    cout << (Lse99::insertUser("user2", "securePass456") ? "Success" : "Failed") << endl;
    
    cout << "\\nTest Case 3:" << endl;
    cout << (Lse99::insertUser("admin", "admin@2024") ? "Success" : "Failed") << endl;
    
    cout << "\\nTest Case 4:" << endl;
    cout << (Lse99::insertUser("testuser", "test1234") ? "Success" : "Failed") << endl;
    
    cout << "\\nTest Case 5:" << endl;
    cout << (Lse99::insertUser("john_doe", "mySecret99") ? "Success" : "Failed") << endl;
    
    return 0;
}

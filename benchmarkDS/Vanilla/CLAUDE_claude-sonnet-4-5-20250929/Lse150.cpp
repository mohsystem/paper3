
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <ctime>
#include <sstream>
#include <sqlite3.h>

using namespace std;

class Lse150 {
private:
    sqlite3* db;
    
    string getCurrentTimestamp() {
        time_t now = time(0);
        char buffer[80];
        strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", localtime(&now));
        return string(buffer);
    }
    
public:
    Lse150() {
        // Open database connection
        int rc = sqlite3_open("messages.db", &db);
        if (rc) {
            cerr << "Can't open database: " << sqlite3_errmsg(db) << endl;
        }
        
        // Create table if not exists
        const char* sql = "CREATE TABLE IF NOT EXISTS messages ("
                         "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                         "username TEXT NOT NULL,"
                         "message TEXT NOT NULL,"
                         "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP);";
        
        char* errMsg;
        rc = sqlite3_exec(db, sql, 0, 0, &errMsg);
        if (rc != SQLITE_OK) {
            cerr << "SQL error: " << errMsg << endl;
            sqlite3_free(errMsg);
        }
    }
    
    ~Lse150() {
        sqlite3_close(db);
    }
    
    void insert_user_message_in_db(const string& username, const string& message) {
        string sql = "INSERT INTO messages (username, message) VALUES (?, ?);";
        sqlite3_stmt* stmt;
        
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 2, message.c_str(), -1, SQLITE_TRANSIENT);
            
            if (sqlite3_step(stmt) != SQLITE_DONE) {
                cerr << "Error inserting data" << endl;
            }
            sqlite3_finalize(stmt);
        }
    }
    
    string post(map<string, string>& request) {
        // Get username and message from request
        string username = request["username"];
        string message = request["message"];
        
        // Insert message into database
        if (!username.empty() && !message.empty()) {
            insert_user_message_in_db(username, message);
        }
        
        // Return redirect instruction
        return "redirect:/";
    }
    
    vector<tuple<string, string, string>> getMessages() {
        vector<tuple<string, string, string>> messages;
        const char* sql = "SELECT username, message, timestamp FROM messages ORDER BY timestamp DESC LIMIT 10;";
        sqlite3_stmt* stmt;
        
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) {
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                string username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
                string message = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
                string timestamp = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
                messages.push_back(make_tuple(username, message, timestamp));
            }
            sqlite3_finalize(stmt);
        }
        
        return messages;
    }
    
    void displayMainPage() {
        cout << "<!DOCTYPE html>" << endl;
        cout << "<html><head><title>Message Board</title></head>" << endl;
        cout << "<body>" << endl;
        cout << "<h1>Message Board</h1>" << endl;
        cout << "<form method='post' action='/post'>" << endl;
        cout << "<label>Username: <input type='text' name='username' required></label><br><br>" << endl;
        cout << "<label>Message: <textarea name='message' required></textarea></label><br><br>" << endl;
        cout << "<input type='submit' value='Submit Message'>" << endl;
        cout << "</form>" << endl;
        cout << "<h2>Recent Messages:</h2>" << endl;
        
        auto messages = getMessages();
        for (const auto& msg : messages) {
            cout << "<p><strong>" << get<0>(msg) << "</strong>: " 
                 << get<1>(msg) << " <em>(" << get<2>(msg) << ")</em></p>" << endl;
        }
        
        cout << "</body></html>" << endl;
    }
};

int main() {
    Lse150 app;
    
    cout << "Running test cases..." << endl;
    
    // Test case 1
    map<string, string> request1 = {{"username", "John"}, {"message", "Hello World"}};
    string result1 = app.post(request1);
    cout << "Test 1: " << result1 << endl;
    
    // Test case 2
    map<string, string> request2 = {{"username", "Alice"}, {"message", "Test message"}};
    string result2 = app.post(request2);
    cout << "Test 2: " << result2 << endl;
    
    // Test case 3
    map<string, string> request3 = {{"username", "Bob"}, {"message", "Another test"}};
    string result3 = app.post(request3);
    cout << "Test 3: " << result3 << endl;
    
    // Test case 4
    map<string, string> request4 = {{"username", "Charlie"}, {"message", "Fourth message"}};
    string result4 = app.post(request4);
    cout << "Test 4: " << result4 << endl;
    
    // Test case 5
    map<string, string> request5 = {{"username", "David"}, {"message", "Last test message"}};
    string result5 = app.post(request5);
    cout << "Test 5: " << result5 << endl;
    
    cout << "\\nDisplaying messages:" << endl;
    auto messages = app.getMessages();
    for (const auto& msg : messages) {
        cout << get<0>(msg) << ": " << get<1>(msg) << " (" << get<2>(msg) << ")" << endl;
    }
    
    return 0;
}

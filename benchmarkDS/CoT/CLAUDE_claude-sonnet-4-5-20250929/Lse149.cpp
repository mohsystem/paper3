
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sqlite3.h>
#include <microhttpd.h>
#include <cstring>
#include <sstream>
#include <algorithm>

class Lse149 {
private:
    sqlite3* db;
    
    std::string escapeHtml(const std::string& text) {
        std::string result;
        for (char c : text) {
            switch (c) {
                case '&': result += "&amp;"; break;
                case '<': result += "&lt;"; break;
                case '>': result += "&gt;"; break;
                case '"': result += "&quot;"; break;
                case '\\'': result += "&#x27;"; break;
                default: result += c;
            }
        }
        return result;
    }
    
    std::string urlDecode(const std::string& str) {
        std::string result;
        for (size_t i = 0; i < str.length(); i++) {
            if (str[i] == '%' && i + 2 < str.length()) {
                int value;
                std::istringstream is(str.substr(i + 1, 2));
                if (is >> std::hex >> value) {
                    result += static_cast<char>(value);
                    i += 2;
                } else {
                    result += str[i];
                }
            } else if (str[i] == '+') {
                result += ' ';
            } else {
                result += str[i];
            }
        }
        return result;
    }
    
    std::map<std::string, std::string> parsePostData(const std::string& postData) {
        std::map<std::string, std::string> params;
        std::istringstream stream(postData);
        std::string pair;
        
        while (std::getline(stream, pair, '&')) {
            size_t pos = pair.find('=');
            if (pos != std::string::npos) {
                std::string key = urlDecode(pair.substr(0, pos));
                std::string value = urlDecode(pair.substr(pos + 1));
                params[key] = value;
            }
        }
        return params;
    }
    
public:
    Lse149() {
        sqlite3_open("messages.db", &db);
        const char* sql = "CREATE TABLE IF NOT EXISTS messages ("
                         "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                         "username TEXT NOT NULL, "
                         "message TEXT NOT NULL, "
                         "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP)";
        sqlite3_exec(db, sql, nullptr, nullptr, nullptr);
    }
    
    ~Lse149() {
        sqlite3_close(db);
    }
    
    bool insertUserMessageInDb(const std::string& username, const std::string& message) {
        if (username.empty() || message.empty() || username.length() > 50 || message.length() > 500) {
            return false;
        }
        
        sqlite3_stmt* stmt;
        const char* sql = "INSERT INTO messages (username, message) VALUES (?, ?)";
        
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            return false;
        }
        
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, message.c_str(), -1, SQLITE_TRANSIENT);
        
        bool success = sqlite3_step(stmt) == SQLITE_DONE;
        sqlite3_finalize(stmt);
        return success;
    }
    
    std::vector<std::map<std::string, std::string>> getAllMessages() {
        std::vector<std::map<std::string, std::string>> messages;
        sqlite3_stmt* stmt;
        const char* sql = "SELECT username, message, created_at FROM messages ORDER BY created_at DESC";
        
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                std::map<std::string, std::string> msg;
                msg["username"] = escapeHtml(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
                msg["message"] = escapeHtml(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));
                msg["created_at"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
                messages.push_back(msg);
            }
            sqlite3_finalize(stmt);
        }
        return messages;
    }
    
    std::string generateHomePage() {
        std::ostringstream html;
        html << "<html><body><h1>Messages</h1>";
        html << "<form method='post' action='/post'>";
        html << "Username: <input type='text' name='username' required maxlength='50'><br>";
        html << "Message: <textarea name='message' required maxlength='500'></textarea><br>";
        html << "<input type='submit' value='Post Message'></form><hr>";
        
        auto messages = getAllMessages();
        for (const auto& msg : messages) {
            html << "<div><strong>" << msg.at("username") << "</strong>: "
                 << msg.at("message") << " <em>(" << msg.at("created_at") << ")</em></div>";
        }
        html << "</body></html>";
        return html.str();
    }
    
    void handlePost(const std::string& postData) {
        auto params = parsePostData(postData);
        std::string username = params["username"];
        std::string message = params["message"];
        insertUserMessageInDb(username, message);
    }
};

int main() {
    Lse149 app;
    
    // Test cases
    std::cout << "\\nTest Cases:" << std::endl;
    std::cout << "1. Valid insertion: " << app.insertUserMessageInDb("Alice", "Hello World") << std::endl;
    std::cout << "2. Valid insertion: " << app.insertUserMessageInDb("Bob", "Test message") << std::endl;
    std::cout << "3. Empty username: " << app.insertUserMessageInDb("", "Message") << std::endl;
    std::cout << "4. Long username: " << app.insertUserMessageInDb(std::string(51, 'a'), "Message") << std::endl;
    std::cout << "5. XSS attempt: " << app.insertUserMessageInDb("Hacker", "<script>alert('xss')</script>") << std::endl;
    
    return 0;
}

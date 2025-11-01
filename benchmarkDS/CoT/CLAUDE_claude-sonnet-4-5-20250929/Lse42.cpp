
#include <iostream>
#include <string>
#include <map>
#include <memory>
#include <sstream>
#include <algorithm>

class Lse42 {
private:
    std::map<std::string, std::map<std::string, std::string>> sessions;
    
    std::string escapeHtml(const std::string& input) {
        std::string output;
        for (char c : input) {
            switch (c) {
                case '&': output += "&amp;"; break;
                case '<': output += "&lt;"; break;
                case '>': output += "&gt;"; break;
                case '"': output += "&quot;"; break;
                case '\\'': output += "&#x27;"; break;
                default: output += c;
            }
        }
        return output;
    }
    
    std::string maskSSN(const std::string& ssn) {
        if (ssn.empty() || ssn.length() < 4) {
            return "***-**-****";
        }
        return "***-**-" + ssn.substr(ssn.length() - 4);
    }
    
public:
    void createSession(const std::string& sessionId, const std::string& username, 
                      const std::string& email, const std::string& ssn) {
        std::map<std::string, std::string> userData;
        userData["username"] = username;
        userData["email"] = email;
        userData["ssn"] = ssn;
        sessions[sessionId] = userData;
    }
    
    std::string getInfo(const std::string& sessionId) {
        if (sessionId.empty()) {
            return "Unauthorized: No session found";
        }
        
        auto it = sessions.find(sessionId);
        if (it == sessions.end()) {
            return "Unauthorized: Invalid session";
        }
        
        auto& session = it->second;
        std::string username = session.count("username") ? session["username"] : "N/A";
        std::string email = session.count("email") ? session["email"] : "N/A";
        std::string ssn = session.count("ssn") ? session["ssn"] : "N/A";
        
        std::ostringstream oss;
        oss << "Username: " << escapeHtml(username) 
            << "; Email: " << escapeHtml(email)
            << "; SSN: " << maskSSN(ssn);
        
        return oss.str();
    }
};

int main() {
    Lse42 app;
    
    // Test case 1: Valid session
    app.createSession("session1", "john_doe", "john@example.com", "123-45-6789");
    std::cout << "Test Case 1: " << app.getInfo("session1") << std::endl;
    
    // Test case 2: Another valid session
    app.createSession("session2", "jane_smith", "jane@example.com", "987-65-4321");
    std::cout << "Test Case 2: " << app.getInfo("session2") << std::endl;
    
    // Test case 3: Invalid session
    std::cout << "Test Case 3: " << app.getInfo("invalid_session") << std::endl;
    
    // Test case 4: Empty session ID
    std::cout << "Test Case 4: " << app.getInfo("") << std::endl;
    
    // Test case 5: XSS attempt
    app.createSession("session3", "<script>alert('XSS')</script>", "test@example.com", "111-11-1111");
    std::cout << "Test Case 5: " << app.getInfo("session3") << std::endl;
    
    return 0;
}


#include <iostream>
#include <string>
#include <map>
#include <sstream>
#include <algorithm>

class Lse40 {
private:
    std::map<std::string, std::string> session;
    
    std::string escapeHtml(const std::string& input) {
        std::string output;
        for (char c : input) {
            switch (c) {
                case '&': output += "&amp;"; break;
                case '<': output += "&lt;"; break;
                case '>': output += "&gt;"; break;
                case '"': output += "&quot;"; break;
                case '\\'': output += "&#x27;"; break;
                default: output += c; break;
            }
        }
        return output;
    }
    
    std::string getLastFourDigits(const std::string& ssn) {
        if (ssn.length() < 4) {
            return "****";
        }
        return ssn.substr(ssn.length() - 4);
    }
    
public:
    void setSession(const std::string& key, const std::string& value) {
        session[key] = value;
    }
    
    std::string getSession(const std::string& key) {
        if (session.find(key) != session.end()) {
            return session[key];
        }
        return "";
    }
    
    void clearSession() {
        session.clear();
    }
    
    std::string info() {
        std::string username = getSession("username");
        std::string email = getSession("email");
        std::string ssn = getSession("ssn");
        
        if (username.empty() && email.empty() && ssn.empty()) {
            return "<html><body><h1>Error: No session data found</h1></body></html>";
        }
        
        std::string username_safe = escapeHtml(username);
        std::string email_safe = escapeHtml(email);
        std::string ssn_last4 = escapeHtml(getLastFourDigits(ssn));
        
        std::ostringstream html;
        html << "<html><head><title>User Info</title></head><body>"
             << "<h1>User Information</h1>"
             << "<p><strong>Username:</strong> " << username_safe << "</p>"
             << "<p><strong>Email:</strong> " << email_safe << "</p>"
             << "<p><strong>SSN (Last 4):</strong> " << ssn_last4 << "</p>"
             << "</body></html>";
        
        return html.str();
    }
};

int main() {
    Lse40 app;
    
    // Test case 1: Normal data
    std::cout << "Test 1:\\n";
    app.setSession("username", "john_doe");
    app.setSession("email", "john@example.com");
    app.setSession("ssn", "123456789");
    std::cout << app.info() << "\\n\\n";
    
    // Test case 2: XSS attempt
    std::cout << "Test 2 (XSS):\\n";
    app.clearSession();
    app.setSession("username", "jane<script>alert('xss')</script>");
    app.setSession("email", "jane@test.com");
    app.setSession("ssn", "987654321");
    std::cout << app.info() << "\\n\\n";
    
    // Test case 3: Short SSN
    std::cout << "Test 3 (Short SSN):\\n";
    app.clearSession();
    app.setSession("username", "bob");
    app.setSession("email", "bob@mail.com");
    app.setSession("ssn", "111");
    std::cout << app.info() << "\\n\\n";
    
    // Test case 4: Normal data
    std::cout << "Test 4:\\n";
    app.clearSession();
    app.setSession("username", "alice");
    app.setSession("email", "alice@domain.com");
    app.setSession("ssn", "555667788");
    std::cout << app.info() << "\\n\\n";
    
    // Test case 5: Empty session
    std::cout << "Test 5 (Empty):\\n";
    app.clearSession();
    std::cout << app.info() << "\\n";
    
    return 0;
}

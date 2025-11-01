
#include <iostream>
#include <string>
#include <map>
#include <random>
#include <sstream>
#include <iomanip>

class Lse52 {
private:
    std::map<std::string, std::string> userDatabase;
    std::map<std::string, std::string> sessions;
    
    std::string generateUUID() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 15);
        
        const char* hex = "0123456789abcdef";
        std::stringstream ss;
        
        for (int i = 0; i < 32; i++) {
            if (i == 8 || i == 12 || i == 16 || i == 20) ss << "-";
            ss << hex[dis(gen)];
        }
        return ss.str();
    }
    
public:
    Lse52() {
        // Predefined users for testing
        userDatabase["admin"] = "admin123";
        userDatabase["user1"] = "password1";
        userDatabase["user2"] = "password2";
        userDatabase["testuser"] = "testpass";
        userDatabase["demo"] = "demo123";
    }
    
    std::string renderLoginPage(const std::string& error = "") {
        std::stringstream html;
        html << "<html><body>";
        html << "<h2>Login Page</h2>";
        if (!error.empty()) {
            html << "<p style='color:red;'>" << error << "</p>";
        }
        html << "<form method='post' action='/login'>";
        html << "Username: <input type='text' name='username' required><br>";
        html << "Password: <input type='password' name='password' required><br>";
        html << "<input type='submit' value='Login'>";
        html << "</form>";
        html << "<a href='/'>Back to Home</a>";
        html << "</body></html>";
        return html.str();
    }
    
    std::string renderIndexPage() {
        return "<html><body><h2>Welcome to Index Page</h2>"
               "<a href='/login'>Go to Login</a></body></html>";
    }
    
    std::string renderSecretPage(const std::string& username) {
        std::stringstream html;
        html << "<html><body><h2>Secret settings</h2>";
        html << "<p>Welcome, " << username << "!</p>";
        html << "<a href='/logout'>Logout</a></body></html>";
        return html.str();
    }
    
    bool doLogin(const std::string& username, const std::string& password) {
        if (username.empty() || password.empty()) {
            return false;
        }
        auto it = userDatabase.find(username);
        if (it == userDatabase.end()) {
            return false;
        }
        return it->second == password;
    }
    
    std::string createSession(const std::string& username) {
        std::string sessionId = generateUUID();
        sessions[sessionId] = username;
        return sessionId;
    }
    
    std::string getUserFromSession(const std::string& sessionId) {
        auto it = sessions.find(sessionId);
        if (it != sessions.end()) {
            return it->second;
        }
        return "";
    }
    
    void logout(const std::string& sessionId) {
        sessions.erase(sessionId);
    }
};

int main() {
    Lse52 app;
    std::cout << "=== Login System Test Cases ===\\n\\n";
    
    // Test Case 1: Valid login
    std::cout << "Test Case 1: Valid login\\n";
    bool result1 = app.doLogin("admin", "admin123");
    std::cout << "Result: " << (result1 ? "SUCCESS" : "FAILED") << "\\n";
    if (result1) {
        std::string session1 = app.createSession("admin");
        std::cout << "Session created: " << session1 << "\\n";
        std::cout << app.renderSecretPage("admin") << "\\n";
    }
    std::cout << "\\n";
    
    // Test Case 2: Invalid password
    std::cout << "Test Case 2: Invalid password\\n";
    bool result2 = app.doLogin("admin", "wrongpass");
    std::cout << "Result: " << (result2 ? "SUCCESS" : "FAILED") << "\\n\\n";
    
    // Test Case 3: Non-existent user
    std::cout << "Test Case 3: Non-existent user\\n";
    bool result3 = app.doLogin("nonexistent", "password");
    std::cout << "Result: " << (result3 ? "SUCCESS" : "FAILED") << "\\n\\n";
    
    // Test Case 4: Valid login with different user
    std::cout << "Test Case 4: Valid login with user1\\n";
    bool result4 = app.doLogin("user1", "password1");
    std::cout << "Result: " << (result4 ? "SUCCESS" : "FAILED") << "\\n";
    if (result4) {
        std::string session4 = app.createSession("user1");
        std::cout << "Session created: " << session4 << "\\n";
    }
    std::cout << "\\n";
    
    // Test Case 5: Empty credentials
    std::cout << "Test Case 5: Empty credentials\\n";
    bool result5 = app.doLogin("", "");
    std::cout << "Result: " << (result5 ? "SUCCESS" : "FAILED") << "\\n\\n";
    
    // Display pages
    std::cout << "=== Page Renders ===\\n\\n";
    std::cout << "Index Page:\\n" << app.renderIndexPage() << "\\n\\n";
    std::cout << "Login Page:\\n" << app.renderLoginPage() << "\\n";
    
    return 0;
}


#include <iostream>
#include <string>
#include <map>
#include <random>
#include <sstream>
#include <iomanip>
#include <regex>
#include <ctime>

class Task40 {
private:
    struct UserSettings {
        std::string username;
        std::string email;
        std::string theme;
        bool notifications;
        
        UserSettings(const std::string& u = "", const std::string& e = "", 
                    const std::string& t = "light", bool n = true)
            : username(u), email(e), theme(t), notifications(n) {}
        
        std::string toString() const {
            std::ostringstream oss;
            oss << "UserSettings{username='" << username 
                << "', email='" << email 
                << "', theme='" << theme 
                << "', notifications=" << std::boolalpha << notifications << "}";
            return oss.str();
        }
    };
    
    std::map<std::string, std::string> csrfTokenStore;
    std::map<std::string, UserSettings> userDatabase;
    std::mt19937 rng;
    
    std::string generateRandomToken() {
        std::uniform_int_distribution<int> dist(0, 255);
        std::ostringstream oss;
        for (int i = 0; i < 32; ++i) {
            oss << std::hex << std::setw(2) << std::setfill('0') << dist(rng);
        }
        return oss.str();
    }
    
public:
    Task40() {
        rng.seed(static_cast<unsigned int>(std::time(nullptr)));
    }
    
    std::string generateCSRFToken(const std::string& sessionId) {
        std::string token = generateRandomToken();
        csrfTokenStore[sessionId] = token;
        return token;
    }
    
    bool validateCSRFToken(const std::string& sessionId, const std::string& token) {
        if (sessionId.empty() || token.empty()) {
            return false;
        }
        auto it = csrfTokenStore.find(sessionId);
        return it != csrfTokenStore.end() && it->second == token;
    }
    
    std::string getCSRFTokenForSession(const std::string& sessionId) {
        auto it = csrfTokenStore.find(sessionId);
        if (it != csrfTokenStore.end()) {
            return it->second;
        }
        return generateCSRFToken(sessionId);
    }
    
    std::string updateUserSettings(const std::string& sessionId, const std::string& csrfToken,
                                   const std::string& username, const std::string& email,
                                   const std::string& theme, bool notifications) {
        // Validate CSRF token
        if (!validateCSRFToken(sessionId, csrfToken)) {
            return "ERROR: Invalid CSRF token. Request rejected.";
        }
        
        // Validate input
        if (username.empty() || username.find_first_not_of(" \\t\\n\\r") == std::string::npos) {
            return "ERROR: Username cannot be empty.";
        }
        
        std::regex emailPattern("^[A-Za-z0-9+_.-]+@(.+)$");
        if (email.empty() || !std::regex_match(email, emailPattern)) {
            return "ERROR: Invalid email format.";
        }
        
        // Update user settings
        UserSettings settings(username, email, theme, notifications);
        userDatabase[sessionId] = settings;
        
        // Generate new CSRF token after successful update
        generateCSRFToken(sessionId);
        
        return "SUCCESS: User settings updated successfully. " + settings.toString();
    }
    
    std::string generateSettingsForm(const std::string& sessionId) {
        std::string csrfToken = getCSRFTokenForSession(sessionId);
        UserSettings currentSettings;
        auto it = userDatabase.find(sessionId);
        if (it != userDatabase.end()) {
            currentSettings = it->second;
        }
        
        std::ostringstream html;
        html << "<!DOCTYPE html>\\n<html>\\n<head>\\n";
        html << "<title>User Settings</title>\\n";
        html << "<style>body{font-family:Arial;padding:20px;}</style>\\n";
        html << "</head>\\n<body>\\n";
        html << "<h2>Update User Settings</h2>\\n";
        html << "<form method='POST' action='/update-settings'>\\n";
        html << "  <input type='hidden' name='csrf_token' value='" << csrfToken << "'>\\n";
        html << "  <label>Username:</label><br>\\n";
        html << "  <input type='text' name='username' value='" << currentSettings.username << "' required><br><br>\\n";
        html << "  <label>Email:</label><br>\\n";
        html << "  <input type='email' name='email' value='" << currentSettings.email << "' required><br><br>\\n";
        html << "  <label>Theme:</label><br>\\n";
        html << "  <select name='theme'>\\n";
        html << "    <option value='light'" << (currentSettings.theme == "light" ? " selected" : "") << ">Light</option>\\n";
        html << "    <option value='dark'" << (currentSettings.theme == "dark" ? " selected" : "") << ">Dark</option>\\n";
        html << "  </select><br><br>\\n";
        html << "  <label><input type='checkbox' name='notifications'" << (currentSettings.notifications ? " checked" : "") << "> Enable Notifications</label><br><br>\\n";
        html << "  <button type='submit'>Update Settings</button>\\n";
        html << "</form>\\n</body>\\n</html>";
        
        return html.str();
    }
};

int main() {
    Task40 app;
    std::cout << "=== Web Application CSRF Protection Test Cases ===\\n\\n";
    
    // Test Case 1: Valid CSRF token - should succeed
    std::cout << "Test Case 1: Valid CSRF token\\n";
    std::string session1 = "session_user1";
    std::string token1 = app.generateCSRFToken(session1);
    std::string result1 = app.updateUserSettings(session1, token1, "john_doe", "john@example.com", "dark", true);
    std::cout << result1 << "\\n\\n";
    
    // Test Case 2: Invalid CSRF token - should fail
    std::cout << "Test Case 2: Invalid CSRF token\\n";
    std::string session2 = "session_user2";
    app.generateCSRFToken(session2);
    std::string result2 = app.updateUserSettings(session2, "invalid_token_12345", "jane_doe", "jane@example.com", "light", false);
    std::cout << result2 << "\\n\\n";
    
    // Test Case 3: Missing CSRF token - should fail
    std::cout << "Test Case 3: Missing CSRF token\\n";
    std::string session3 = "session_user3";
    app.generateCSRFToken(session3);
    std::string result3 = app.updateUserSettings(session3, "", "alice", "alice@example.com", "dark", true);
    std::cout << result3 << "\\n\\n";
    
    // Test Case 4: Invalid email format - should fail
    std::cout << "Test Case 4: Invalid email format\\n";
    std::string session4 = "session_user4";
    std::string token4 = app.generateCSRFToken(session4);
    std::string result4 = app.updateUserSettings(session4, token4, "bob", "invalid-email", "light", false);
    std::cout << result4 << "\\n\\n";
    
    // Test Case 5: Valid update with all fields
    std::cout << "Test Case 5: Valid update with all fields\\n";
    std::string session5 = "session_user5";
    std::string token5 = app.generateCSRFToken(session5);
    std::string result5 = app.updateUserSettings(session5, token5, "charlie_brown", "charlie@example.com", "light", true);
    std::cout << result5 << "\\n\\n";
    
    // Demonstrate form generation
    std::cout << "Test Case 6: Generate Settings Form HTML\\n";
    std::string formHtml = app.generateSettingsForm(session1);
    std::cout << "Form generated (first 200 chars): " << formHtml.substr(0, 200) << "...\\n";
    
    return 0;
}

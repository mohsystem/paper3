
#include <iostream>
#include <string>
#include <map>
#include <random>
#include <sstream>
#include <iomanip>
#include <ctime>

using namespace std;

struct UserSettings {
    string username;
    string email;
    string theme;
    bool notifications;
    
    UserSettings(string u = "john_doe", string e = "john@example.com", 
                 string t = "light", bool n = true)
        : username(u), email(e), theme(t), notifications(n) {}
    
    string toString() const {
        return "UserSettings{username='" + username + "', email='" + email + 
               "', theme='" + theme + "', notifications=" + 
               (notifications ? "true" : "false") + "}";
    }
};

class Task40 {
private:
    map<string, string> csrfTokens;
    map<string, UserSettings> userDatabase;
    mt19937 rng;
    
    string generateRandomToken() {
        uniform_int_distribution<int> dist(0, 255);
        stringstream ss;
        for (int i = 0; i < 32; i++) {
            ss << hex << setfill('0') << setw(2) << dist(rng);
        }
        return ss.str();
    }
    
public:
    Task40() {
        rng.seed(time(nullptr));
    }
    
    string generateCSRFToken(const string& sessionId) {
        string token = generateRandomToken();
        csrfTokens[sessionId] = token;
        return token;
    }
    
    bool validateCSRFToken(const string& sessionId, const string& token) {
        auto it = csrfTokens.find(sessionId);
        return it != csrfTokens.end() && it->second == token;
    }
    
    string generateSettingsForm(const string& sessionId, const UserSettings& settings) {
        string csrfToken = generateCSRFToken(sessionId);
        
        stringstream html;
        html << "<!DOCTYPE html>\\n<html>\\n<head>\\n";
        html << "<title>User Settings</title>\\n";
        html << "<style>body{font-family:Arial;margin:20px;}";
        html << "form{max-width:400px;}input,select{width:100%;padding:8px;margin:5px 0;}</style>\\n";
        html << "</head>\\n<body>\\n";
        html << "<h1>Update User Settings</h1>\\n";
        html << "<form method='POST' action='/update-settings'>\\n";
        html << "<input type='hidden' name='csrf_token' value='" << csrfToken << "'>\\n";
        html << "<label>Username:</label>\\n";
        html << "<input type='text' name='username' value='" << settings.username << "' required><br>\\n";
        html << "<label>Email:</label>\\n";
        html << "<input type='email' name='email' value='" << settings.email << "' required><br>\\n";
        html << "<label>Theme:</label>\\n";
        html << "<select name='theme'>\\n";
        html << "<option value='light'" << (settings.theme == "light" ? " selected" : "") << ">Light</option>\\n";
        html << "<option value='dark'" << (settings.theme == "dark" ? " selected" : "") << ">Dark</option>\\n";
        html << "</select><br>\\n";
        html << "<label><input type='checkbox' name='notifications' " 
             << (settings.notifications ? "checked" : "") << "> Enable Notifications</label><br>\\n";
        html << "<input type='submit' value='Update Settings'>\\n";
        html << "</form>\\n</body>\\n</html>";
        
        return html.str();
    }
    
    string processSettingsUpdate(const string& sessionId, const string& csrfToken,
                                 const string& username, const string& email,
                                 const string& theme, bool notifications) {
        if (!validateCSRFToken(sessionId, csrfToken)) {
            return "ERROR: Invalid CSRF token. Possible CSRF attack detected!";
        }
        
        UserSettings settings(username, email, theme, notifications);
        userDatabase[sessionId] = settings;
        csrfTokens.erase(sessionId); // Invalidate token after use
        
        return "SUCCESS: Settings updated successfully - " + settings.toString();
    }
    
    UserSettings getUserSettings(const string& sessionId) {
        auto it = userDatabase.find(sessionId);
        if (it != userDatabase.end()) {
            return it->second;
        }
        return UserSettings();
    }
};

int main() {
    cout << "=== CSRF Protection Demo - Web Application ===\\n\\n";
    Task40 app;
    
    // Test Case 1: Generate form for new session
    cout << "Test Case 1: Generate form with CSRF token\\n";
    string session1 = "session_" + to_string(time(nullptr));
    UserSettings settings1 = app.getUserSettings(session1);
    string form = app.generateSettingsForm(session1, settings1);
    cout << "Form generated with CSRF token (showing first 200 chars):\\n";
    cout << form.substr(0, min((size_t)200, form.length())) << "...\\n\\n";
    
    // Test Case 2: Valid update with correct CSRF token
    cout << "Test Case 2: Valid update with correct CSRF token\\n";
    string session2 = "session_user2";
    string token2 = app.generateCSRFToken(session2);
    string result2 = app.processSettingsUpdate(session2, token2, "alice",
                                               "alice@example.com", "dark", false);
    cout << result2 << "\\n\\n";
    
    // Test Case 3: Invalid update with wrong CSRF token
    cout << "Test Case 3: Invalid update with wrong CSRF token\\n";
    string session3 = "session_user3";
    app.generateCSRFToken(session3);
    string result3 = app.processSettingsUpdate(session3, "invalid_token", "bob",
                                               "bob@example.com", "light", true);
    cout << result3 << "\\n\\n";
    
    // Test Case 4: Token reuse attempt
    cout << "Test Case 4: Token reuse attempt\\n";
    string session4 = "session_user4";
    string token4 = app.generateCSRFToken(session4);
    app.processSettingsUpdate(session4, token4, "charlie", "charlie@example.com", "dark", true);
    string result4 = app.processSettingsUpdate(session4, token4, "charlie_changed",
                                               "charlie2@example.com", "light", false);
    cout << result4 << "\\n\\n";
    
    // Test Case 5: Multiple users with different tokens
    cout << "Test Case 5: Multiple concurrent sessions\\n";
    string session5a = "session_user5a";
    string session5b = "session_user5b";
    string token5a = app.generateCSRFToken(session5a);
    string token5b = app.generateCSRFToken(session5b);
    string result5a = app.processSettingsUpdate(session5a, token5a, "david",
                                                "david@example.com", "light", true);
    string result5b = app.processSettingsUpdate(session5b, token5b, "eve",
                                                "eve@example.com", "dark", false);
    cout << "User A: " << result5a << "\\n";
    cout << "User B: " << result5b << "\\n";
    
    return 0;
}

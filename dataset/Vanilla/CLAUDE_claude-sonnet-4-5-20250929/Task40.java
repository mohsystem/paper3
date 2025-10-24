
import java.io.*;
import java.security.SecureRandom;
import java.util.*;
import javax.servlet.*;
import javax.servlet.http.*;

// Note: This is a simplified example showing CSRF protection concepts
// For a full web application, you would need a servlet container like Tomcat

class Task40 {
    private static Map<String, String> csrfTokens = new HashMap<>();
    private static Map<String, UserSettings> userDatabase = new HashMap<>();
    private static SecureRandom secureRandom = new SecureRandom();
    
    static class UserSettings {
        String username;
        String email;
        String theme;
        boolean notifications;
        
        public UserSettings(String username, String email, String theme, boolean notifications) {
            this.username = username;
            this.email = email;
            this.theme = theme;
            this.notifications = notifications;
        }
        
        @Override
        public String toString() {
            return "UserSettings{username='" + username + "', email='" + email + 
                   "', theme='" + theme + "', notifications=" + notifications + "}";
        }
    }
    
    // Generate CSRF token for session
    public static String generateCSRFToken(String sessionId) {
        byte[] tokenBytes = new byte[32];
        secureRandom.nextBytes(tokenBytes);
        String token = Base64.getEncoder().encodeToString(tokenBytes);
        csrfTokens.put(sessionId, token);
        return token;
    }
    
    // Validate CSRF token
    public static boolean validateCSRFToken(String sessionId, String token) {
        String storedToken = csrfTokens.get(sessionId);
        return storedToken != null && storedToken.equals(token);
    }
    
    // Generate HTML form with CSRF token
    public static String generateSettingsForm(String sessionId, UserSettings settings) {
        String csrfToken = generateCSRFToken(sessionId);
        StringBuilder html = new StringBuilder();
        html.append("<!DOCTYPE html>\\n<html>\\n<head>\\n");
        html.append("<title>User Settings</title>\\n");
        html.append("<style>body{font-family:Arial;margin:20px;}");
        html.append("form{max-width:400px;}input,select{width:100%;padding:8px;margin:5px 0;}</style>\\n");
        html.append("</head>\\n<body>\\n");
        html.append("<h1>Update User Settings</h1>\\n");
        html.append("<form method='POST' action='/update-settings'>\\n");
        html.append("<input type='hidden' name='csrf_token' value='" + csrfToken + "'>\\n");
        html.append("<label>Username:</label>\\n");
        html.append("<input type='text' name='username' value='" + settings.username + "' required><br>\\n");
        html.append("<label>Email:</label>\\n");
        html.append("<input type='email' name='email' value='" + settings.email + "' required><br>\\n");
        html.append("<label>Theme:</label>\\n");
        html.append("<select name='theme'>\\n");
        html.append("<option value='light'" + (settings.theme.equals("light") ? " selected" : "") + ">Light</option>\\n");
        html.append("<option value='dark'" + (settings.theme.equals("dark") ? " selected" : "") + ">Dark</option>\\n");
        html.append("</select><br>\\n");
        html.append("<label><input type='checkbox' name='notifications' " + 
                    (settings.notifications ? "checked" : "") + "> Enable Notifications</label><br>\\n");
        html.append("<input type='submit' value='Update Settings'>\\n");
        html.append("</form>\\n</body>\\n</html>");
        return html.toString();
    }
    
    // Process form submission with CSRF validation
    public static String processSettingsUpdate(String sessionId, String csrfToken, 
                                               String username, String email, 
                                               String theme, boolean notifications) {
        if (!validateCSRFToken(sessionId, csrfToken)) {
            return "ERROR: Invalid CSRF token. Possible CSRF attack detected!";
        }
        
        UserSettings settings = new UserSettings(username, email, theme, notifications);
        userDatabase.put(sessionId, settings);
        csrfTokens.remove(sessionId); // Invalidate token after use
        
        return "SUCCESS: Settings updated successfully - " + settings.toString();
    }
    
    // Get user settings
    public static UserSettings getUserSettings(String sessionId) {
        return userDatabase.getOrDefault(sessionId, 
            new UserSettings("john_doe", "john@example.com", "light", true));
    }
    
    public static void main(String[] args) {
        System.out.println("=== CSRF Protection Demo - Web Application ===\\n");
        
        // Test Case 1: Generate form for new session
        System.out.println("Test Case 1: Generate form with CSRF token");
        String session1 = "session_" + System.currentTimeMillis();
        UserSettings settings1 = getUserSettings(session1);
        String form = generateSettingsForm(session1, settings1);
        System.out.println("Form generated with CSRF token (showing first 200 chars):");
        System.out.println(form.substring(0, Math.min(200, form.length())) + "...\\n");
        
        // Test Case 2: Valid update with correct CSRF token
        System.out.println("Test Case 2: Valid update with correct CSRF token");
        String session2 = "session_user2";
        String token2 = generateCSRFToken(session2);
        String result2 = processSettingsUpdate(session2, token2, "alice", 
                                               "alice@example.com", "dark", false);
        System.out.println(result2 + "\\n");
        
        // Test Case 3: Invalid update with wrong CSRF token
        System.out.println("Test Case 3: Invalid update with wrong CSRF token");
        String session3 = "session_user3";
        generateCSRFToken(session3);
        String result3 = processSettingsUpdate(session3, "invalid_token", "bob", 
                                               "bob@example.com", "light", true);
        System.out.println(result3 + "\\n");
        
        // Test Case 4: Token reuse attempt
        System.out.println("Test Case 4: Token reuse attempt");
        String session4 = "session_user4";
        String token4 = generateCSRFToken(session4);
        processSettingsUpdate(session4, token4, "charlie", "charlie@example.com", "dark", true);
        String result4 = processSettingsUpdate(session4, token4, "charlie_changed", 
                                               "charlie2@example.com", "light", false);
        System.out.println(result4 + "\\n");
        
        // Test Case 5: Multiple users with different tokens
        System.out.println("Test Case 5: Multiple concurrent sessions");
        String session5a = "session_user5a";
        String session5b = "session_user5b";
        String token5a = generateCSRFToken(session5a);
        String token5b = generateCSRFToken(session5b);
        String result5a = processSettingsUpdate(session5a, token5a, "david", 
                                                "david@example.com", "light", true);
        String result5b = processSettingsUpdate(session5b, token5b, "eve", 
                                                "eve@example.com", "dark", false);
        System.out.println("User A: " + result5a);
        System.out.println("User B: " + result5b);
    }
}

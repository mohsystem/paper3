
import java.io.*;
import java.security.SecureRandom;
import java.util.*;
import javax.servlet.*;
import javax.servlet.http.*;
import java.security.MessageDigest;
import java.nio.charset.StandardCharsets;

public class Task40 {
    private static Map<String, String> csrfTokenStore = new HashMap<>();
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
    
    // Generate CSRF token
    public static String generateCSRFToken(String sessionId) {
        byte[] randomBytes = new byte[32];
        secureRandom.nextBytes(randomBytes);
        String token = Base64.getEncoder().encodeToString(randomBytes);
        csrfTokenStore.put(sessionId, token);
        return token;
    }
    
    // Validate CSRF token
    public static boolean validateCSRFToken(String sessionId, String token) {
        if (sessionId == null || token == null) {
            return false;
        }
        String storedToken = csrfTokenStore.get(sessionId);
        return storedToken != null && storedToken.equals(token);
    }
    
    // Simulate getting CSRF token for form rendering
    public static String getCSRFTokenForSession(String sessionId) {
        return csrfTokenStore.getOrDefault(sessionId, generateCSRFToken(sessionId));
    }
    
    // Update user settings with CSRF protection
    public static String updateUserSettings(String sessionId, String csrfToken, 
                                           String username, String email, 
                                           String theme, boolean notifications) {
        // Validate CSRF token
        if (!validateCSRFToken(sessionId, csrfToken)) {
            return "ERROR: Invalid CSRF token. Request rejected.";
        }
        
        // Validate input
        if (username == null || username.trim().isEmpty()) {
            return "ERROR: Username cannot be empty.";
        }
        
        if (email == null || !email.matches("^[A-Za-z0-9+_.-]+@(.+)$")) {
            return "ERROR: Invalid email format.";
        }
        
        // Update user settings
        UserSettings settings = new UserSettings(username, email, theme, notifications);
        userDatabase.put(sessionId, settings);
        
        // Generate new CSRF token after successful update
        generateCSRFToken(sessionId);
        
        return "SUCCESS: User settings updated successfully. " + settings.toString();
    }
    
    // Generate HTML form with CSRF token
    public static String generateSettingsForm(String sessionId) {
        String csrfToken = getCSRFTokenForSession(sessionId);
        UserSettings currentSettings = userDatabase.getOrDefault(sessionId, 
            new UserSettings("", "", "light", true));
        
        StringBuilder html = new StringBuilder();
        html.append("<!DOCTYPE html>\\n<html>\\n<head>\\n");
        html.append("<title>User Settings</title>\\n");
        html.append("<style>body{font-family:Arial;padding:20px;}</style>\\n");
        html.append("</head>\\n<body>\\n");
        html.append("<h2>Update User Settings</h2>\\n");
        html.append("<form method='POST' action='/update-settings'>\\n");
        html.append("  <input type='hidden' name='csrf_token' value='" + csrfToken + "'>\\n");
        html.append("  <label>Username:</label><br>\\n");
        html.append("  <input type='text' name='username' value='" + currentSettings.username + "' required><br><br>\\n");
        html.append("  <label>Email:</label><br>\\n");
        html.append("  <input type='email' name='email' value='" + currentSettings.email + "' required><br><br>\\n");
        html.append("  <label>Theme:</label><br>\\n");
        html.append("  <select name='theme'>\\n");
        html.append("    <option value='light'" + (currentSettings.theme.equals("light") ? " selected" : "") + ">Light</option>\\n");
        html.append("    <option value='dark'" + (currentSettings.theme.equals("dark") ? " selected" : "") + ">Dark</option>\\n");
        html.append("  </select><br><br>\\n");
        html.append("  <label><input type='checkbox' name='notifications'" + (currentSettings.notifications ? " checked" : "") + "> Enable Notifications</label><br><br>\\n");
        html.append("  <button type='submit'>Update Settings</button>\\n");
        html.append("</form>\\n</body>\\n</html>");
        
        return html.toString();
    }
    
    public static void main(String[] args) {
        System.out.println("=== Web Application CSRF Protection Test Cases ===\\n");
        
        // Test Case 1: Valid CSRF token - should succeed
        System.out.println("Test Case 1: Valid CSRF token");
        String session1 = "session_user1";
        String token1 = generateCSRFToken(session1);
        String result1 = updateUserSettings(session1, token1, "john_doe", "john@example.com", "dark", true);
        System.out.println(result1);
        System.out.println();
        
        // Test Case 2: Invalid CSRF token - should fail
        System.out.println("Test Case 2: Invalid CSRF token");
        String session2 = "session_user2";
        generateCSRFToken(session2);
        String result2 = updateUserSettings(session2, "invalid_token_12345", "jane_doe", "jane@example.com", "light", false);
        System.out.println(result2);
        System.out.println();
        
        // Test Case 3: Missing CSRF token - should fail
        System.out.println("Test Case 3: Missing CSRF token");
        String session3 = "session_user3";
        generateCSRFToken(session3);
        String result3 = updateUserSettings(session3, null, "alice", "alice@example.com", "dark", true);
        System.out.println(result3);
        System.out.println();
        
        // Test Case 4: Invalid email format - should fail
        System.out.println("Test Case 4: Invalid email format");
        String session4 = "session_user4";
        String token4 = generateCSRFToken(session4);
        String result4 = updateUserSettings(session4, token4, "bob", "invalid-email", "light", false);
        System.out.println(result4);
        System.out.println();
        
        // Test Case 5: Valid update with all fields
        System.out.println("Test Case 5: Valid update with all fields");
        String session5 = "session_user5";
        String token5 = generateCSRFToken(session5);
        String result5 = updateUserSettings(session5, token5, "charlie_brown", "charlie@example.com", "light", true);
        System.out.println(result5);
        System.out.println();
        
        // Demonstrate form generation
        System.out.println("Test Case 6: Generate Settings Form HTML");
        String formHtml = generateSettingsForm(session1);
        System.out.println("Form generated (first 200 chars): " + formHtml.substring(0, Math.min(200, formHtml.length())) + "...");
    }
}

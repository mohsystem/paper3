
import java.io.*;
import javax.servlet.*;
import javax.servlet.http.*;
import java.util.*;

public class Lse52 {
    private static final Map<String, String> userDatabase = new HashMap<>();
    
    static {
        // Predefined users for testing
        userDatabase.put("admin", "admin123");
        userDatabase.put("user1", "password1");
        userDatabase.put("user2", "password2");
        userDatabase.put("testuser", "testpass");
        userDatabase.put("demo", "demo123");
    }
    
    // Session storage (in production, use proper session management)
    private static Map<String, String> sessions = new HashMap<>();
    
    public static String renderLoginPage(String error) {
        StringBuilder html = new StringBuilder();
        html.append("<html><body>");
        html.append("<h2>Login Page</h2>");
        if (error != null) {
            html.append("<p style='color:red;'>").append(error).append("</p>");
        }
        html.append("<form method='post' action='/login'>");
        html.append("Username: <input type='text' name='username' required><br>");
        html.append("Password: <input type='password' name='password' required><br>");
        html.append("<input type='submit' value='Login'>");
        html.append("</form>");
        html.append("<a href='/'>Back to Home</a>");
        html.append("</body></html>");
        return html.toString();
    }
    
    public static String renderIndexPage() {
        return "<html><body><h2>Welcome to Index Page</h2>" +
               "<a href='/login'>Go to Login</a></body></html>";
    }
    
    public static String renderSecretPage(String username) {
        return "<html><body><h2>Secret settings</h2>" +
               "<p>Welcome, " + username + "!</p>" +
               "<a href='/logout'>Logout</a></body></html>";
    }
    
    public static boolean doLogin(String username, String password) {
        if (username == null || password == null) {
            return false;
        }
        String storedPassword = userDatabase.get(username);
        return storedPassword != null && storedPassword.equals(password);
    }
    
    public static String createSession(String username) {
        String sessionId = UUID.randomUUID().toString();
        sessions.put(sessionId, username);
        return sessionId;
    }
    
    public static String getUserFromSession(String sessionId) {
        return sessions.get(sessionId);
    }
    
    public static void logout(String sessionId) {
        sessions.remove(sessionId);
    }
    
    public static void main(String[] args) {
        System.out.println("=== Login System Test Cases ===\\n");
        
        // Test Case 1: Valid login
        System.out.println("Test Case 1: Valid login");
        boolean result1 = doLogin("admin", "admin123");
        System.out.println("Result: " + (result1 ? "SUCCESS" : "FAILED"));
        if (result1) {
            String session1 = createSession("admin");
            System.out.println("Session created: " + session1);
            System.out.println(renderSecretPage("admin"));
        }
        System.out.println();
        
        // Test Case 2: Invalid password
        System.out.println("Test Case 2: Invalid password");
        boolean result2 = doLogin("admin", "wrongpass");
        System.out.println("Result: " + (result2 ? "SUCCESS" : "FAILED"));
        System.out.println();
        
        // Test Case 3: Non-existent user
        System.out.println("Test Case 3: Non-existent user");
        boolean result3 = doLogin("nonexistent", "password");
        System.out.println("Result: " + (result3 ? "SUCCESS" : "FAILED"));
        System.out.println();
        
        // Test Case 4: Valid login with different user
        System.out.println("Test Case 4: Valid login with user1");
        boolean result4 = doLogin("user1", "password1");
        System.out.println("Result: " + (result4 ? "SUCCESS" : "FAILED"));
        if (result4) {
            String session4 = createSession("user1");
            System.out.println("Session created: " + session4);
        }
        System.out.println();
        
        // Test Case 5: Null credentials
        System.out.println("Test Case 5: Null credentials");
        boolean result5 = doLogin(null, null);
        System.out.println("Result: " + (result5 ? "SUCCESS" : "FAILED"));
        System.out.println();
        
        // Display pages
        System.out.println("=== Page Renders ===\\n");
        System.out.println("Index Page:");
        System.out.println(renderIndexPage());
        System.out.println("\\nLogin Page:");
        System.out.println(renderLoginPage(null));
    }
}

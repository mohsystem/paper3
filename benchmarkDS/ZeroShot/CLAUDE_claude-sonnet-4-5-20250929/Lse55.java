
import java.util.*;
import javax.servlet.http.*;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.nio.charset.StandardCharsets;

public class Lse55 {
    // In-memory user storage (in production, use a proper database)
    private static Map<String, User> users = new HashMap<>();
    private static Map<String, String> sessions = new HashMap<>();
    
    static class User {
        String username;
        String passwordHash;
        String role;
        
        User(String username, String passwordHash, String role) {
            this.username = username;
            this.passwordHash = passwordHash;
            this.role = role;
        }
    }
    
    // Initialize with test users
    static {
        try {
            users.put("admin", new User("admin", hashPassword("admin123"), "admin"));
            users.put("user1", new User("user1", hashPassword("user123"), "user"));
            users.put("user2", new User("user2", hashPassword("pass123"), "user"));
            users.put("testuser", new User("testuser", hashPassword("test123"), "user"));
            users.put("john", new User("john", hashPassword("john123"), "user"));
        } catch (NoSuchAlgorithmException e) {
            e.printStackTrace();
        }
    }
    
    // Hash password using SHA-256
    private static String hashPassword(String password) throws NoSuchAlgorithmException {
        MessageDigest digest = MessageDigest.getInstance("SHA-256");
        byte[] hash = digest.digest(password.getBytes(StandardCharsets.UTF_8));
        StringBuilder hexString = new StringBuilder();
        for (byte b : hash) {
            String hex = Integer.toHexString(0xff & b);
            if (hex.length() == 1) hexString.append('0');
            hexString.append(hex);
        }
        return hexString.toString();
    }
    
    // GET /login - Display login page
    public static String login() {
        return "LOGIN_PAGE: Please enter username and password";
    }
    
    // POST /login - Process login
    public static String doLogin(String username, String password) {
        try {
            if (username == null || password == null || username.isEmpty() || password.isEmpty()) {
                return "ERROR: Username and password required";
            }
            
            User user = users.get(username);
            if (user == null) {
                return "ERROR: Invalid credentials";
            }
            
            String hashedPassword = hashPassword(password);
            if (!user.passwordHash.equals(hashedPassword)) {
                return "ERROR: Invalid credentials";
            }
            
            // Create session
            String sessionId = UUID.randomUUID().toString();
            sessions.put(sessionId, username);
            
            return "SUCCESS: Logged in as " + username + " | Session: " + sessionId;
        } catch (NoSuchAlgorithmException e) {
            return "ERROR: System error";
        }
    }
    
    // GET /index - Home page
    public static String index(String sessionId) {
        if (sessionId == null || !sessions.containsKey(sessionId)) {
            return "INDEX_PAGE: Welcome! Please login to continue.";
        }
        
        String username = sessions.get(sessionId);
        return "INDEX_PAGE: Welcome " + username + "! You are logged in.";
    }
    
    // GET /user - User page
    public static String userPage(String sessionId) {
        if (sessionId == null || !sessions.containsKey(sessionId)) {
            return "ERROR: Unauthorized. Please login first.";
        }
        
        String username = sessions.get(sessionId);
        User user = users.get(username);
        
        if (user == null) {
            return "ERROR: User not found";
        }
        
        return "USER_PAGE: Hello " + username + "! This is your user dashboard.";
    }
    
    // GET /admin - Admin page
    public static String adminPage(String sessionId) {
        if (sessionId == null || !sessions.containsKey(sessionId)) {
            return "ERROR: Unauthorized. Please login first.";
        }
        
        String username = sessions.get(sessionId);
        User user = users.get(username);
        
        if (user == null || !user.role.equals("admin")) {
            return "ERROR: Forbidden. Admin access required.";
        }
        
        return "ADMIN_PAGE: Welcome Admin " + username + "! You have full access.";
    }
    
    // Test cases
    public static void main(String[] args) {
        System.out.println("=== Test Case 1: Access login page ===");
        System.out.println(login());
        
        System.out.println("\\n=== Test Case 2: Successful user login ===");
        String session1 = doLogin("user1", "user123");
        System.out.println(session1);
        String sessionId1 = session1.contains("Session:") ? session1.split("Session: ")[1] : null;
        
        System.out.println("\\n=== Test Case 3: Failed login with wrong password ===");
        System.out.println(doLogin("user1", "wrongpass"));
        
        System.out.println("\\n=== Test Case 4: Access index and user page ===");
        System.out.println(index(sessionId1));
        System.out.println(userPage(sessionId1));
        
        System.out.println("\\n=== Test Case 5: Admin login and access admin page ===");
        String session2 = doLogin("admin", "admin123");
        System.out.println(session2);
        String sessionId2 = session2.contains("Session:") ? session2.split("Session: ")[1] : null;
        System.out.println(adminPage(sessionId2));
        System.out.println("User trying to access admin page:");
        System.out.println(adminPage(sessionId1));
    }
}

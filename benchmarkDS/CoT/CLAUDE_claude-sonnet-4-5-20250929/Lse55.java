
import java.io.*;
import java.util.*;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.concurrent.ConcurrentHashMap;

public class Lse55 {
    private static Map<String, User> users = new ConcurrentHashMap<>();
    private static Map<String, Session> sessions = new ConcurrentHashMap<>();
    
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
    
    static class Session {
        String username;
        String role;
        long expiryTime;
        
        Session(String username, String role) {
            this.username = username;
            this.role = role;
            this.expiryTime = System.currentTimeMillis() + 3600000; // 1 hour
        }
        
        boolean isValid() {
            return System.currentTimeMillis() < expiryTime;
        }
    }
    
    private static String hashPassword(String password) {
        try {
            MessageDigest md = MessageDigest.getInstance("SHA-256");
            byte[] hash = md.digest(password.getBytes());
            StringBuilder hexString = new StringBuilder();
            for (byte b : hash) {
                String hex = Integer.toHexString(0xff & b);
                if (hex.length() == 1) hexString.append('0');
                hexString.append(hex);
            }
            return hexString.toString();
        } catch (NoSuchAlgorithmException e) {
            throw new RuntimeException("Error hashing password", e);
        }
    }
    
    private static String generateSessionToken() {
        return UUID.randomUUID().toString();
    }
    
    public static String login() {
        return "LOGIN_PAGE: Please provide username and password";
    }
    
    public static Map<String, String> doLogin(String username, String password) {
        Map<String, String> response = new HashMap<>();
        
        if (username == null || password == null || username.trim().isEmpty() || password.isEmpty()) {
            response.put("status", "error");
            response.put("message", "Invalid credentials");
            return response;
        }
        
        User user = users.get(username.trim());
        if (user == null || !user.passwordHash.equals(hashPassword(password))) {
            response.put("status", "error");
            response.put("message", "Invalid credentials");
            return response;
        }
        
        String sessionToken = generateSessionToken();
        sessions.put(sessionToken, new Session(user.username, user.role));
        
        response.put("status", "success");
        response.put("sessionToken", sessionToken);
        response.put("message", "Login successful");
        return response;
    }
    
    public static String index() {
        return "INDEX_PAGE: Welcome to the home page";
    }
    
    public static String userPage(String sessionToken) {
        if (sessionToken == null || sessionToken.isEmpty()) {
            return "ERROR: Authentication required";
        }
        
        Session session = sessions.get(sessionToken);
        if (session == null || !session.isValid()) {
            return "ERROR: Invalid or expired session";
        }
        
        return "USER_PAGE: Welcome " + session.username + "!";
    }
    
    public static String adminPage(String sessionToken) {
        if (sessionToken == null || sessionToken.isEmpty()) {
            return "ERROR: Authentication required";
        }
        
        Session session = sessions.get(sessionToken);
        if (session == null || !session.isValid()) {
            return "ERROR: Invalid or expired session";
        }
        
        if (!"admin".equals(session.role)) {
            return "ERROR: Unauthorized - Admin access required";
        }
        
        return "ADMIN_PAGE: Welcome Admin " + session.username + "!";
    }
    
    public static void main(String[] args) {
        // Initialize test users
        users.put("user1", new User("user1", hashPassword("password123"), "user"));
        users.put("admin1", new User("admin1", hashPassword("admin123"), "admin"));
        
        System.out.println("=== Test Case 1: Login page ===");
        System.out.println(login());
        System.out.println();
        
        System.out.println("=== Test Case 2: Failed login ===");
        Map<String, String> result1 = doLogin("user1", "wrongpassword");
        System.out.println(result1);
        System.out.println();
        
        System.out.println("=== Test Case 3: Successful user login ===");
        Map<String, String> result2 = doLogin("user1", "password123");
        System.out.println(result2);
        String userToken = result2.get("sessionToken");
        System.out.println();
        
        System.out.println("=== Test Case 4: User accessing user page ===");
        System.out.println(userPage(userToken));
        System.out.println();
        
        System.out.println("=== Test Case 5: User accessing admin page (should fail) ===");
        System.out.println(adminPage(userToken));
        System.out.println();
        
        System.out.println("=== Test Case 6: Admin login and access ===");
        Map<String, String> result3 = doLogin("admin1", "admin123");
        String adminToken = result3.get("sessionToken");
        System.out.println(adminPage(adminToken));
        System.out.println();
        
        System.out.println("=== Test Case 7: Index page ===");
        System.out.println(index());
    }
}

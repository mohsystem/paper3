
import java.util.*;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

public class Lse57 {
    private static Map<String, User> users = new HashMap<>();
    private static String loggedInUser = null;
    
    static class User {
        String email;
        String passwordHash;
        
        User(String email, String password) {
            this.email = email;
            this.passwordHash = hashPassword(password);
        }
    }
    
    private static String hashPassword(String password) {
        try {
            MessageDigest md = MessageDigest.getInstance("SHA-256");
            byte[] hash = md.digest(password.toBytes());
            StringBuilder hexString = new StringBuilder();
            for (byte b : hash) {
                String hex = Integer.toHexString(0xff & b);
                if (hex.length() == 1) hexString.append('0');
                hexString.append(hex);
            }
            return hexString.toString();
        } catch (NoSuchAlgorithmException e) {
            return password;
        }
    }
    
    public static boolean register(String email, String password) {
        if (users.containsKey(email)) {
            return false;
        }
        users.put(email, new User(email, password));
        return true;
    }
    
    public static boolean login(String email, String password) {
        User user = users.get(email);
        if (user != null && user.passwordHash.equals(hashPassword(password))) {
            loggedInUser = email;
            return true;
        }
        return false;
    }
    
    public static boolean changeEmail(String oldEmail, String newEmail, String confirmPassword) {
        // Check if user is logged in
        if (loggedInUser == null) {
            return false;
        }
        
        // Check if old email matches logged in user's email\n        if (!loggedInUser.equals(oldEmail)) {\n            return false;\n        }\n        \n        // Check if old email exists\n        User user = users.get(oldEmail);\n        if (user == null) {\n            return false;\n        }\n        \n        // Verify confirm password\n        if (!user.passwordHash.equals(hashPassword(confirmPassword))) {\n            return false;\n        }\n        \n        // Check if new email already exists\n        if (users.containsKey(newEmail)) {\n            return false;\n        }\n        \n        // Change email\n        users.remove(oldEmail);\n        user.email = newEmail;\n        users.put(newEmail, user);\n        loggedInUser = newEmail;\n        \n        return true;\n    }\n    \n    public static void logout() {\n        loggedInUser = null;\n    }\n    \n    public static void main(String[] args) {\n        // Test Case 1: Successful email change\n        System.out.println("Test Case 1: Successful email change");\n        register("user1@example.com", "password123");\n        login("user1@example.com", "password123");\n        boolean result1 = changeEmail("user1@example.com", "newuser1@example.com", "password123");\n        System.out.println("Result: " + result1 + " (Expected: true)");\n        logout();\n        \n        // Test Case 2: Not logged in\n        System.out.println("\\nTest Case 2: Not logged in");\n        register("user2@example.com", "password456");\n        boolean result2 = changeEmail("user2@example.com", "newuser2@example.com", "password456");\n        System.out.println("Result: " + result2 + " (Expected: false)");\n        \n        // Test Case 3: Wrong confirm password\n        System.out.println("\\nTest Case 3: Wrong confirm password");\n        register("user3@example.com", "password789");\n        login("user3@example.com", "password789");\n        boolean result3 = changeEmail("user3@example.com", "newuser3@example.com", "wrongpassword");\n        System.out.println("Result: " + result3 + " (Expected: false)");\n        logout();\n        \n        // Test Case 4: Old email doesn't match logged in user
        System.out.println("\\nTest Case 4: Old email doesn't match logged in user");
        register("user4@example.com", "password111");
        register("user5@example.com", "password222");
        login("user4@example.com", "password111");
        boolean result4 = changeEmail("user5@example.com", "newuser5@example.com", "password222");
        System.out.println("Result: " + result4 + " (Expected: false)");
        logout();
        
        // Test Case 5: New email already exists
        System.out.println("\\nTest Case 5: New email already exists");
        register("user6@example.com", "password333");
        register("user7@example.com", "password444");
        login("user6@example.com", "password333");
        boolean result5 = changeEmail("user6@example.com", "user7@example.com", "password333");
        System.out.println("Result: " + result5 + " (Expected: false)");
        logout();
    }
}

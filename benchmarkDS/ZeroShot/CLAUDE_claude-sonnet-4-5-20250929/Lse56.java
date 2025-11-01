
import java.util.*;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

public class Lse56 {
    private static Map<String, UserAccount> userDatabase = new HashMap<>();
    private static String currentLoggedInUser = null;
    
    static class UserAccount {
        String email;
        String passwordHash;
        
        UserAccount(String email, String password) {
            this.email = email;
            this.passwordHash = hashPassword(password);
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
            throw new RuntimeException(e);
        }
    }
    
    public static boolean login(String email, String password) {
        UserAccount user = userDatabase.get(email);
        if (user != null && user.passwordHash.equals(hashPassword(password))) {
            currentLoggedInUser = email;
            return true;
        }
        return false;
    }
    
    public static void logout() {
        currentLoggedInUser = null;
    }
    
    public static boolean changeEmail(String oldEmail, String newEmail, String confirmPassword) {
        // Check if user is logged in
        if (currentLoggedInUser == null) {
            System.out.println("Error: User must be logged in to change email");
            return false;
        }
        
        // Check if old email matches current user's email\n        UserAccount currentUser = userDatabase.get(currentLoggedInUser);\n        if (!currentUser.email.equals(oldEmail)) {\n            System.out.println("Error: Old email does not match current email");\n            return false;\n        }\n        \n        // Check if new email is different from old email\n        if (oldEmail.equals(newEmail)) {\n            System.out.println("Error: New email must be different from old email");\n            return false;\n        }\n        \n        // Verify password\n        if (!currentUser.passwordHash.equals(hashPassword(confirmPassword))) {\n            System.out.println("Error: Incorrect password");\n            return false;\n        }\n        \n        // Check if new email is already taken\n        if (userDatabase.containsKey(newEmail)) {\n            System.out.println("Error: Email already exists");\n            return false;\n        }\n        \n        // Change email\n        userDatabase.remove(oldEmail);\n        currentUser.email = newEmail;\n        userDatabase.put(newEmail, currentUser);\n        currentLoggedInUser = newEmail;\n        \n        System.out.println("Email changed successfully from " + oldEmail + " to " + newEmail);\n        return true;\n    }\n    \n    public static void main(String[] args) {\n        // Test Case 1: Register users and successful email change\n        userDatabase.put("user1@example.com", new UserAccount("user1@example.com", "password123"));\n        login("user1@example.com", "password123");\n        System.out.println("Test 1: " + changeEmail("user1@example.com", "newuser1@example.com", "password123"));\n        logout();\n        \n        // Test Case 2: Attempt to change email without being logged in\n        System.out.println("\\nTest 2: " + changeEmail("newuser1@example.com", "another@example.com", "password123"));\n        \n        // Test Case 3: Incorrect password\n        userDatabase.put("user2@example.com", new UserAccount("user2@example.com", "pass456"));\n        login("user2@example.com", "pass456");\n        System.out.println("\\nTest 3: " + changeEmail("user2@example.com", "newuser2@example.com", "wrongpassword"));\n        logout();\n        \n        // Test Case 4: Old email doesn't match current email
        login("user2@example.com", "pass456");
        System.out.println("\\nTest 4: " + changeEmail("wrongemail@example.com", "newuser2@example.com", "pass456"));
        logout();
        
        // Test Case 5: New email same as old email
        login("user2@example.com", "pass456");
        System.out.println("\\nTest 5: " + changeEmail("user2@example.com", "user2@example.com", "pass456"));
        logout();
    }
}


import java.util.*;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

public class Lse57 {
    private static Map<String, User> userDatabase = new HashMap<>();
    private static User loggedInUser = null;
    
    static class User {
        private String email;
        private String passwordHash;
        
        public User(String email, String password) {
            this.email = email;
            this.passwordHash = hashPassword(password);
        }
        
        public String getEmail() {
            return email;
        }
        
        public void setEmail(String email) {
            this.email = email;
        }
        
        public boolean verifyPassword(String password) {
            return this.passwordHash.equals(hashPassword(password));
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
    }
    
    public static boolean login(String email, String password) {
        if (userDatabase.containsKey(email)) {
            User user = userDatabase.get(email);
            if (user.verifyPassword(password)) {
                loggedInUser = user;
                return true;
            }
        }
        return false;
    }
    
    public static void logout() {
        loggedInUser = null;
    }
    
    public static boolean registerUser(String email, String password) {
        if (!userDatabase.containsKey(email)) {
            User newUser = new User(email, password);
            userDatabase.put(email, newUser);
            return true;
        }
        return false;
    }
    
    public static String changeEmail(String oldEmail, String newEmail, String confirmPassword) {
        // Check if user is logged in
        if (loggedInUser == null) {
            return "Error: User not logged in";
        }
        
        // Verify old email matches logged in user's email\n        if (!loggedInUser.getEmail().equals(oldEmail)) {\n            return "Error: Old email does not match current user email";\n        }\n        \n        // Verify old email exists in database\n        if (!userDatabase.containsKey(oldEmail)) {\n            return "Error: Old email does not exist";\n        }\n        \n        // Verify confirm password is correct\n        if (!loggedInUser.verifyPassword(confirmPassword)) {\n            return "Error: Incorrect password";\n        }\n        \n        // Check if new email is already in use\n        if (userDatabase.containsKey(newEmail) && !newEmail.equals(oldEmail)) {\n            return "Error: New email already exists";\n        }\n        \n        // Validate email format\n        if (!isValidEmail(newEmail)) {\n            return "Error: Invalid email format";\n        }\n        \n        // Change email\n        userDatabase.remove(oldEmail);\n        loggedInUser.setEmail(newEmail);\n        userDatabase.put(newEmail, loggedInUser);\n        \n        return "Success: Email changed successfully";\n    }\n    \n    private static boolean isValidEmail(String email) {\n        return email != null && email.matches("^[A-Za-z0-9+_.-]+@[A-Za-z0-9.-]+\\\\.[A-Za-z]{2,}$");\n    }\n    \n    public static void main(String[] args) {\n        // Test Case 1: Register and change email successfully\n        System.out.println("Test Case 1: Successful email change");\n        registerUser("user1@example.com", "password123");\n        login("user1@example.com", "password123");\n        String result1 = changeEmail("user1@example.com", "newuser1@example.com", "password123");\n        System.out.println(result1);\n        logout();\n        System.out.println();\n        \n        // Test Case 2: Not logged in\n        System.out.println("Test Case 2: User not logged in");\n        String result2 = changeEmail("user2@example.com", "newuser2@example.com", "password123");\n        System.out.println(result2);\n        System.out.println();\n        \n        // Test Case 3: Wrong password\n        System.out.println("Test Case 3: Wrong password");\n        registerUser("user3@example.com", "password456");\n        login("user3@example.com", "password456");\n        String result3 = changeEmail("user3@example.com", "newuser3@example.com", "wrongpassword");\n        System.out.println(result3);\n        logout();\n        System.out.println();\n        \n        // Test Case 4: Old email doesn't match logged in user
        System.out.println("Test Case 4: Old email mismatch");
        registerUser("user4@example.com", "password789");
        login("user4@example.com", "password789");
        String result4 = changeEmail("different@example.com", "newuser4@example.com", "password789");
        System.out.println(result4);
        logout();
        System.out.println();
        
        // Test Case 5: New email already exists
        System.out.println("Test Case 5: New email already exists");
        registerUser("user5@example.com", "password101");
        registerUser("existing@example.com", "password102");
        login("user5@example.com", "password101");
        String result5 = changeEmail("user5@example.com", "existing@example.com", "password101");
        System.out.println(result5);
        logout();
    }
}

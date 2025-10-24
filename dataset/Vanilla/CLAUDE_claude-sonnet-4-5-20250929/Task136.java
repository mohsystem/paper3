
import java.util.*;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.nio.charset.StandardCharsets;

class Task136 {
    private Map<String, String> userDatabase;
    
    public Task136() {
        this.userDatabase = new HashMap<>();
    }
    
    public boolean registerUser(String username, String password) {
        if (username == null || password == null || username.isEmpty() || password.isEmpty()) {
            return false;
        }
        if (userDatabase.containsKey(username)) {
            return false;
        }
        String hashedPassword = hashPassword(password);
        userDatabase.put(username, hashedPassword);
        return true;
    }
    
    public boolean authenticateUser(String username, String password) {
        if (username == null || password == null || !userDatabase.containsKey(username)) {
            return false;
        }
        String hashedPassword = hashPassword(password);
        return userDatabase.get(username).equals(hashedPassword);
    }
    
    public boolean deleteUser(String username) {
        if (username == null || !userDatabase.containsKey(username)) {
            return false;
        }
        userDatabase.remove(username);
        return true;
    }
    
    public boolean changePassword(String username, String oldPassword, String newPassword) {
        if (!authenticateUser(username, oldPassword)) {
            return false;
        }
        String hashedPassword = hashPassword(newPassword);
        userDatabase.put(username, hashedPassword);
        return true;
    }
    
    private String hashPassword(String password) {
        try {
            MessageDigest digest = MessageDigest.getInstance("SHA-256");
            byte[] hash = digest.digest(password.getBytes(StandardCharsets.UTF_8));
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
    
    public static void main(String[] args) {
        Task136 auth = new Task136();
        
        // Test Case 1: Register a new user
        System.out.println("Test 1 - Register user 'john': " + auth.registerUser("john", "password123"));
        
        // Test Case 2: Authenticate with correct credentials
        System.out.println("Test 2 - Authenticate 'john' with correct password: " + auth.authenticateUser("john", "password123"));
        
        // Test Case 3: Authenticate with incorrect credentials
        System.out.println("Test 3 - Authenticate 'john' with wrong password: " + auth.authenticateUser("john", "wrongpassword"));
        
        // Test Case 4: Register duplicate user
        System.out.println("Test 4 - Register duplicate user 'john': " + auth.registerUser("john", "newpassword"));
        
        // Test Case 5: Change password
        System.out.println("Test 5 - Change password for 'john': " + auth.changePassword("john", "password123", "newpassword123"));
        System.out.println("Test 5 - Authenticate with new password: " + auth.authenticateUser("john", "newpassword123"));
    }
}

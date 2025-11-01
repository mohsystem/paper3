
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;

public class Lse39 {
    private static Map<String, String> users = new HashMap<>();
    private static Map<String, String> salts = new HashMap<>();
    
    // Initialize with some test users (in production, this would be a database)
    static {
        try {
            registerUser("user1", "Password123!");
            registerUser("admin", "Admin@2024");
            registerUser("test", "Test#Pass1");
            registerUser("john", "John$ecure9");
            registerUser("alice", "Alice&Pass7");
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
    
    // Register a new user with hashed password and salt
    public static void registerUser(String username, String password) throws NoSuchAlgorithmException {
        if (username == null || password == null || username.trim().isEmpty() || password.length() < 8) {
            throw new IllegalArgumentException("Invalid username or password");
        }
        
        String salt = generateSalt();
        String hashedPassword = hashPassword(password, salt);
        users.put(username, hashedPassword);
        salts.put(username, salt);
    }
    
    // Generate a random salt
    private static String generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[16];
        random.nextBytes(salt);
        return Base64.getEncoder().encodeToString(salt);
    }
    
    // Hash password with salt using SHA-256
    private static String hashPassword(String password, String salt) throws NoSuchAlgorithmException {
        MessageDigest md = MessageDigest.getInstance("SHA-256");
        md.update(Base64.getDecoder().decode(salt));
        byte[] hashedPassword = md.digest(password.getBytes());
        return Base64.getEncoder().encodeToString(hashedPassword);
    }
    
    // Login method with secure password verification
    public static String login(String username, String password) {
        // Input validation
        if (username == null || password == null || username.trim().isEmpty() || password.trim().isEmpty()) {
            return "Username or password is incorrect";
        }
        
        // Check if user exists
        if (!users.containsKey(username)) {
            return "Username or password is incorrect";
        }
        
        try {
            // Get stored hash and salt
            String storedHash = users.get(username);
            String salt = salts.get(username);
            
            // Hash the provided password with the stored salt
            String providedHash = hashPassword(password, salt);
            
            // Constant-time comparison to prevent timing attacks
            if (MessageDigest.isEqual(storedHash.getBytes(), providedHash.getBytes())) {
                return "Login successful! Redirecting to session page...";
            } else {
                return "Username or password is incorrect";
            }
        } catch (NoSuchAlgorithmException e) {
            return "An error occurred during authentication";
        }
    }
    
    public static void main(String[] args) {
        System.out.println("=== Login System Test Cases ===\\n");
        
        // Test case 1: Valid login
        System.out.println("Test 1 - Valid credentials (user1):");
        System.out.println(login("user1", "Password123!"));
        System.out.println();
        
        // Test case 2: Valid login with different user
        System.out.println("Test 2 - Valid credentials (admin):");
        System.out.println(login("admin", "Admin@2024"));
        System.out.println();
        
        // Test case 3: Invalid password
        System.out.println("Test 3 - Invalid password:");
        System.out.println(login("user1", "WrongPassword"));
        System.out.println();
        
        // Test case 4: Non-existent user
        System.out.println("Test 4 - Non-existent user:");
        System.out.println(login("nonexistent", "Password123!"));
        System.out.println();
        
        // Test case 5: Empty credentials
        System.out.println("Test 5 - Empty credentials:");
        System.out.println(login("", ""));
        System.out.println();
    }
}

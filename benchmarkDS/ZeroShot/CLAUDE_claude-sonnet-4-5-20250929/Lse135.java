
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.HashMap;
import java.util.Map;

public class Lse135 {
    // Simulated secure user database with hashed passwords
    private static final Map<String, String> userDatabase = new HashMap<>();
    
    static {
        // Store users with hashed passwords (SHA-256)
        // For demo: password "admin123" and "user456" are pre-hashed
        userDatabase.put("admin", hashPassword("admin123"));
        userDatabase.put("user1", hashPassword("user456"));
        userDatabase.put("testuser", hashPassword("test789"));
    }
    
    // Hash password using SHA-256
    private static String hashPassword(String password) {
        try {
            MessageDigest md = MessageDigest.getInstance("SHA-256");
            byte[] hashedBytes = md.digest(password.getBytes());
            StringBuilder sb = new StringBuilder();
            for (byte b : hashedBytes) {
                sb.append(String.format("%02x", b));
            }
            return sb.toString();
        } catch (NoSuchAlgorithmException e) {
            throw new RuntimeException("Error hashing password", e);
        }
    }
    
    // Main function to check credentials
    public static boolean checkMod(String username, String password) {
        if (username == null || password == null || username.isEmpty() || password.isEmpty()) {
            return false;
        }
        
        // Get stored hashed password for username
        String storedHash = userDatabase.get(username);
        
        if (storedHash == null) {
            return false;
        }
        
        // Hash the provided password and compare
        String inputHash = hashPassword(password);
        return storedHash.equals(inputHash);
    }
    
    // Main method with test cases
    public static void main(String[] args) {
        System.out.println("Test Case 1 (Valid - admin): " + checkMod("admin", "admin123"));
        System.out.println("Test Case 2 (Valid - user1): " + checkMod("user1", "user456"));
        System.out.println("Test Case 3 (Invalid password): " + checkMod("admin", "wrongpass"));
        System.out.println("Test Case 4 (Invalid username): " + checkMod("nonexistent", "admin123"));
        System.out.println("Test Case 5 (Empty credentials): " + checkMod("", ""));
    }
}

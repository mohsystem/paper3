
import java.sql.*;
import java.security.MessageDigest;
import java.security.SecureRandom;
import java.util.Base64;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;

public class Lse34 {
    private static final String DB_URL = "jdbc:sqlite:users.db";
    
    // Secure password hashing using PBKDF2
    private static String hashPassword(String password, byte[] salt) throws Exception {
        PBEKeySpec spec = new PBEKeySpec(password.toCharArray(), salt, 10000, 256);
        SecretKeyFactory factory = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
        byte[] hash = factory.generateSecret(spec).getEncoded();
        return Base64.getEncoder().encodeToString(hash);
    }
    
    private static byte[] generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[16];
        random.nextBytes(salt);
        return salt;
    }
    
    // Secure login function using prepared statements
    public static String authenticateUser(String username, String password) {
        if (username == null || password == null || username.trim().isEmpty() || password.trim().isEmpty()) {
            return "redirect:/login?error=invalid";
        }
        
        String query = "SELECT password_hash, salt FROM users WHERE username = ?";
        
        try (Connection conn = DriverManager.getConnection(DB_URL);
             PreparedStatement pstmt = conn.prepareStatement(query)) {
            
            pstmt.setString(1, username);
            ResultSet rs = pstmt.executeQuery();
            
            if (rs.next()) {
                String storedHash = rs.getString("password_hash");
                byte[] salt = Base64.getDecoder().decode(rs.getString("salt"));
                String inputHash = hashPassword(password, salt);
                
                if (storedHash.equals(inputHash)) {
                    return "redirect:/dashboard";
                }
            }
        } catch (Exception e) {
            System.err.println("Authentication error: " + e.getMessage());
        }
        
        return "redirect:/login?error=invalid";
    }
    
    // Helper function to create user (for testing)
    private static void createUser(String username, String password) throws Exception {
        byte[] salt = generateSalt();
        String passwordHash = hashPassword(password, salt);
        String saltStr = Base64.getEncoder().encodeToString(salt);
        
        String query = "INSERT INTO users (username, password_hash, salt) VALUES (?, ?, ?)";
        
        try (Connection conn = DriverManager.getConnection(DB_URL);
             PreparedStatement pstmt = conn.prepareStatement(query)) {
            
            pstmt.setString(1, username);
            pstmt.setString(2, passwordHash);
            pstmt.setString(3, saltStr);
            pstmt.executeUpdate();
        }
    }
    
    private static void initDatabase() throws Exception {
        try (Connection conn = DriverManager.getConnection(DB_URL);
             Statement stmt = conn.createStatement()) {
            
            stmt.execute("CREATE TABLE IF NOT EXISTS users (" +
                        "id INTEGER PRIMARY KEY AUTOINCREMENT, " +
                        "username TEXT UNIQUE NOT NULL, " +
                        "password_hash TEXT NOT NULL, " +
                        "salt TEXT NOT NULL)");
        }
    }
    
    public static void main(String[] args) throws Exception {
        initDatabase();
        
        // Test cases
        System.out.println("=== Test Cases ===");
        
        // Setup test users
        try {
            createUser("admin", "SecurePass123!");
            createUser("user1", "MyPassword456");
            createUser("testuser", "Test@789");
        } catch (Exception e) {
            System.out.println("Users may already exist");
        }
        
        // Test 1: Valid credentials
        System.out.println("Test 1 - Valid login: " + 
            authenticateUser("admin", "SecurePass123!"));
        
        // Test 2: Invalid password
        System.out.println("Test 2 - Wrong password: " + 
            authenticateUser("admin", "WrongPass"));
        
        // Test 3: Non-existent user
        System.out.println("Test 3 - Invalid user: " + 
            authenticateUser("hacker", "password"));
        
        // Test 4: Empty credentials
        System.out.println("Test 4 - Empty input: " + 
            authenticateUser("", ""));
        
        // Test 5: SQL injection attempt (safely handled)
        System.out.println("Test 5 - SQL injection attempt: " + 
            authenticateUser("admin' OR '1'='1", "anything"));
    }
}

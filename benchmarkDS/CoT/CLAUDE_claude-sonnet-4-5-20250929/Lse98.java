
import java.sql.*;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.Base64;
import java.util.regex.Pattern;

public class Lse98 {
    private static final String DB_URL = "jdbc:sqlite:users.db";
    private static final Pattern USERNAME_PATTERN = Pattern.compile("^[a-zA-Z0-9_]{3,20}$");
    private static final int MIN_PASSWORD_LENGTH = 8;
    
    public static class RegistrationResult {
        public boolean success;
        public String message;
        
        public RegistrationResult(boolean success, String message) {
            this.success = success;
            this.message = message;
        }
    }
    
    public static void initializeDatabase() {
        try (Connection conn = DriverManager.getConnection(DB_URL);
             Statement stmt = conn.createStatement()) {
            String sql = "CREATE TABLE IF NOT EXISTS users (" +
                        "id INTEGER PRIMARY KEY AUTOINCREMENT," +
                        "username TEXT UNIQUE NOT NULL," +
                        "password_hash TEXT NOT NULL," +
                        "salt TEXT NOT NULL," +
                        "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP)";
            stmt.execute(sql);
        } catch (SQLException e) {
            System.err.println("Database initialization failed");
        }
    }
    
    public static String hashPassword(String password, String salt) throws NoSuchAlgorithmException {
        MessageDigest md = MessageDigest.getInstance("SHA-256");
        md.update(Base64.getDecoder().decode(salt));
        byte[] hashedPassword = md.digest(password.getBytes());
        return Base64.getEncoder().encodeToString(hashedPassword);
    }
    
    public static String generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[16];
        random.nextBytes(salt);
        return Base64.getEncoder().encodeToString(salt);
    }
    
    public static boolean validateUsername(String username) {
        return username != null && USERNAME_PATTERN.matcher(username).matches();
    }
    
    public static boolean validatePassword(String password) {
        return password != null && password.length() >= MIN_PASSWORD_LENGTH;
    }
    
    public static RegistrationResult registerUser(String username, String password) {
        if (!validateUsername(username)) {
            return new RegistrationResult(false, "Invalid username format");
        }
        
        if (!validatePassword(password)) {
            return new RegistrationResult(false, "Password must be at least " + MIN_PASSWORD_LENGTH + " characters");
        }
        
        try {
            String salt = generateSalt();
            String hashedPassword = hashPassword(password, salt);
            
            try (Connection conn = DriverManager.getConnection(DB_URL);
                 PreparedStatement pstmt = conn.prepareStatement(
                     "INSERT INTO users (username, password_hash, salt) VALUES (?, ?, ?)")) {
                
                pstmt.setString(1, username);
                pstmt.setString(2, hashedPassword);
                pstmt.setString(3, salt);
                pstmt.executeUpdate();
                
                return new RegistrationResult(true, "Registration successful");
            }
        } catch (SQLException e) {
            if (e.getMessage().contains("UNIQUE constraint failed")) {
                return new RegistrationResult(false, "Username already exists");
            }
            return new RegistrationResult(false, "Registration failed");
        } catch (NoSuchAlgorithmException e) {
            return new RegistrationResult(false, "System error occurred");
        }
    }
    
    public static void main(String[] args) {
        initializeDatabase();
        
        System.out.println("=== User Registration Test Cases ===\\n");
        
        // Test Case 1: Valid registration
        RegistrationResult result1 = registerUser("john_doe", "SecurePass123");
        System.out.println("Test 1 - Valid registration: " + result1.message);
        
        // Test Case 2: Duplicate username
        RegistrationResult result2 = registerUser("john_doe", "AnotherPass456");
        System.out.println("Test 2 - Duplicate username: " + result2.message);
        
        // Test Case 3: Invalid username (too short)
        RegistrationResult result3 = registerUser("ab", "ValidPassword1");
        System.out.println("Test 3 - Invalid username: " + result3.message);
        
        // Test Case 4: Weak password
        RegistrationResult result4 = registerUser("jane_doe", "short");
        System.out.println("Test 4 - Weak password: " + result4.message);
        
        // Test Case 5: Valid registration with special characters in password
        RegistrationResult result5 = registerUser("alice_smith", "Str0ng!P@ssw0rd");
        System.out.println("Test 5 - Valid with special chars: " + result5.message);
    }
}

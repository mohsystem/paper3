
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.sql.*;
import java.util.Base64;
import java.util.regex.Pattern;

public class Task46 {
    private static final String DB_URL = "jdbc:sqlite:users.db";
    private static final Pattern EMAIL_PATTERN = Pattern.compile(
        "^[A-Za-z0-9+_.-]+@[A-Za-z0-9.-]+\\\\.[A-Za-z]{2,}$"
    );
    
    public static class RegistrationResult {
        public boolean success;
        public String message;
        
        public RegistrationResult(boolean success, String message) {
            this.success = success;
            this.message = message;
        }
    }
    
    private static void initializeDatabase() {
        try (Connection conn = DriverManager.getConnection(DB_URL);
             Statement stmt = conn.createStatement()) {
            String sql = "CREATE TABLE IF NOT EXISTS users (" +
                        "id INTEGER PRIMARY KEY AUTOINCREMENT," +
                        "name TEXT NOT NULL," +
                        "email TEXT UNIQUE NOT NULL," +
                        "password_hash TEXT NOT NULL," +
                        "salt TEXT NOT NULL)";
            stmt.execute(sql);
        } catch (SQLException e) {
            System.err.println("Database initialization error: " + e.getMessage());
        }
    }
    
    private static String generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[16];
        random.nextBytes(salt);
        return Base64.getEncoder().encodeToString(salt);
    }
    
    private static String hashPassword(String password, String salt) {
        try {
            MessageDigest md = MessageDigest.getInstance("SHA-256");
            md.update(Base64.getDecoder().decode(salt));
            byte[] hashedPassword = md.digest(password.getBytes());
            return Base64.getEncoder().encodeToString(hashedPassword);
        } catch (NoSuchAlgorithmException e) {
            throw new RuntimeException("Hashing algorithm not found", e);
        }
    }
    
    private static boolean validateName(String name) {
        return name != null && name.trim().length() >= 2 && name.trim().length() <= 100;
    }
    
    private static boolean validateEmail(String email) {
        return email != null && EMAIL_PATTERN.matcher(email).matches();
    }
    
    private static boolean validatePassword(String password) {
        return password != null && password.length() >= 8 && password.length() <= 128;
    }
    
    public static RegistrationResult registerUser(String name, String email, String password) {
        if (!validateName(name)) {
            return new RegistrationResult(false, "Invalid name. Must be 2-100 characters.");
        }
        
        if (!validateEmail(email)) {
            return new RegistrationResult(false, "Invalid email format.");
        }
        
        if (!validatePassword(password)) {
            return new RegistrationResult(false, "Invalid password. Must be 8-128 characters.");
        }
        
        String salt = generateSalt();
        String passwordHash = hashPassword(password, salt);
        
        String sql = "INSERT INTO users (name, email, password_hash, salt) VALUES (?, ?, ?, ?)";
        
        try (Connection conn = DriverManager.getConnection(DB_URL);
             PreparedStatement pstmt = conn.prepareStatement(sql)) {
            
            pstmt.setString(1, name.trim());
            pstmt.setString(2, email.toLowerCase().trim());
            pstmt.setString(3, passwordHash);
            pstmt.setString(4, salt);
            
            pstmt.executeUpdate();
            return new RegistrationResult(true, "User registered successfully.");
            
        } catch (SQLException e) {
            if (e.getMessage().contains("UNIQUE constraint failed")) {
                return new RegistrationResult(false, "Email already registered.");
            }
            return new RegistrationResult(false, "Registration failed: " + e.getMessage());
        }
    }
    
    public static void main(String[] args) {
        initializeDatabase();
        
        System.out.println("=== User Registration System Test Cases ===\\n");
        
        // Test Case 1: Valid registration
        RegistrationResult result1 = registerUser("John Doe", "john.doe@example.com", "SecurePass123");
        System.out.println("Test 1 - Valid registration: " + result1.message);
        
        // Test Case 2: Duplicate email
        RegistrationResult result2 = registerUser("Jane Doe", "john.doe@example.com", "AnotherPass456");
        System.out.println("Test 2 - Duplicate email: " + result2.message);
        
        // Test Case 3: Invalid email format
        RegistrationResult result3 = registerUser("Bob Smith", "invalid-email", "Password789");
        System.out.println("Test 3 - Invalid email: " + result3.message);
        
        // Test Case 4: Weak password
        RegistrationResult result4 = registerUser("Alice Brown", "alice@example.com", "123");
        System.out.println("Test 4 - Weak password: " + result4.message);
        
        // Test Case 5: Invalid name
        RegistrationResult result5 = registerUser("A", "test@example.com", "ValidPass123");
        System.out.println("Test 5 - Invalid name: " + result5.message);
    }
}

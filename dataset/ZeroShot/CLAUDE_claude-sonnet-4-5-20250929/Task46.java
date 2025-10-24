
import java.sql.*;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
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
    
    static {
        initializeDatabase();
    }
    
    private static void initializeDatabase() {
        try (Connection conn = DriverManager.getConnection(DB_URL);
             Statement stmt = conn.createStatement()) {
            String sql = "CREATE TABLE IF NOT EXISTS users (" +
                        "id INTEGER PRIMARY KEY AUTOINCREMENT," +
                        "name TEXT NOT NULL," +
                        "email TEXT UNIQUE NOT NULL," +
                        "password_hash TEXT NOT NULL," +
                        "salt TEXT NOT NULL," +
                        "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP)";
            stmt.execute(sql);
        } catch (SQLException e) {
            System.err.println("Database initialization error: " + e.getMessage());
        }
    }
    
    public static RegistrationResult registerUser(String name, String email, String password) {
        if (!validateInput(name, email, password)) {
            return new RegistrationResult(false, "Invalid input parameters");
        }
        
        if (!validateEmail(email)) {
            return new RegistrationResult(false, "Invalid email format");
        }
        
        if (!validatePassword(password)) {
            return new RegistrationResult(false, "Password must be at least 8 characters long");
        }
        
        try {
            String salt = generateSalt();
            String passwordHash = hashPassword(password, salt);
            
            if (insertUser(name, email, passwordHash, salt)) {
                return new RegistrationResult(true, "User registered successfully");
            } else {
                return new RegistrationResult(false, "Email already exists");
            }
        } catch (Exception e) {
            return new RegistrationResult(false, "Registration failed: " + e.getMessage());
        }
    }
    
    private static boolean validateInput(String name, String email, String password) {
        return name != null && !name.trim().isEmpty() &&
               email != null && !email.trim().isEmpty() &&
               password != null && !password.isEmpty();
    }
    
    private static boolean validateEmail(String email) {
        return EMAIL_PATTERN.matcher(email).matches();
    }
    
    private static boolean validatePassword(String password) {
        return password.length() >= 8;
    }
    
    private static String generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[16];
        random.nextBytes(salt);
        return Base64.getEncoder().encodeToString(salt);
    }
    
    private static String hashPassword(String password, String salt) 
            throws NoSuchAlgorithmException {
        MessageDigest md = MessageDigest.getInstance("SHA-256");
        md.update(Base64.getDecoder().decode(salt));
        byte[] hashedPassword = md.digest(password.getBytes());
        return Base64.getEncoder().encodeToString(hashedPassword);
    }
    
    private static boolean insertUser(String name, String email, 
                                     String passwordHash, String salt) {
        String sql = "INSERT INTO users (name, email, password_hash, salt) VALUES (?, ?, ?, ?)";
        
        try (Connection conn = DriverManager.getConnection(DB_URL);
             PreparedStatement pstmt = conn.prepareStatement(sql)) {
            
            pstmt.setString(1, name);
            pstmt.setString(2, email);
            pstmt.setString(3, passwordHash);
            pstmt.setString(4, salt);
            pstmt.executeUpdate();
            return true;
            
        } catch (SQLException e) {
            if (e.getMessage().contains("UNIQUE constraint failed")) {
                return false;
            }
            System.err.println("Insert error: " + e.getMessage());
            return false;
        }
    }
    
    public static void main(String[] args) {
        System.out.println("=== User Registration System - Test Cases ===\\n");
        
        // Test Case 1: Valid registration
        RegistrationResult result1 = registerUser("John Doe", "john.doe@example.com", "SecurePass123");
        System.out.println("Test 1 - Valid registration:");
        System.out.println("Result: " + result1.success + " - " + result1.message + "\\n");
        
        // Test Case 2: Duplicate email
        RegistrationResult result2 = registerUser("Jane Doe", "john.doe@example.com", "AnotherPass456");
        System.out.println("Test 2 - Duplicate email:");
        System.out.println("Result: " + result2.success + " - " + result2.message + "\\n");
        
        // Test Case 3: Invalid email format
        RegistrationResult result3 = registerUser("Bob Smith", "invalid-email", "Password789");
        System.out.println("Test 3 - Invalid email:");
        System.out.println("Result: " + result3.success + " - " + result3.message + "\\n");
        
        // Test Case 4: Weak password
        RegistrationResult result4 = registerUser("Alice Brown", "alice@example.com", "weak");
        System.out.println("Test 4 - Weak password:");
        System.out.println("Result: " + result4.success + " - " + result4.message + "\\n");
        
        // Test Case 5: Valid registration with different user
        RegistrationResult result5 = registerUser("Charlie Wilson", "charlie@example.com", "StrongPass999");
        System.out.println("Test 5 - Another valid registration:");
        System.out.println("Result: " + result5.success + " - " + result5.message);
    }
}

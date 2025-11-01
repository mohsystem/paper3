
import java.sql.*;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.Base64;

public class Lse97 {
    private static final String DB_URL = "jdbc:sqlite:users.db";
    
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
                        "username TEXT UNIQUE NOT NULL," +
                        "password_hash TEXT NOT NULL," +
                        "salt TEXT NOT NULL," +
                        "first_name TEXT NOT NULL," +
                        "last_name TEXT NOT NULL," +
                        "email TEXT NOT NULL)";
            stmt.execute(sql);
        } catch (SQLException e) {
            e.printStackTrace();
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
            throw new RuntimeException("Error hashing password", e);
        }
    }
    
    public static RegistrationResult registerUser(String username, String password, 
                                                   String firstName, String lastName, 
                                                   String email) {
        if (username == null || username.trim().isEmpty() ||
            password == null || password.trim().isEmpty() ||
            firstName == null || firstName.trim().isEmpty() ||
            lastName == null || lastName.trim().isEmpty() ||
            email == null || email.trim().isEmpty()) {
            return new RegistrationResult(false, "All fields are required");
        }
        
        if (!email.matches("^[A-Za-z0-9+_.-]+@[A-Za-z0-9.-]+$")) {
            return new RegistrationResult(false, "Invalid email format");
        }
        
        initializeDatabase();
        
        try (Connection conn = DriverManager.getConnection(DB_URL)) {
            String checkSql = "SELECT COUNT(*) FROM users WHERE username = ?";
            try (PreparedStatement checkStmt = conn.prepareStatement(checkSql)) {
                checkStmt.setString(1, username);
                ResultSet rs = checkStmt.executeQuery();
                if (rs.next() && rs.getInt(1) > 0) {
                    return new RegistrationResult(false, "Username already exists");
                }
            }
            
            String salt = generateSalt();
            String passwordHash = hashPassword(password, salt);
            
            String insertSql = "INSERT INTO users (username, password_hash, salt, first_name, last_name, email) " +
                             "VALUES (?, ?, ?, ?, ?, ?)";
            try (PreparedStatement insertStmt = conn.prepareStatement(insertSql)) {
                insertStmt.setString(1, username);
                insertStmt.setString(2, passwordHash);
                insertStmt.setString(3, salt);
                insertStmt.setString(4, firstName);
                insertStmt.setString(5, lastName);
                insertStmt.setString(6, email);
                insertStmt.executeUpdate();
            }
            
            return new RegistrationResult(true, "Registration succeeded");
            
        } catch (SQLException e) {
            return new RegistrationResult(false, "Database error: " + e.getMessage());
        }
    }
    
    public static void main(String[] args) {
        System.out.println("=== User Registration System Tests ===\\n");
        
        RegistrationResult result1 = registerUser("john_doe", "SecurePass123!", 
                                                   "John", "Doe", "john@example.com");
        System.out.println("Test 1: " + result1.message);
        
        RegistrationResult result2 = registerUser("jane_smith", "StrongP@ssw0rd", 
                                                   "Jane", "Smith", "jane@example.com");
        System.out.println("Test 2: " + result2.message);
        
        RegistrationResult result3 = registerUser("john_doe", "AnotherPass456", 
                                                   "John", "Different", "different@example.com");
        System.out.println("Test 3: " + result3.message);
        
        RegistrationResult result4 = registerUser("bob_wilson", "MyP@ss123", 
                                                   "Bob", "Wilson", "invalid-email");
        System.out.println("Test 4: " + result4.message);
        
        RegistrationResult result5 = registerUser("alice_brown", "", 
                                                   "Alice", "Brown", "alice@example.com");
        System.out.println("Test 5: " + result5.message);
    }
}

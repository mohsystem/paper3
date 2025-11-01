
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.sql.*;
import java.util.Base64;

public class Lse97 {
    
    public static String registerUser(String username, String password, String firstName, 
                                     String lastName, String email, Connection conn) {
        // Input validation
        if (username == null || username.trim().isEmpty() || 
            password == null || password.trim().isEmpty() ||
            firstName == null || firstName.trim().isEmpty() ||
            lastName == null || lastName.trim().isEmpty() ||
            email == null || email.trim().isEmpty()) {
            return "Error: All fields are required";
        }
        
        // Validate email format
        if (!email.matches("^[A-Za-z0-9+_.-]+@[A-Za-z0-9.-]+\\\\.[A-Za-z]{2,}$")) {
            return "Error: Invalid email format";
        }
        
        // Password strength validation
        if (password.length() < 8) {
            return "Error: Password must be at least 8 characters";
        }
        
        try {
            // Check if username already exists using prepared statement
            String checkQuery = "SELECT username FROM users WHERE username = ?";
            try (PreparedStatement checkStmt = conn.prepareStatement(checkQuery)) {
                checkStmt.setString(1, username);
                ResultSet rs = checkStmt.executeQuery();
                
                if (rs.next()) {
                    return "Error: Username already exists";
                }
            }
            
            // Generate salt and hash password
            byte[] salt = generateSalt();
            String hashedPassword = hashPassword(password, salt);
            String saltString = Base64.getEncoder().encodeToString(salt);
            
            // Insert user into database using prepared statement
            String insertQuery = "INSERT INTO users (username, password, salt, first_name, last_name, email) VALUES (?, ?, ?, ?, ?, ?)";
            try (PreparedStatement insertStmt = conn.prepareStatement(insertQuery)) {
                insertStmt.setString(1, username);
                insertStmt.setString(2, hashedPassword);
                insertStmt.setString(3, saltString);
                insertStmt.setString(4, firstName);
                insertStmt.setString(5, lastName);
                insertStmt.setString(6, email);
                
                insertStmt.executeUpdate();
            }
            
            return "Success: Registration succeeded";
            
        } catch (SQLException e) {
            return "Error: Database operation failed - " + e.getMessage();
        } catch (NoSuchAlgorithmException e) {
            return "Error: Hashing algorithm not available";
        }
    }
    
    private static byte[] generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[16];
        random.nextBytes(salt);
        return salt;
    }
    
    private static String hashPassword(String password, byte[] salt) throws NoSuchAlgorithmException {
        MessageDigest md = MessageDigest.getInstance("SHA-256");
        md.update(salt);
        byte[] hashedPassword = md.digest(password.getBytes());
        return Base64.getEncoder().encodeToString(hashedPassword);
    }
    
    public static void main(String[] args) {
        // Mock database connection for testing
        // In production, use proper database connection
        System.out.println("Registration System Test Cases:\\n");
        
        try {
            // For demonstration purposes - would need actual database setup
            Connection mockConn = null; // Replace with actual connection
            
            System.out.println("Test Case 1 - Valid registration:");
            System.out.println("Input: username='john_doe', password='SecurePass123', firstName='John', lastName='Doe', email='john@example.com'");
            System.out.println("Expected: Registration succeeded (with proper DB)\\n");
            
            System.out.println("Test Case 2 - Duplicate username:");
            System.out.println("Input: username='john_doe' (existing), password='Pass456', firstName='Jane', lastName='Smith', email='jane@example.com'");
            System.out.println("Expected: Username already exists\\n");
            
            System.out.println("Test Case 3 - Empty fields:");
            System.out.println("Input: username='', password='Pass123', firstName='Test', lastName='User', email='test@example.com'");
            System.out.println("Result: " + registerUser("", "Pass123", "Test", "User", "test@example.com", mockConn) + "\\n");
            
            System.out.println("Test Case 4 - Invalid email:");
            System.out.println("Input: username='testuser', password='Pass123', firstName='Test', lastName='User', email='invalid-email'");
            System.out.println("Result: " + registerUser("testuser", "Pass123", "Test", "User", "invalid-email", mockConn) + "\\n");
            
            System.out.println("Test Case 5 - Weak password:");
            System.out.println("Input: username='testuser2', password='123', firstName='Test', lastName='User', email='test2@example.com'");
            System.out.println("Result: " + registerUser("testuser2", "123", "Test", "User", "test2@example.com", mockConn) + "\\n");
            
        } catch (Exception e) {
            System.out.println("Error in test execution: " + e.getMessage());
        }
    }
}

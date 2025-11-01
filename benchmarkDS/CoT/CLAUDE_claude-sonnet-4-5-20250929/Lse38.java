
import java.sql.*;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.nio.charset.StandardCharsets;
import java.util.Base64;

public class Lse38 {
    private static final String DB_URL = "jdbc:mysql://localhost:3306/userdb";
    private static final String DB_USER = "root";
    private static final String DB_PASSWORD = "password";
    
    public static String hashPassword(String password) {
        try {
            MessageDigest digest = MessageDigest.getInstance("SHA-256");
            byte[] hash = digest.digest(password.getBytes(StandardCharsets.UTF_8));
            return Base64.getEncoder().encodeToString(hash);
        } catch (NoSuchAlgorithmException e) {
            throw new RuntimeException("Error hashing password", e);
        }
    }
    
    public static String authenticateUser(String username, String password) {
        if (username == null || password == null || username.trim().isEmpty() || password.trim().isEmpty()) {
            return "login";
        }
        
        String hashedPassword = hashPassword(password);
        
        try (Connection conn = DriverManager.getConnection(DB_URL, DB_USER, DB_PASSWORD)) {
            String query = "SELECT password_hash FROM users WHERE username = ?";
            try (PreparedStatement stmt = conn.prepareStatement(query)) {
                stmt.setString(1, username);
                
                try (ResultSet rs = stmt.executeQuery()) {
                    if (rs.next()) {
                        String storedHash = rs.getString("password_hash");
                        if (storedHash != null && storedHash.equals(hashedPassword)) {
                            return "dashboard";
                        }
                    }
                }
            }
        } catch (SQLException e) {
            System.err.println("Database error: " + e.getMessage());
        }
        
        return "login";
    }
    
    public static void main(String[] args) {
        System.out.println("Test Case 1 - Valid user:");
        System.out.println("Redirect to: " + authenticateUser("admin", "admin123"));
        
        System.out.println("\\nTest Case 2 - Invalid password:");
        System.out.println("Redirect to: " + authenticateUser("admin", "wrongpass"));
        
        System.out.println("\\nTest Case 3 - Non-existent user:");
        System.out.println("Redirect to: " + authenticateUser("nonexistent", "password"));
        
        System.out.println("\\nTest Case 4 - Empty credentials:");
        System.out.println("Redirect to: " + authenticateUser("", ""));
        
        System.out.println("\\nTest Case 5 - Null credentials:");
        System.out.println("Redirect to: " + authenticateUser(null, null));
    }
}

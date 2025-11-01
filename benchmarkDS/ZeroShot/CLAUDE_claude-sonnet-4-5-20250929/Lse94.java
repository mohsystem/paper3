
import java.security.MessageDigest;
import java.security.SecureRandom;
import java.sql.*;
import java.util.Base64;

public class Lse94 {
    private static final String DB_URL = "jdbc:sqlite:users.db";
    
    public static String generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[16];
        random.nextBytes(salt);
        return Base64.getEncoder().encodeToString(salt);
    }
    
    public static String hashPassword(String password, String salt) {
        try {
            MessageDigest md = MessageDigest.getInstance("SHA-256");
            md.update(Base64.getDecoder().decode(salt));
            byte[] hashedPassword = md.digest(password.getBytes());
            return Base64.getEncoder().encodeToString(hashedPassword);
        } catch (Exception e) {
            throw new RuntimeException("Error hashing password", e);
        }
    }
    
    public static boolean insertUser(String username, String password) {
        String salt = generateSalt();
        String hashedPassword = hashPassword(password, salt);
        
        try (Connection conn = DriverManager.getConnection(DB_URL)) {
            // Create table if not exists
            String createTableSQL = "CREATE TABLE IF NOT EXISTS users (" +
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, " +
                    "username TEXT UNIQUE NOT NULL, " +
                    "password TEXT NOT NULL, " +
                    "salt TEXT NOT NULL)";
            Statement stmt = conn.createStatement();
            stmt.execute(createTableSQL);
            
            // Insert user
            String insertSQL = "INSERT INTO users (username, password, salt) VALUES (?, ?, ?)";
            PreparedStatement pstmt = conn.prepareStatement(insertSQL);
            pstmt.setString(1, username);
            pstmt.setString(2, hashedPassword);
            pstmt.setString(3, salt);
            pstmt.executeUpdate();
            return true;
        } catch (SQLException e) {
            System.err.println("Error inserting user: " + e.getMessage());
            return false;
        }
    }
    
    public static void main(String[] args) {
        System.out.println("Test Case 1: " + insertUser("alice", "password123"));
        System.out.println("Test Case 2: " + insertUser("bob", "securePass456"));
        System.out.println("Test Case 3: " + insertUser("charlie", "myP@ssw0rd"));
        System.out.println("Test Case 4: " + insertUser("david", "test1234"));
        System.out.println("Test Case 5: " + insertUser("eve", "strongPassword!"));
    }
}


import java.security.MessageDigest;
import java.security.SecureRandom;
import java.sql.*;
import java.util.Base64;

public class Lse94 {
    
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
            throw new RuntimeException(e);
        }
    }
    
    public static boolean insertUser(String username, String password) {
        String salt = generateSalt();
        String hashedPassword = hashPassword(password, salt);
        
        String url = "jdbc:sqlite:users.db";
        
        try (Connection conn = DriverManager.getConnection(url)) {
            // Create table if not exists
            String createTableSQL = "CREATE TABLE IF NOT EXISTS users (" +
                                   "id INTEGER PRIMARY KEY AUTOINCREMENT," +
                                   "username TEXT NOT NULL UNIQUE," +
                                   "password TEXT NOT NULL," +
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
            System.out.println("Error: " + e.getMessage());
            return false;
        }
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test 1: " + insertUser("alice", "password123"));
        System.out.println("Test 2: " + insertUser("bob", "securePass456"));
        System.out.println("Test 3: " + insertUser("charlie", "mySecret789"));
        System.out.println("Test 4: " + insertUser("diana", "p@ssw0rd!"));
        System.out.println("Test 5: " + insertUser("eve", "test12345"));
    }
}

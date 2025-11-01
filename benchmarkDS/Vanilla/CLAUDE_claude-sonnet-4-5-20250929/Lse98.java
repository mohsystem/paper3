
import java.sql.*;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.nio.charset.StandardCharsets;

public class Lse98 {
    
    public static String hashPassword(String password) {
        try {
            MessageDigest digest = MessageDigest.getInstance("SHA-256");
            byte[] hash = digest.digest(password.getBytes(StandardCharsets.UTF_8));
            StringBuilder hexString = new StringBuilder();
            for (byte b : hash) {
                String hex = Integer.toHexString(0xff & b);
                if (hex.length() == 1) hexString.append('0');
                hexString.append(hex);
            }
            return hexString.toString();
        } catch (NoSuchAlgorithmException e) {
            throw new RuntimeException(e);
        }
    }
    
    public static boolean registerUser(String username, String password) {
        String hashedPassword = hashPassword(password);
        String url = "jdbc:sqlite:users.db";
        
        try {
            Connection conn = DriverManager.getConnection(url);
            String createTableSQL = "CREATE TABLE IF NOT EXISTS users (id INTEGER PRIMARY KEY AUTOINCREMENT, username TEXT UNIQUE, password TEXT)";
            Statement stmt = conn.createStatement();
            stmt.execute(createTableSQL);
            
            String insertSQL = "INSERT INTO users (username, password) VALUES (?, ?)";
            PreparedStatement pstmt = conn.prepareStatement(insertSQL);
            pstmt.setString(1, username);
            pstmt.setString(2, hashedPassword);
            pstmt.executeUpdate();
            
            conn.close();
            return true;
        } catch (SQLException e) {
            System.out.println("Error: " + e.getMessage());
            return false;
        }
    }
    
    public static void main(String[] args) {
        System.out.println("Test Case 1: " + registerUser("alice", "password123"));
        System.out.println("Test Case 2: " + registerUser("bob", "securePass456"));
        System.out.println("Test Case 3: " + registerUser("charlie", "mySecret789"));
        System.out.println("Test Case 4: " + registerUser("diana", "test@2024"));
        System.out.println("Test Case 5: " + registerUser("eve", "strongPwd!"));
    }
}

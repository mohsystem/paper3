
import java.sql.*;
import java.util.Scanner;

public class Task37 {
    private static final String DB_URL = "jdbc:sqlite:users.db";
    
    public static void initializeDatabase() {
        try (Connection conn = DriverManager.getConnection(DB_URL);
             Statement stmt = conn.createStatement()) {
            
            String createTableSQL = "CREATE TABLE IF NOT EXISTS users (" +
                    "id INTEGER PRIMARY KEY AUTOINCREMENT," +
                    "username TEXT NOT NULL UNIQUE," +
                    "email TEXT," +
                    "fullname TEXT)";
            stmt.execute(createTableSQL);
            
            String insertSQL = "INSERT OR IGNORE INTO users (username, email, fullname) VALUES " +
                    "('john_doe', 'john@example.com', 'John Doe')," +
                    "('jane_smith', 'jane@example.com', 'Jane Smith')," +
                    "('bob_wilson', 'bob@example.com', 'Bob Wilson')," +
                    "('alice_brown', 'alice@example.com', 'Alice Brown')," +
                    "('charlie_davis', 'charlie@example.com', 'Charlie Davis')";
            stmt.execute(insertSQL);
            
        } catch (SQLException e) {
            System.out.println("Database initialization error: " + e.getMessage());
        }
    }
    
    public static String getUserInfo(String username) {
        String query = "SELECT * FROM users WHERE username = ?";
        StringBuilder result = new StringBuilder();
        
        try (Connection conn = DriverManager.getConnection(DB_URL);
             PreparedStatement pstmt = conn.prepareStatement(query)) {
            
            pstmt.setString(1, username);
            ResultSet rs = pstmt.executeQuery();
            
            if (rs.next()) {
                result.append("User found:\\n");
                result.append("ID: ").append(rs.getInt("id")).append("\\n");
                result.append("Username: ").append(rs.getString("username")).append("\\n");
                result.append("Email: ").append(rs.getString("email")).append("\\n");
                result.append("Full Name: ").append(rs.getString("fullname")).append("\\n");
            } else {
                result.append("User not found: ").append(username);
            }
            
        } catch (SQLException e) {
            result.append("Database error: ").append(e.getMessage());
        }
        
        return result.toString();
    }
    
    public static void main(String[] args) {
        initializeDatabase();
        
        System.out.println("Test Case 1:");
        System.out.println(getUserInfo("john_doe"));
        System.out.println("\\n" + "=".repeat(50) + "\\n");
        
        System.out.println("Test Case 2:");
        System.out.println(getUserInfo("jane_smith"));
        System.out.println("\\n" + "=".repeat(50) + "\\n");
        
        System.out.println("Test Case 3:");
        System.out.println(getUserInfo("alice_brown"));
        System.out.println("\\n" + "=".repeat(50) + "\\n");
        
        System.out.println("Test Case 4:");
        System.out.println(getUserInfo("nonexistent_user"));
        System.out.println("\\n" + "=".repeat(50) + "\\n");
        
        System.out.println("Test Case 5:");
        System.out.println(getUserInfo("charlie_davis"));
    }
}

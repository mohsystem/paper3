
import java.sql.*;
import java.util.Properties;
import java.io.InputStream;
import java.io.IOException;

public class Task37 {
    private static final int MAX_USERNAME_LENGTH = 50;
    private static final String USERNAME_PATTERN = "^[a-zA-Z0-9_-]+$";
    
    public static String getUserInfo(String username) {
        if (username == null || username.isEmpty()) {
            return "Error: Username cannot be null or empty";
        }
        
        if (username.length() > MAX_USERNAME_LENGTH) {
            return "Error: Username exceeds maximum length";
        }
        
        if (!username.matches(USERNAME_PATTERN)) {
            return "Error: Username contains invalid characters";
        }
        
        Properties props = new Properties();
        try (InputStream input = Task37.class.getClassLoader().getResourceAsStream("db.properties")) {
            if (input == null) {
                return "Error: Database configuration file not found";
            }
            props.load(input);
        } catch (IOException e) {
            return "Error: Failed to load database configuration";
        }
        
        String url = props.getProperty("db.url");
        String dbUser = props.getProperty("db.user");
        String dbPassword = props.getProperty("db.password");
        
        if (url == null || dbUser == null || dbPassword == null) {
            return "Error: Missing database configuration";
        }
        
        String query = "SELECT id, username, email, created_at FROM users WHERE username = ?";
        
        try (Connection conn = DriverManager.getConnection(url, dbUser, dbPassword);
             PreparedStatement pstmt = conn.prepareStatement(query)) {
            
            pstmt.setString(1, username);
            
            try (ResultSet rs = pstmt.executeQuery()) {
                if (rs.next()) {
                    int id = rs.getInt("id");
                    String user = rs.getString("username");
                    String email = rs.getString("email");
                    String createdAt = rs.getString("created_at");
                    
                    return String.format("User found - ID: %d, Username: %s, Email: %s, Created: %s",
                            id, user, email, createdAt);
                } else {
                    return "User not found";
                }
            }
        } catch (SQLException e) {
            return "Error: Database operation failed";
        }
    }
    
    public static void main(String[] args) {
        System.out.println("Test Case 1 - Valid username:");
        System.out.println(getUserInfo("john_doe"));
        System.out.println();
        
        System.out.println("Test Case 2 - Valid username with numbers:");
        System.out.println(getUserInfo("user123"));
        System.out.println();
        
        System.out.println("Test Case 3 - Invalid username with special chars:");
        System.out.println(getUserInfo("user'; DROP TABLE users--"));
        System.out.println();
        
        System.out.println("Test Case 4 - Null username:");
        System.out.println(getUserInfo(null));
        System.out.println();
        
        System.out.println("Test Case 5 - Empty username:");
        System.out.println(getUserInfo(""));
    }
}

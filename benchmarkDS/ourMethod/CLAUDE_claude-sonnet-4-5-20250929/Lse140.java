
import java.sql.*;
import java.util.Properties;
import java.io.FileInputStream;
import java.io.IOException;

public class Lse140 {
    // Check if username exists in database
    // Returns true if exists, false otherwise
    public static boolean checkUsernameExists(String username) {
        // Rules#2: Input validation - check username is not null/empty and within reasonable bounds
        if (username == null || username.trim().isEmpty()) {
            return false;
        }
        // Limit username length to prevent potential DoS
        if (username.length() > 255) {
            return false;
        }
        
        Connection conn = null;
        PreparedStatement pstmt = null;
        ResultSet rs = null;
        
        try {
            // Rules#3: Never hard code credentials - load from external config
            Properties props = new Properties();
            try (FileInputStream fis = new FileInputStream("db.properties")) {
                props.load(fis);
            }
            
            String dbUrl = props.getProperty("db.url");
            String dbUser = props.getProperty("db.user");
            String dbPassword = props.getProperty("db.password");
            
            // Validate connection properties exist
            if (dbUrl == null || dbUser == null || dbPassword == null) {
                throw new SQLException("Database configuration incomplete");
            }
            
            // Rules#6: TLS enforcement - ensure connection uses SSL
            Properties connProps = new Properties();
            connProps.setProperty("user", dbUser);
            connProps.setProperty("password", dbPassword);
            connProps.setProperty("useSSL", "true");
            connProps.setProperty("requireSSL", "true");
            
            conn = DriverManager.getConnection(dbUrl, connProps);
            
            // Rules#32: Use parameterized queries to prevent SQL injection
            String sql = "SELECT COUNT(*) FROM users WHERE username = ?";
            pstmt = conn.prepareStatement(sql);
            pstmt.setString(1, username);
            
            rs = pstmt.executeQuery();
            
            if (rs.next()) {
                int count = rs.getInt(1);
                return count > 0;
            }
            
            return false;
            
        } catch (SQLException | IOException e) {
            // Rules#11: Don't leak internal details in errors\n            System.err.println("Database operation failed");\n            return false;\n        } finally {\n            // Clean up resources in reverse order of creation\n            try {\n                if (rs != null) rs.close();\n                if (pstmt != null) pstmt.close();\n                if (conn != null) conn.close();\n            } catch (SQLException e) {\n                System.err.println("Resource cleanup failed");\n            }\n        }\n    }\n    \n    public static void main(String[] args) {\n        // Test case 1: Valid username that exists\n        System.out.println("Test 1 (existing user): " + checkUsernameExists("alice"));\n        \n        // Test case 2: Valid username that doesn't exist
        System.out.println("Test 2 (non-existing user): " + checkUsernameExists("nonexistent"));
        
        // Test case 3: Null username
        System.out.println("Test 3 (null): " + checkUsernameExists(null));
        
        // Test case 4: Empty username
        System.out.println("Test 4 (empty): " + checkUsernameExists(""));
        
        // Test case 5: Username with special characters
        System.out.println("Test 5 (special chars): " + checkUsernameExists("user@test"));
    }
}

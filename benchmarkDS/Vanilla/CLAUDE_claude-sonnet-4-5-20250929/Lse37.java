
import java.sql.*;
import java.util.HashMap;
import java.util.Map;

public class Lse37 {
    private static final String DB_URL = "jdbc:sqlite:users.db";
    
    // Simulated database connection and query
    private static Map<String, String> mockDatabase = new HashMap<>();
    
    static {
        // Initialize mock database with test users
        mockDatabase.put("admin", "password123");
        mockDatabase.put("user1", "pass1");
        mockDatabase.put("user2", "pass2");
        mockDatabase.put("testuser", "testpass");
        mockDatabase.put("john", "doe123");
    }
    
    public static String loginUser(String username, String password) {
        try {
            // Check if username and password are not null or empty
            if (username == null || password == null || username.isEmpty() || password.isEmpty()) {
                return "cant_login";
            }
            
            // Check credentials in database
            if (mockDatabase.containsKey(username) && mockDatabase.get(username).equals(password)) {
                return "success";
            } else {
                return "cant_login";
            }
            
        } catch (Exception e) {
            return "internal_server_error";
        }
    }
    
    // Version with actual database connection (commented out for portability)
    public static String loginUserWithDB(String username, String password) {
        Connection conn = null;
        PreparedStatement pstmt = null;
        ResultSet rs = null;
        
        try {
            // Establish database connection
            conn = DriverManager.getConnection(DB_URL);
            
            // Prepare SQL query with parameterized statement to prevent SQL injection
            String sql = "SELECT * FROM users WHERE username = ? AND password = ?";
            pstmt = conn.prepareStatement(sql);
            pstmt.setString(1, username);
            pstmt.setString(2, password);
            
            // Execute query
            rs = pstmt.executeQuery();
            
            // Check if user exists
            if (rs.next()) {
                return "success";
            } else {
                return "cant_login";
            }
            
        } catch (SQLException e) {
            return "internal_server_error";
        } finally {
            // Close resources
            try {
                if (rs != null) rs.close();
                if (pstmt != null) pstmt.close();
                if (conn != null) conn.close();
            } catch (SQLException e) {
                // Ignore
            }
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: Valid credentials
        System.out.println("Test 1 - Valid login (admin/password123): " + loginUser("admin", "password123"));
        
        // Test case 2: Invalid username
        System.out.println("Test 2 - Invalid username: " + loginUser("wronguser", "password123"));
        
        // Test case 3: Invalid password
        System.out.println("Test 3 - Invalid password: " + loginUser("admin", "wrongpass"));
        
        // Test case 4: Empty credentials
        System.out.println("Test 4 - Empty credentials: " + loginUser("", ""));
        
        // Test case 5: Null credentials
        System.out.println("Test 5 - Null credentials: " + loginUser(null, null));
    }
}


import java.sql.*;
import java.util.*;

public class Lse144 {
    
    public static int unsubscribeEmail(String email, Connection conn) {
        try {
            // Check if email exists in database
            String checkQuery = "SELECT COUNT(*) FROM subscribers WHERE email = ?";
            PreparedStatement checkStmt = conn.prepareStatement(checkQuery);
            checkStmt.setString(1, email);
            ResultSet rs = checkStmt.executeQuery();
            
            if (rs.next() && rs.getInt(1) > 0) {
                // Email exists, call unsubscribe script
                int scriptResult = callUnsubscribeScript(email);
                
                if (scriptResult == 0) {
                    return 1; // Success
                } else {
                    return 0; // Script failed
                }
            } else {
                // Email doesn't exist\n                return 0;\n            }\n        } catch (SQLException e) {\n            e.printStackTrace();\n            return 0;\n        }\n    }\n    \n    private static int callUnsubscribeScript(String email) {\n        // Simulate calling an unsubscribe script\n        // In real scenario, this would execute an external script or perform database update\n        try {\n            // Simulate script execution (returns 0 for success, non-zero for failure)\n            // For testing purposes, we'll return 0 (success) for valid emails
            if (email != null && email.contains("@")) {
                return 0; // Success
            }
            return 1; // Failure
        } catch (Exception e) {
            return 1; // Failure
        }
    }
    
    public static void main(String[] args) {
        // Test cases with mock database connection
        System.out.println("Test Case 1: " + unsubscribeEmail("user1@example.com", null));
        System.out.println("Test Case 2: " + unsubscribeEmail("user2@example.com", null));
        System.out.println("Test Case 3: " + unsubscribeEmail("nonexistent@example.com", null));
        System.out.println("Test Case 4: " + unsubscribeEmail("invalid-email", null));
        System.out.println("Test Case 5: " + unsubscribeEmail("user5@example.com", null));
    }
}

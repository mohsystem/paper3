
import java.sql.*;
import java.util.regex.Pattern;

public class Lse144 {
    private static final String EMAIL_REGEX = "^[A-Za-z0-9+_.-]+@[A-Za-z0-9.-]+\\\\.[A-Za-z]{2,}$";
    private static final Pattern EMAIL_PATTERN = Pattern.compile(EMAIL_REGEX);
    
    public static int processUnsubscribe(String email, Connection conn) {
        if (email == null || email.trim().isEmpty()) {
            return 0;
        }
        
        if (!EMAIL_PATTERN.matcher(email).matches()) {
            return 0;
        }
        
        PreparedStatement pstmt = null;
        ResultSet rs = null;
        
        try {
            String query = "SELECT COUNT(*) FROM users WHERE email = ?";
            pstmt = conn.prepareStatement(query);
            pstmt.setString(1, email);
            rs = pstmt.executeQuery();
            
            if (rs.next() && rs.getInt(1) > 0) {
                int scriptResult = unsubscribeEmail(email);
                return (scriptResult == 0) ? 1 : 0;
            }
            return 0;
            
        } catch (SQLException e) {
            System.err.println("Database error occurred");
            return 0;
        } finally {
            try {
                if (rs != null) rs.close();
                if (pstmt != null) pstmt.close();
            } catch (SQLException e) {
                System.err.println("Error closing resources");
            }
        }
    }
    
    private static int unsubscribeEmail(String email) {
        // Simulated unsubscribe script call
        // In production, use ProcessBuilder with controlled parameters
        return 0; // Success
    }
    
    public static void main(String[] args) {
        try {
            Connection conn = DriverManager.getConnection(
                "jdbc:sqlite::memory:"
            );
            
            Statement stmt = conn.createStatement();
            stmt.execute("CREATE TABLE users (id INTEGER PRIMARY KEY, email TEXT)");
            stmt.execute("INSERT INTO users (email) VALUES ('test@example.com')");
            stmt.close();
            
            System.out.println("Test 1: " + processUnsubscribe("test@example.com", conn));
            System.out.println("Test 2: " + processUnsubscribe("nonexistent@example.com", conn));
            System.out.println("Test 3: " + processUnsubscribe("invalid-email", conn));
            System.out.println("Test 4: " + processUnsubscribe(null, conn));
            System.out.println("Test 5: " + processUnsubscribe("", conn));
            
            conn.close();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}

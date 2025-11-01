
import java.sql.*;
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.util.regex.Pattern;

public class Lse144 {
    // Email validation pattern (RFC 5322 simplified)
    private static final Pattern EMAIL_PATTERN = Pattern.compile(
        "^[a-zA-Z0-9_+&*-]+(?:\\\\.[a-zA-Z0-9_+&*-]+)*@(?:[a-zA-Z0-9-]+\\\\.)+[a-zA-Z]{2,7}$"
    );
    
    // Maximum email length to prevent buffer overflow
    private static final int MAX_EMAIL_LENGTH = 254;
    
    /**
     * Checks if email exists in database and unsubscribes if found.
     * Returns 1 if unsubscribe successful, 0 otherwise.
     * 
     * @param email The email address to unsubscribe
     * @param dbUrl Database connection URL
     * @param dbUser Database username
     * @param dbPassword Database password
     * @param unsubscribeScriptPath Path to unsubscribe script
     * @return 1 if successful, 0 otherwise
     */
    public static int unsubscribeEmail(String email, String dbUrl, String dbUser, 
                                       String dbPassword, String unsubscribeScriptPath) {
        // Input validation: check for null and empty
        if (email == null || email.trim().isEmpty()) {
            return 0;
        }
        
        email = email.trim();
        
        // Validate email length to prevent buffer overflow
        if (email.length() > MAX_EMAIL_LENGTH) {
            return 0;
        }
        
        // Validate email format to prevent injection
        if (!EMAIL_PATTERN.matcher(email).matches()) {
            return 0;
        }
        
        // Validate database connection parameters
        if (dbUrl == null || dbUser == null || dbPassword == null || unsubscribeScriptPath == null) {
            return 0;
        }
        
        // Validate script path to prevent path traversal
        if (unsubscribeScriptPath.contains("..") || unsubscribeScriptPath.contains("~")) {
            return 0;
        }
        
        Connection conn = null;
        PreparedStatement pstmt = null;
        ResultSet rs = null;
        
        try {
            // Use prepared statement to prevent SQL injection
            conn = DriverManager.getConnection(dbUrl, dbUser, dbPassword);
            
            // Check if email exists in database
            String query = "SELECT COUNT(*) FROM subscribers WHERE email = ?";
            pstmt = conn.prepareStatement(query);
            pstmt.setString(1, email);
            
            rs = pstmt.executeQuery();
            
            if (rs.next() && rs.getInt(1) > 0) {
                // Email exists, call unsubscribe script
                return callUnsubscribeScript(email, unsubscribeScriptPath);
            } else {
                // Email doesn't exist\n                return 0;\n            }\n            \n        } catch (SQLException e) {\n            // Log error securely without exposing sensitive details\n            System.err.println("Database error occurred");\n            return 0;\n        } finally {\n            // Close resources in reverse order of creation\n            try {\n                if (rs != null) rs.close();\n                if (pstmt != null) pstmt.close();\n                if (conn != null) conn.close();\n            } catch (SQLException e) {\n                System.err.println("Error closing database resources");\n            }\n        }\n    }\n    \n    /**\n     * Calls the unsubscribe script with validated email parameter\n     * \n     * @param email Validated email address\n     * @param scriptPath Path to unsubscribe script\n     * @return 1 if script returns 0, otherwise 0\n     */\n    private static int callUnsubscribeScript(String email, String scriptPath) {\n        ProcessBuilder pb = null;\n        Process process = null;\n        \n        try {\n            // Use ProcessBuilder to prevent command injection\n            // Pass email as separate argument, not concatenated into command\n            pb = new ProcessBuilder(scriptPath, email);\n            \n            // Clear environment variables to prevent information leakage\n            pb.environment().clear();\n            \n            // Set timeout to prevent hanging\n            process = pb.start();\n            \n            // Wait for process with timeout (5 seconds)\n            boolean finished = process.waitFor(5, java.util.concurrent.TimeUnit.SECONDS);\n            \n            if (!finished) {\n                process.destroyForcibly();\n                return 0;\n            }\n            \n            int exitCode = process.exitValue();\n            \n            // Return 1 if script returns 0 (success), otherwise 0\n            return (exitCode == 0) ? 1 : 0;\n            \n        } catch (Exception e) {\n            // Log generic error without exposing details\n            System.err.println("Script execution error occurred");\n            if (process != null) {\n                process.destroyForcibly();\n            }\n            return 0;\n        } finally {\n            if (process != null && process.isAlive()) {\n                process.destroyForcibly();\n            }\n        }\n    }\n    \n    public static void main(String[] args) {\n        // Test cases with safe mock values\n        String dbUrl = "jdbc:sqlite::memory:";\n        String dbUser = "testuser";\n        String dbPassword = "testpass";\n        String scriptPath = "/usr/local/bin/unsubscribe.sh";\n        \n        // Test case 1: Valid email format\n        System.out.println("Test 1 (valid email): " + \n            unsubscribeEmail("user@example.com", dbUrl, dbUser, dbPassword, scriptPath));\n        \n        // Test case 2: Null email\n        System.out.println("Test 2 (null email): " + \n            unsubscribeEmail(null, dbUrl, dbUser, dbPassword, scriptPath));\n        \n        // Test case 3: Empty email\n        System.out.println("Test 3 (empty email): " + \n            unsubscribeEmail("", dbUrl, dbUser, dbPassword, scriptPath));\n        \n        // Test case 4: Invalid email format (SQL injection attempt)\n        System.out.println("Test 4 (injection attempt): " + \n            unsubscribeEmail("' OR '1'='1", dbUrl, dbUser, dbPassword, scriptPath));\n        \n        // Test case 5: Email too long\n        String longEmail = "a".repeat(300) + "@example.com";\n        System.out.println("Test 5 (too long): " + 
            unsubscribeEmail(longEmail, dbUrl, dbUser, dbPassword, scriptPath));
    }
}

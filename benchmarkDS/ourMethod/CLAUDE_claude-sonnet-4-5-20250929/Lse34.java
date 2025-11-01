
import java.sql.*;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.Base64;
import java.nio.charset.StandardCharsets;

public class Lse34 {
    // Database connection parameters - should be externalized in production
    private static final String DB_URL = System.getenv("DB_URL") != null ? 
        System.getenv("DB_URL") : "jdbc:sqlite:users.db";
    
    // Secure password hashing with PBKDF2
    private static final int ITERATIONS = 210000; // Meets minimum requirement
    private static final int KEY_LENGTH = 256;
    
    /**
     * Hashes a password using PBKDF2-HMAC-SHA256 with unique salt
     * @param password The password to hash
     * @param salt The unique salt for this password
     * @return Base64 encoded hash
     */
    private static String hashPassword(String password, byte[] salt) {
        try {
            javax.crypto.spec.PBEKeySpec spec = new javax.crypto.spec.PBEKeySpec(
                password.toCharArray(), salt, ITERATIONS, KEY_LENGTH);
            javax.crypto.SecretKeyFactory factory = 
                javax.crypto.SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
            byte[] hash = factory.generateSecret(spec).getEncoded();
            spec.clearPassword(); // Clear sensitive data
            return Base64.getEncoder().encodeToString(hash);
        } catch (Exception e) {
            throw new RuntimeException("Error hashing password", e);
        }
    }
    
    /**
     * Generates a cryptographically secure random salt
     * @return 16-byte salt
     */
    private static byte[] generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[16];
        random.nextBytes(salt);
        return salt;
    }
    
    /**
     * Validates input to prevent SQL injection and ensure reasonable length
     * @param input The input to validate
     * @param fieldName The field name for error messages
     * @return true if valid
     */
    private static boolean validateInput(String input, String fieldName) {
        if (input == null || input.isEmpty()) {
            System.err.println(fieldName + " cannot be empty");
            return false;
        }
        // Check length constraints
        if (input.length() < 3 || input.length() > 100) {
            System.err.println(fieldName + " must be between 3 and 100 characters");
            return false;
        }
        return true;
    }
    
    /**
     * Creates the database table if it doesn't exist\n     */\n    private static void initializeDatabase() {\n        // Use try-with-resources to ensure connection is closed\n        try (Connection conn = DriverManager.getConnection(DB_URL);\n             Statement stmt = conn.createStatement()) {\n            \n            String createTable = "CREATE TABLE IF NOT EXISTS users (" +\n                "id INTEGER PRIMARY KEY AUTOINCREMENT, " +\n                "username TEXT UNIQUE NOT NULL, " +\n                "password_hash TEXT NOT NULL, " +\n                "salt TEXT NOT NULL)";\n            stmt.execute(createTable);\n            \n        } catch (SQLException e) {\n            System.err.println("Database initialization failed");\n            throw new RuntimeException("Database error", e);\n        }\n    }\n    \n    /**\n     * Registers a new user with secure password hashing\n     * @param username The username\n     * @param password The password\n     * @return true if registration successful\n     */\n    private static boolean registerUser(String username, String password) {\n        // Validate inputs\n        if (!validateInput(username, "Username") || !validateInput(password, "Password")) {\n            return false;\n        }\n        \n        // Generate unique salt for this user\n        byte[] salt = generateSalt();\n        String saltBase64 = Base64.getEncoder().encodeToString(salt);\n        String passwordHash = hashPassword(password, salt);\n        \n        // Use parameterized query to prevent SQL injection\n        String sql = "INSERT INTO users (username, password_hash, salt) VALUES (?, ?, ?)";\n        \n        try (Connection conn = DriverManager.getConnection(DB_URL);\n             PreparedStatement pstmt = conn.prepareStatement(sql)) {\n            \n            pstmt.setString(1, username);\n            pstmt.setString(2, passwordHash);\n            pstmt.setString(3, saltBase64);\n            pstmt.executeUpdate();\n            return true;\n            \n        } catch (SQLException e) {\n            if (e.getMessage().contains("UNIQUE constraint failed")) {\n                System.err.println("Username already exists");\n            } else {\n                System.err.println("Registration failed");\n            }\n            return false;\n        }\n    }\n    \n    /**\n     * Authenticates a user and returns the redirect page\n     * @param username The username\n     * @param password The password\n     * @return "dashboard" if authenticated, "login" otherwise\n     */\n    public static String authenticateUser(String username, String password) {\n        // Validate inputs early\n        if (!validateInput(username, "Username") || !validateInput(password, "Password")) {\n            return "login";\n        }\n        \n        // Use parameterized query to prevent SQL injection\n        String sql = "SELECT password_hash, salt FROM users WHERE username = ?";\n        \n        try (Connection conn = DriverManager.getConnection(DB_URL);\n             PreparedStatement pstmt = conn.prepareStatement(sql)) {\n            \n            pstmt.setString(1, username);\n            \n            try (ResultSet rs = pstmt.executeQuery()) {\n                if (rs.next()) {\n                    String storedHash = rs.getString("password_hash");\n                    String saltBase64 = rs.getString("salt");\n                    byte[] salt = Base64.getDecoder().decode(saltBase64);\n                    \n                    // Hash the provided password with the stored salt\n                    String providedHash = hashPassword(password, salt);\n                    \n                    // Use constant-time comparison to prevent timing attacks\n                    if (MessageDigest.isEqual(\n                            storedHash.getBytes(StandardCharsets.UTF_8),\n                            providedHash.getBytes(StandardCharsets.UTF_8))) {\n                        return "dashboard";\n                    }\n                }\n            }\n            \n        } catch (SQLException e) {\n            // Log error securely without revealing details to user\n            System.err.println("Authentication error occurred");\n            // Do not log user input or sensitive details\n        }\n        \n        // Return generic failure - don't reveal if username exists
        return "login";
    }
    
    public static void main(String[] args) {
        // Initialize database
        initializeDatabase();
        
        // Test case 1: Register and authenticate valid user
        System.out.println("Test 1: Valid user registration and login");
        registerUser("testuser1", "SecurePass123!");
        String result1 = authenticateUser("testuser1", "SecurePass123!");
        System.out.println("Result: " + result1 + " (Expected: dashboard)");
        
        // Test case 2: Wrong password
        System.out.println("\\nTest 2: Wrong password");
        String result2 = authenticateUser("testuser1", "WrongPassword");
        System.out.println("Result: " + result2 + " (Expected: login)");
        
        // Test case 3: Non-existent user
        System.out.println("\\nTest 3: Non-existent user");
        String result3 = authenticateUser("nonexistent", "password123");
        System.out.println("Result: " + result3 + " (Expected: login)");
        
        // Test case 4: Empty username
        System.out.println("\\nTest 4: Empty username");
        String result4 = authenticateUser("", "password");
        System.out.println("Result: " + result4 + " (Expected: login)");
        
        // Test case 5: Multiple users with unique salts
        System.out.println("\\nTest 5: Multiple users with unique salts");
        registerUser("testuser2", "AnotherPass456!");
        String result5a = authenticateUser("testuser2", "AnotherPass456!");
        String result5b = authenticateUser("testuser1", "SecurePass123!");
        System.out.println("User2 Result: " + result5a + " (Expected: dashboard)");
        System.out.println("User1 Result: " + result5b + " (Expected: dashboard)");
    }
}

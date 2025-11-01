
import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.Base64;
import java.util.regex.Pattern;

public class Lse97 {
    // Pattern for input validation - alphanumeric and common safe chars
    private static final Pattern USERNAME_PATTERN = Pattern.compile("^[a-zA-Z0-9_]{3,30}$");
    private static final Pattern EMAIL_PATTERN = Pattern.compile("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\\\.[a-zA-Z]{2,}$");
    private static final Pattern NAME_PATTERN = Pattern.compile("^[a-zA-Z\\\\s'-]{1,50}$");
    private static final int MIN_PASSWORD_LENGTH = 8;
    private static final int MAX_PASSWORD_LENGTH = 128;
    private static final int PBKDF2_ITERATIONS = 210000;
    private static final int SALT_LENGTH = 16;
    
    public static class RegistrationResult {
        public final boolean success;
        public final String message;
        
        public RegistrationResult(boolean success, String message) {
            this.success = success;
            this.message = message;
        }
    }
    
    // Rules#3: Never hard code passwords - read from environment or config
    private static String getDbUrl() {
        String url = System.getenv("DB_URL");
        return url != null ? url : "jdbc:sqlite:users.db";
    }
    
    private static String getDbUser() {
        return System.getenv("DB_USER");
    }
    
    private static String getDbPassword() {
        return System.getenv("DB_PASSWORD");
    }
    
    // Rules#2: Input validation - check type, length, range, encoding
    private static boolean validateUsername(String username) {
        if (username == null || username.isEmpty()) {
            return false;
        }
        return USERNAME_PATTERN.matcher(username).matches();
    }
    
    private static boolean validatePassword(String password) {
        if (password == null) {
            return false;
        }
        int length = password.length();
        return length >= MIN_PASSWORD_LENGTH && length <= MAX_PASSWORD_LENGTH;
    }
    
    private static boolean validateName(String name) {
        if (name == null || name.isEmpty()) {
            return false;
        }
        return NAME_PATTERN.matcher(name).matches();
    }
    
    private static boolean validateEmail(String email) {
        if (email == null || email.isEmpty()) {
            return false;
        }
        return EMAIL_PATTERN.matcher(email).matches() && email.length() <= 254;
    }
    
    // Rules#5: Use PBKDF2-HMAC-SHA-256 with unique salt and 210000 iterations
    private static String hashPassword(String password, byte[] salt) throws NoSuchAlgorithmException {
        try {
            javax.crypto.spec.PBEKeySpec spec = new javax.crypto.spec.PBEKeySpec(
                password.toCharArray(), salt, PBKDF2_ITERATIONS, 256);
            javax.crypto.SecretKeyFactory factory = javax.crypto.SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
            byte[] hash = factory.generateSecret(spec).getEncoded();
            // Rules#9: Clear password from memory
            spec.clearPassword();
            return Base64.getEncoder().encodeToString(hash);
        } catch (java.security.spec.InvalidKeySpecException e) {
            throw new NoSuchAlgorithmException("Key spec error", e);
        }
    }
    
    // Rules#5: Generate salt with CSPRNG
    private static byte[] generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[SALT_LENGTH];
        random.nextBytes(salt);
        return salt;
    }
    
    // Rules#34-42: Validate and sanitize all inputs before database operations
    public static RegistrationResult registerUser(String username, String password, 
                                                   String firstName, String lastName, String email) {
        // Rules#34: Validate all inputs at trust boundary
        if (!validateUsername(username)) {
            return new RegistrationResult(false, "Registration failed");
        }
        if (!validatePassword(password)) {
            return new RegistrationResult(false, "Registration failed");
        }
        if (!validateName(firstName)) {
            return new RegistrationResult(false, "Registration failed");
        }
        if (!validateName(lastName)) {
            return new RegistrationResult(false, "Registration failed");
        }
        if (!validateEmail(email)) {
            return new RegistrationResult(false, "Registration failed");
        }
        
        Connection conn = null;
        PreparedStatement checkStmt = null;
        PreparedStatement insertStmt = null;
        ResultSet rs = null;
        
        try {
            // Rules#3: Get DB credentials from environment
            String dbUrl = getDbUrl();
            String dbUser = getDbUser();
            String dbPassword = getDbPassword();
            
            if (dbUser != null && dbPassword != null) {
                conn = DriverManager.getConnection(dbUrl, dbUser, dbPassword);
            } else {
                conn = DriverManager.getConnection(dbUrl);
            }
            
            // Rules#42: Use parameterized queries to prevent SQL injection
            String checkSql = "SELECT COUNT(*) FROM users WHERE username = ?";
            checkStmt = conn.prepareStatement(checkSql);
            checkStmt.setString(1, username);
            rs = checkStmt.executeQuery();
            
            if (rs.next() && rs.getInt(1) > 0) {
                // Rules#11: Generic error message - no internal details leaked
                return new RegistrationResult(false, "Registration failed");
            }
            
            // Rules#5: Generate salt and hash password
            byte[] salt = generateSalt();
            String hashedPassword = hashPassword(password, salt);
            String saltBase64 = Base64.getEncoder().encodeToString(salt);
            
            // Rules#42: Parameterized insert statement
            String insertSql = "INSERT INTO users (username, password_hash, salt, first_name, last_name, email) VALUES (?, ?, ?, ?, ?, ?)";
            insertStmt = conn.prepareStatement(insertSql);
            insertStmt.setString(1, username);
            insertStmt.setString(2, hashedPassword);
            insertStmt.setString(3, saltBase64);
            insertStmt.setString(4, firstName);
            insertStmt.setString(5, lastName);
            insertStmt.setString(6, email);
            
            int rowsAffected = insertStmt.executeUpdate();
            
            if (rowsAffected > 0) {
                return new RegistrationResult(true, "Registration succeeded");
            } else {
                return new RegistrationResult(false, "Registration failed");
            }
            
        } catch (SQLException | NoSuchAlgorithmException e) {
            // Rules#11: Generic message to user, detailed log for monitoring
            System.err.println("Registration error: " + e.getClass().getName());
            return new RegistrationResult(false, "Registration failed");
        } finally {
            // Cleanup resources in reverse order
            try { if (rs != null) rs.close(); } catch (SQLException e) { /* ignore */ }
            try { if (checkStmt != null) checkStmt.close(); } catch (SQLException e) { /* ignore */ }
            try { if (insertStmt != null) insertStmt.close(); } catch (SQLException e) { /* ignore */ }
            try { if (conn != null) conn.close(); } catch (SQLException e) { /* ignore */ }
        }
    }
    
    // Initialize database table for testing
    private static void initDatabase() {
        try (Connection conn = DriverManager.getConnection(getDbUrl())) {
            String createTable = "CREATE TABLE IF NOT EXISTS users (" +
                "id INTEGER PRIMARY KEY AUTOINCREMENT," +
                "username TEXT UNIQUE NOT NULL," +
                "password_hash TEXT NOT NULL," +
                "salt TEXT NOT NULL," +
                "first_name TEXT NOT NULL," +
                "last_name TEXT NOT NULL," +
                "email TEXT NOT NULL" +
                ")";
            try (PreparedStatement stmt = conn.prepareStatement(createTable)) {
                stmt.execute();
            }
        } catch (SQLException e) {
            System.err.println("Database init error: " + e.getMessage());
        }
    }
    
    public static void main(String[] args) {
        initDatabase();
        
        // Test case 1: Valid registration
        RegistrationResult r1 = registerUser("john_doe", "SecurePass123!", "John", "Doe", "john@example.com");
        System.out.println("Test 1 - Valid registration: " + r1.message);
        
        // Test case 2: Duplicate username
        RegistrationResult r2 = registerUser("john_doe", "AnotherPass456!", "Jane", "Smith", "jane@example.com");
        System.out.println("Test 2 - Duplicate username: " + r2.message);
        
        // Test case 3: Invalid username (too short)
        RegistrationResult r3 = registerUser("ab", "ValidPass789!", "Bob", "Johnson", "bob@example.com");
        System.out.println("Test 3 - Invalid username: " + r3.message);
        
        // Test case 4: Invalid password (too short)
        RegistrationResult r4 = registerUser("alice_w", "short", "Alice", "Williams", "alice@example.com");
        System.out.println("Test 4 - Invalid password: " + r4.message);
        
        // Test case 5: Invalid email format
        RegistrationResult r5 = registerUser("charlie_b", "ValidPass999!", "Charlie", "Brown", "invalid-email");
        System.out.println("Test 5 - Invalid email: " + r5.message);
    }
}

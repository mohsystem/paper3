
import java.sql.*;
import java.util.regex.Pattern;
import java.security.MessageDigest;
import java.nio.charset.StandardCharsets;

public class Task49 {
    private static final String DB_URL = "jdbc:sqlite:users.db";
    private static final Pattern EMAIL_PATTERN = Pattern.compile("^[A-Za-z0-9+_.-]+@[A-Za-z0-9.-]+\\\\.[A-Za-z]{2,}$");
    private static final Pattern NAME_PATTERN = Pattern.compile("^[A-Za-z\\\\s]{1,100}$");
    
    static class APIResponse {
        boolean success;
        String message;
        
        APIResponse(boolean success, String message) {
            this.success = success;
            this.message = message;
        }
    }
    
    static class UserData {
        String name;
        String email;
        String password;
        
        UserData(String name, String email, String password) {
            this.name = name;
            this.email = email;
            this.password = password;
        }
    }
    
    public static void initializeDatabase() {
        try (Connection conn = DriverManager.getConnection(DB_URL);
             Statement stmt = conn.createStatement()) {
            String sql = "CREATE TABLE IF NOT EXISTS users (" +
                        "id INTEGER PRIMARY KEY AUTOINCREMENT," +
                        "name TEXT NOT NULL," +
                        "email TEXT UNIQUE NOT NULL," +
                        "password_hash TEXT NOT NULL," +
                        "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP)";
            stmt.execute(sql);
        } catch (SQLException e) {
            System.err.println("Database initialization error: " + e.getMessage());
        }
    }
    
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
        } catch (Exception e) {
            throw new RuntimeException("Hashing error: " + e.getMessage());
        }
    }
    
    public static boolean validateInput(UserData userData) {
        if (userData.name == null || userData.email == null || userData.password == null) {
            return false;
        }
        
        if (userData.name.length() > 100 || userData.email.length() > 255 || userData.password.length() < 8) {
            return false;
        }
        
        if (!NAME_PATTERN.matcher(userData.name).matches()) {
            return false;
        }
        
        if (!EMAIL_PATTERN.matcher(userData.email).matches()) {
            return false;
        }
        
        return true;
    }
    
    public static APIResponse createUser(UserData userData) {
        if (!validateInput(userData)) {
            return new APIResponse(false, "Invalid input data");
        }
        
        String passwordHash = hashPassword(userData.password);
        
        String sql = "INSERT INTO users (name, email, password_hash) VALUES (?, ?, ?)";
        
        try (Connection conn = DriverManager.getConnection(DB_URL);
             PreparedStatement pstmt = conn.prepareStatement(sql)) {
            
            pstmt.setString(1, userData.name);
            pstmt.setString(2, userData.email);
            pstmt.setString(3, passwordHash);
            
            pstmt.executeUpdate();
            return new APIResponse(true, "User created successfully");
            
        } catch (SQLException e) {
            if (e.getMessage().contains("UNIQUE constraint failed")) {
                return new APIResponse(false, "Email already exists");
            }
            return new APIResponse(false, "Database error: " + e.getMessage());
        }
    }
    
    public static void main(String[] args) {
        initializeDatabase();
        
        System.out.println("=== Test Case 1: Valid user ===");
        UserData user1 = new UserData("John Doe", "john@example.com", "SecurePass123");
        APIResponse response1 = createUser(user1);
        System.out.println("Success: " + response1.success + ", Message: " + response1.message);
        
        System.out.println("\\n=== Test Case 2: Duplicate email ===");
        UserData user2 = new UserData("Jane Doe", "john@example.com", "AnotherPass456");
        APIResponse response2 = createUser(user2);
        System.out.println("Success: " + response2.success + ", Message: " + response2.message);
        
        System.out.println("\\n=== Test Case 3: Invalid email ===");
        UserData user3 = new UserData("Alice Smith", "invalid-email", "Password789");
        APIResponse response3 = createUser(user3);
        System.out.println("Success: " + response3.success + ", Message: " + response3.message);
        
        System.out.println("\\n=== Test Case 4: Short password ===");
        UserData user4 = new UserData("Bob Johnson", "bob@example.com", "short");
        APIResponse response4 = createUser(user4);
        System.out.println("Success: " + response4.success + ", Message: " + response4.message);
        
        System.out.println("\\n=== Test Case 5: Invalid name with numbers ===");
        UserData user5 = new UserData("Charlie123", "charlie@example.com", "ValidPass123");
        APIResponse response5 = createUser(user5);
        System.out.println("Success: " + response5.success + ", Message: " + response5.message);
    }
}

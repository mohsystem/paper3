
import java.sql.*;
import java.util.*;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

class Task46 {
    private static final String DB_URL = "jdbc:sqlite:users.db";
    
    static {
        try {
            Class.forName("org.sqlite.JDBC");
            initializeDatabase();
        } catch (ClassNotFoundException e) {
            System.err.println("SQLite JDBC driver not found: " + e.getMessage());
        }
    }
    
    private static void initializeDatabase() {
        try (Connection conn = DriverManager.getConnection(DB_URL);
             Statement stmt = conn.createStatement()) {
            String sql = "CREATE TABLE IF NOT EXISTS users (" +
                        "id INTEGER PRIMARY KEY AUTOINCREMENT," +
                        "name TEXT NOT NULL," +
                        "email TEXT UNIQUE NOT NULL," +
                        "password TEXT NOT NULL)";
            stmt.execute(sql);
        } catch (SQLException e) {
            System.err.println("Database initialization error: " + e.getMessage());
        }
    }
    
    private static String hashPassword(String password) {
        try {
            MessageDigest md = MessageDigest.getInstance("SHA-256");
            byte[] hash = md.digest(password.getBytes());
            StringBuilder hexString = new StringBuilder();
            for (byte b : hash) {
                String hex = Integer.toHexString(0xff & b);
                if (hex.length() == 1) hexString.append('0');
                hexString.append(hex);
            }
            return hexString.toString();
        } catch (NoSuchAlgorithmException e) {
            return password;
        }
    }
    
    public static boolean registerUser(String name, String email, String password) {
        String hashedPassword = hashPassword(password);
        String sql = "INSERT INTO users (name, email, password) VALUES (?, ?, ?)";
        
        try (Connection conn = DriverManager.getConnection(DB_URL);
             PreparedStatement pstmt = conn.prepareStatement(sql)) {
            pstmt.setString(1, name);
            pstmt.setString(2, email);
            pstmt.setString(3, hashedPassword);
            pstmt.executeUpdate();
            return true;
        } catch (SQLException e) {
            System.err.println("Registration error: " + e.getMessage());
            return false;
        }
    }
    
    public static boolean userExists(String email) {
        String sql = "SELECT COUNT(*) FROM users WHERE email = ?";
        try (Connection conn = DriverManager.getConnection(DB_URL);
             PreparedStatement pstmt = conn.prepareStatement(sql)) {
            pstmt.setString(1, email);
            ResultSet rs = pstmt.executeQuery();
            return rs.getInt(1) > 0;
        } catch (SQLException e) {
            return false;
        }
    }
    
    public static void main(String[] args) {
        System.out.println("User Registration System - Test Cases\\n");
        
        // Test Case 1
        System.out.println("Test Case 1: Register new user");
        boolean result1 = registerUser("John Doe", "john@example.com", "password123");
        System.out.println("Registration successful: " + result1);
        System.out.println("User exists: " + userExists("john@example.com"));
        System.out.println();
        
        // Test Case 2
        System.out.println("Test Case 2: Register another user");
        boolean result2 = registerUser("Jane Smith", "jane@example.com", "securePass456");
        System.out.println("Registration successful: " + result2);
        System.out.println("User exists: " + userExists("jane@example.com"));
        System.out.println();
        
        // Test Case 3
        System.out.println("Test Case 3: Try to register duplicate email");
        boolean result3 = registerUser("John Another", "john@example.com", "newpassword");
        System.out.println("Registration successful: " + result3);
        System.out.println();
        
        // Test Case 4
        System.out.println("Test Case 4: Register user with special characters");
        boolean result4 = registerUser("Mike O'Brien", "mike.obrien@example.com", "P@ssw0rd!");
        System.out.println("Registration successful: " + result4);
        System.out.println("User exists: " + userExists("mike.obrien@example.com"));
        System.out.println();
        
        // Test Case 5
        System.out.println("Test Case 5: Register user with long name");
        boolean result5 = registerUser("Alexander Christopher Wellington", "alex@example.com", "myPassword789");
        System.out.println("Registration successful: " + result5);
        System.out.println("User exists: " + userExists("alex@example.com"));
    }
}


import java.sql.*;
import java.util.*;
import java.util.regex.*;

class Task49 {
    private static final String DB_URL = "jdbc:sqlite:users.db";
    private static final Pattern EMAIL_PATTERN = Pattern.compile("^[A-Za-z0-9+_.-]+@[A-Za-z0-9.-]+\\\\.[A-Za-z]{2,}$");
    
    static class User {
        String name;
        String email;
        int age;
        
        User(String name, String email, int age) {
            this.name = name;
            this.email = email;
            this.age = age;
        }
    }
    
    static class ApiResponse {
        boolean success;
        String message;
        
        ApiResponse(boolean success, String message) {
            this.success = success;
            this.message = message;
        }
        
        @Override
        public String toString() {
            return "Success: " + success + ", Message: " + message;
        }
    }
    
    public static void initDatabase() {
        try (Connection conn = DriverManager.getConnection(DB_URL);
             Statement stmt = conn.createStatement()) {
            String sql = "CREATE TABLE IF NOT EXISTS users (" +
                        "id INTEGER PRIMARY KEY AUTOINCREMENT," +
                        "name TEXT NOT NULL," +
                        "email TEXT NOT NULL UNIQUE," +
                        "age INTEGER NOT NULL)";
            stmt.execute(sql);
        } catch (SQLException e) {
            System.err.println("Database initialization error: " + e.getMessage());
        }
    }
    
    public static boolean validateInput(String name, String email, int age) {
        if (name == null || name.trim().isEmpty() || name.length() > 100) {
            return false;
        }
        if (email == null || !EMAIL_PATTERN.matcher(email).matches()) {
            return false;
        }
        if (age < 0 || age > 150) {
            return false;
        }
        return true;
    }
    
    public static String sanitizeInput(String input) {
        if (input == null) return "";
        return input.trim().replaceAll("[<>\\"';]", "");
    }
    
    public static ApiResponse storeUserData(String name, String email, int age) {
        name = sanitizeInput(name);
        email = sanitizeInput(email);
        
        if (!validateInput(name, email, age)) {
            return new ApiResponse(false, "Invalid input data");
        }
        
        String sql = "INSERT INTO users (name, email, age) VALUES (?, ?, ?)";
        
        try (Connection conn = DriverManager.getConnection(DB_URL);
             PreparedStatement pstmt = conn.prepareStatement(sql)) {
            
            pstmt.setString(1, name);
            pstmt.setString(2, email);
            pstmt.setInt(3, age);
            pstmt.executeUpdate();
            
            return new ApiResponse(true, "User data stored successfully");
            
        } catch (SQLException e) {
            if (e.getMessage().contains("UNIQUE constraint failed")) {
                return new ApiResponse(false, "Email already exists");
            }
            return new ApiResponse(false, "Database error: " + e.getMessage());
        }
    }
    
    public static void main(String[] args) {
        initDatabase();
        
        System.out.println("=== API Endpoint Test Cases ===\\n");
        
        // Test Case 1: Valid user data
        System.out.println("Test 1 - Valid user:");
        ApiResponse response1 = storeUserData("John Doe", "john@example.com", 25);
        System.out.println(response1 + "\\n");
        
        // Test Case 2: Another valid user
        System.out.println("Test 2 - Another valid user:");
        ApiResponse response2 = storeUserData("Jane Smith", "jane@example.com", 30);
        System.out.println(response2 + "\\n");
        
        // Test Case 3: Duplicate email
        System.out.println("Test 3 - Duplicate email:");
        ApiResponse response3 = storeUserData("John Clone", "john@example.com", 35);
        System.out.println(response3 + "\\n");
        
        // Test Case 4: Invalid email format
        System.out.println("Test 4 - Invalid email:");
        ApiResponse response4 = storeUserData("Invalid User", "notanemail", 28);
        System.out.println(response4 + "\\n");
        
        // Test Case 5: Invalid age
        System.out.println("Test 5 - Invalid age:");
        ApiResponse response5 = storeUserData("Old Person", "old@example.com", 200);
        System.out.println(response5 + "\\n");
    }
}

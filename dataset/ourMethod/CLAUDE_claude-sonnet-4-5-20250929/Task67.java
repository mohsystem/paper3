
import java.sql.*;
import java.util.Properties;
import java.io.FileInputStream;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.util.regex.Pattern;

public class Task67 {
    private static final Pattern USERNAME_PATTERN = Pattern.compile("^[a-zA-Z0-9_]{3,50}$");
    private static final int MAX_USERNAME_LENGTH = 50;

    public static class Customer {
        private final String username;
        private final String email;
        private final String fullName;
        private final String phoneNumber;

        public Customer(String username, String email, String fullName, String phoneNumber) {
            this.username = username;
            this.email = email;
            this.fullName = fullName;
            this.phoneNumber = phoneNumber;
        }

        public String getUsername() { return username; }
        public String getEmail() { return email; }
        public String getFullName() { return fullName; }
        public String getPhoneNumber() { return phoneNumber; }

        @Override
        public String toString() {
            return "Customer{username='" + username + "', email='" + email + 
                   "', fullName='" + fullName + "', phoneNumber='" + phoneNumber + "'}";
        }
    }

    public static String validateUsername(String username) {
        if (username == null) {
            throw new IllegalArgumentException("Username cannot be null");
        }
        
        String trimmed = username.trim();
        
        if (trimmed.isEmpty()) {
            throw new IllegalArgumentException("Username cannot be empty");
        }
        
        if (trimmed.length() > MAX_USERNAME_LENGTH) {
            throw new IllegalArgumentException("Username exceeds maximum length of " + MAX_USERNAME_LENGTH);
        }
        
        if (!USERNAME_PATTERN.matcher(trimmed).matches()) {
            throw new IllegalArgumentException("Username contains invalid characters. Only alphanumeric and underscore allowed.");
        }
        
        return trimmed;
    }

    public static Customer getCustomerByUsername(String username, String dbUrl, String dbUser, String dbPassword) 
            throws SQLException {
        String validatedUsername = validateUsername(username);
        
        String query = "SELECT username, email, full_name, phone_number FROM customer WHERE username = ?";
        
        try (Connection conn = DriverManager.getConnection(dbUrl, dbUser, dbPassword);
             PreparedStatement stmt = conn.prepareStatement(query)) {
            
            stmt.setString(1, validatedUsername);
            
            try (ResultSet rs = stmt.executeQuery()) {
                if (rs.next()) {
                    String dbUsername = rs.getString("username");
                    String email = rs.getString("email");
                    String fullName = rs.getString("full_name");
                    String phoneNumber = rs.getString("phone_number");
                    
                    return new Customer(dbUsername, email, fullName, phoneNumber);
                }
            }
        }
        
        return null;
    }

    public static void main(String[] args) {
        if (args.length < 1) {
            System.err.println("Usage: java Task67 <customerusername>");
            System.exit(1);
        }

        String username = args[0];
        
        String dbUrl = System.getenv("DB_URL");
        String dbUser = System.getenv("DB_USER");
        String dbPassword = System.getenv("DB_PASSWORD");
        
        if (dbUrl == null || dbUser == null || dbPassword == null) {
            dbUrl = "jdbc:h2:mem:testdb";
            dbUser = "sa";
            dbPassword = "";
        }

        try {
            setupTestDatabase(dbUrl, dbUser, dbPassword);
            
            String[] testUsernames = {"testuser1", "testuser2", "testuser3", "invalid@user", "testuser999"};
            
            for (String testUsername : testUsernames) {
                try {
                    System.out.println("\\nQuerying for username: " + testUsername);
                    Customer customer = getCustomerByUsername(testUsername, dbUrl, dbUser, dbPassword);
                    
                    if (customer != null) {
                        System.out.println("Found: " + customer);
                    } else {
                        System.out.println("Customer not found");
                    }
                } catch (IllegalArgumentException e) {
                    System.err.println("Validation error: " + e.getMessage());
                } catch (SQLException e) {
                    System.err.println("Database error: " + e.getMessage());
                }
            }
            
        } catch (SQLException e) {
            System.err.println("Database setup error: " + e.getMessage());
            System.exit(1);
        }
    }

    private static void setupTestDatabase(String dbUrl, String dbUser, String dbPassword) throws SQLException {
        try (Connection conn = DriverManager.getConnection(dbUrl, dbUser, dbPassword);
             Statement stmt = conn.createStatement()) {
            
            stmt.execute("DROP TABLE IF EXISTS customer");
            stmt.execute("CREATE TABLE customer (" +
                        "username VARCHAR(50) PRIMARY KEY, " +
                        "email VARCHAR(100), " +
                        "full_name VARCHAR(100), " +
                        "phone_number VARCHAR(20))");
            
            String insertQuery = "INSERT INTO customer (username, email, full_name, phone_number) VALUES (?, ?, ?, ?)";
            try (PreparedStatement pstmt = conn.prepareStatement(insertQuery)) {
                String[][] testData = {
                    {"testuser1", "user1@example.com", "John Doe", "555-0101"},
                    {"testuser2", "user2@example.com", "Jane Smith", "555-0102"},
                    {"testuser3", "user3@example.com", "Bob Johnson", "555-0103"},
                    {"admin_user", "admin@example.com", "Admin User", "555-0100"},
                    {"test_user_5", "user5@example.com", "Alice Williams", "555-0105"}
                };
                
                for (String[] data : testData) {
                    pstmt.setString(1, data[0]);
                    pstmt.setString(2, data[1]);
                    pstmt.setString(3, data[2]);
                    pstmt.setString(4, data[3]);
                    pstmt.executeUpdate();
                }
            }
        }
    }
}

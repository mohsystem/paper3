
import java.sql.*;

public class Task67 {
    private static final String DB_URL = "jdbc:mysql://localhost:3306/customerdb";
    private static final String DB_USER = "root";
    private static final String DB_PASSWORD = "password";
    
    static class Customer {
        private int id;
        private String username;
        private String email;
        private String fullName;
        
        public Customer(int id, String username, String email, String fullName) {
            this.id = id;
            this.username = username;
            this.email = email;
            this.fullName = fullName;
        }
        
        @Override
        public String toString() {
            return "Customer{id=" + id + ", username='" + username + 
                   "', email='" + email + "', fullName='" + fullName + "'}";
        }
    }
    
    public static Customer getCustomerByUsername(String customerUsername) {
        // Input validation
        if (customerUsername == null || customerUsername.trim().isEmpty()) {
            System.err.println("Error: Username cannot be null or empty");
            return null;
        }
        
        // Sanitize input (allow only alphanumeric and underscore)
        if (!customerUsername.matches("^[a-zA-Z0-9_]{1,50}$")) {
            System.err.println("Error: Invalid username format");
            return null;
        }
        
        Connection conn = null;
        PreparedStatement pstmt = null;
        ResultSet rs = null;
        Customer customer = null;
        
        try {
            // Secure database connection
            conn = DriverManager.getConnection(DB_URL, DB_USER, DB_PASSWORD);
            
            // Use parameterized query to prevent SQL injection
            String query = "SELECT id, username, email, full_name FROM customer WHERE username = ?";
            pstmt = conn.prepareStatement(query);
            pstmt.setString(1, customerUsername);
            
            rs = pstmt.executeQuery();
            
            if (rs.next()) {
                customer = new Customer(
                    rs.getInt("id"),
                    rs.getString("username"),
                    rs.getString("email"),
                    rs.getString("full_name")
                );
            } else {
                System.out.println("No customer found with username: " + customerUsername);
            }
            
        } catch (SQLException e) {
            // Secure error handling - don't expose sensitive details
            System.err.println("Database error occurred while retrieving customer information");
            e.printStackTrace();
        } finally {
            // Proper resource cleanup
            try {
                if (rs != null) rs.close();
                if (pstmt != null) pstmt.close();
                if (conn != null) conn.close();
            } catch (SQLException e) {
                System.err.println("Error closing database resources");
            }
        }
        
        return customer;
    }
    
    public static void main(String[] args) {
        // Test cases
        String[] testUsernames = {"john_doe", "jane_smith", "admin123", "test_user", "nonexistent"};
        
        if (args.length > 0) {
            // Use command line argument
            Customer customer = getCustomerByUsername(args[0]);
            if (customer != null) {
                System.out.println("Retrieved: " + customer);
            }
        } else {
            // Run test cases
            System.out.println("Running test cases...\\n");
            for (String username : testUsernames) {
                System.out.println("Testing username: " + username);
                Customer customer = getCustomerByUsername(username);
                if (customer != null) {
                    System.out.println("Result: " + customer);
                }
                System.out.println("---");
            }
        }
    }
}

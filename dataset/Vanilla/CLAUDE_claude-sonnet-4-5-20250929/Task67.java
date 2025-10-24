
import java.sql.*;
import java.util.*;

class Customer {
    private String username;
    private String name;
    private String email;
    private String phone;
    
    public Customer(String username, String name, String email, String phone) {
        this.username = username;
        this.name = name;
        this.email = email;
        this.phone = phone;
    }
    
    @Override
    public String toString() {
        return "Customer{username='" + username + "', name='" + name + 
               "', email='" + email + "', phone='" + phone + "'}";
    }
}

public class Task67 {
    private static final String DB_URL = "jdbc:sqlite:customer.db";
    
    public static Customer getCustomerByUsername(String customerUsername) {
        Customer customer = null;
        Connection conn = null;
        PreparedStatement pstmt = null;
        ResultSet rs = null;
        
        try {
            conn = DriverManager.getConnection(DB_URL);
            String sql = "SELECT * FROM customer WHERE customerusername = ?";
            pstmt = conn.prepareStatement(sql);
            pstmt.setString(1, customerUsername);
            rs = pstmt.executeQuery();
            
            if (rs.next()) {
                customer = new Customer(
                    rs.getString("customerusername"),
                    rs.getString("name"),
                    rs.getString("email"),
                    rs.getString("phone")
                );
            }
        } catch (SQLException e) {
            System.err.println("Database error: " + e.getMessage());
        } finally {
            try {
                if (rs != null) rs.close();
                if (pstmt != null) pstmt.close();
                if (conn != null) conn.close();
            } catch (SQLException e) {
                System.err.println("Error closing resources: " + e.getMessage());
            }
        }
        
        return customer;
    }
    
    private static void setupTestDatabase() {
        try (Connection conn = DriverManager.getConnection(DB_URL);
             Statement stmt = conn.createStatement()) {
            
            stmt.execute("DROP TABLE IF EXISTS customer");
            stmt.execute("CREATE TABLE customer (" +
                        "customerusername TEXT PRIMARY KEY, " +
                        "name TEXT, " +
                        "email TEXT, " +
                        "phone TEXT)");
            
            stmt.execute("INSERT INTO customer VALUES ('user1', 'John Doe', 'john@email.com', '123-456-7890')");
            stmt.execute("INSERT INTO customer VALUES ('user2', 'Jane Smith', 'jane@email.com', '098-765-4321')");
            stmt.execute("INSERT INTO customer VALUES ('user3', 'Bob Johnson', 'bob@email.com', '555-555-5555')");
            stmt.execute("INSERT INTO customer VALUES ('user4', 'Alice Brown', 'alice@email.com', '444-444-4444')");
            stmt.execute("INSERT INTO customer VALUES ('user5', 'Charlie Davis', 'charlie@email.com', '333-333-3333')");
            
        } catch (SQLException e) {
            System.err.println("Error setting up test database: " + e.getMessage());
        }
    }
    
    public static void main(String[] args) {
        setupTestDatabase();
        
        String[] testCases = {"user1", "user2", "user3", "user4", "nonexistent"};
        
        if (args.length > 0) {
            Customer customer = getCustomerByUsername(args[0]);
            if (customer != null) {
                System.out.println("Found: " + customer);
            } else {
                System.out.println("Customer not found with username: " + args[0]);
            }
        } else {
            System.out.println("Running test cases:");
            for (String username : testCases) {
                System.out.println("\\nSearching for: " + username);
                Customer customer = getCustomerByUsername(username);
                if (customer != null) {
                    System.out.println("Found: " + customer);
                } else {
                    System.out.println("Customer not found");
                }
            }
        }
    }
}

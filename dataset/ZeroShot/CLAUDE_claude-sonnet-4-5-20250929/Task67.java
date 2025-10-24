
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
        if (customerUsername == null || customerUsername.trim().isEmpty()) {
            return null;
        }
        
        String query = "SELECT username, name, email, phone FROM customer WHERE username = ?";
        
        try (Connection conn = DriverManager.getConnection(DB_URL);
             PreparedStatement pstmt = conn.prepareStatement(query)) {
            
            pstmt.setString(1, customerUsername);
            
            try (ResultSet rs = pstmt.executeQuery()) {
                if (rs.next()) {
                    return new Customer(
                        rs.getString("username"),
                        rs.getString("name"),
                        rs.getString("email"),
                        rs.getString("phone")
                    );
                }
            }
        } catch (SQLException e) {
            System.err.println("Database error: " + e.getMessage());
        }
        
        return null;
    }
    
    private static void initializeDatabase() {
        try (Connection conn = DriverManager.getConnection(DB_URL);
             Statement stmt = conn.createStatement()) {
            
            stmt.execute("CREATE TABLE IF NOT EXISTS customer (" +
                        "username TEXT PRIMARY KEY, " +
                        "name TEXT, " +
                        "email TEXT, " +
                        "phone TEXT)");
            
            stmt.execute("DELETE FROM customer");
            
            stmt.execute("INSERT INTO customer VALUES ('john_doe', 'John Doe', 'john@example.com', '555-0101')");
            stmt.execute("INSERT INTO customer VALUES ('jane_smith', 'Jane Smith', 'jane@example.com', '555-0102')");
            stmt.execute("INSERT INTO customer VALUES ('bob_wilson', 'Bob Wilson', 'bob@example.com', '555-0103')");
            stmt.execute("INSERT INTO customer VALUES ('alice_brown', 'Alice Brown', 'alice@example.com', '555-0104')");
            stmt.execute("INSERT INTO customer VALUES ('charlie_davis', 'Charlie Davis', 'charlie@example.com', '555-0105')");
            
        } catch (SQLException e) {
            System.err.println("Database initialization error: " + e.getMessage());
        }
    }
    
    public static void main(String[] args) {
        initializeDatabase();
        
        String[] testCases = {"john_doe", "jane_smith", "bob_wilson", "nonexistent_user", ""};
        
        if (args.length > 0) {
            Customer customer = getCustomerByUsername(args[0]);
            if (customer != null) {
                System.out.println("Found: " + customer);
            } else {
                System.out.println("Customer not found: " + args[0]);
            }
        } else {
            System.out.println("Running test cases:");
            for (String username : testCases) {
                System.out.println("\\nSearching for: '" + username + "'");
                Customer customer = getCustomerByUsername(username);
                if (customer != null) {
                    System.out.println("Result: " + customer);
                } else {
                    System.out.println("Result: Customer not found");
                }
            }
        }
    }
}

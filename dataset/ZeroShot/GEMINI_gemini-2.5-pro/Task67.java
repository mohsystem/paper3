import java.sql.*;
import java.util.ArrayList;
import java.util.List;

class Customer {
    private String username;
    private String firstName;
    private String lastName;
    private String email;

    public Customer(String username, String firstName, String lastName, String email) {
        this.username = username;
        this.firstName = firstName;
        this.lastName = lastName;
        this.email = email;
    }

    @Override
    public String toString() {
        return "Customer{" +
                "username='" + username + '\'' +
                ", firstName='" + firstName + '\'' +
                ", lastName='" + lastName + '\'' +
                ", email='" + email + '\'' +
                '}';
    }
}

public class Task67 {

    private static final String DB_URL = "jdbc:sqlite:customers.db";

    /**
     * Sets up the database, creates the customer table, and populates it with sample data.
     * This method is for demonstration purposes.
     */
    public static void setupDatabase() {
        String createTableSQL = "CREATE TABLE IF NOT EXISTS customer ("
                + " customer_id INTEGER PRIMARY KEY AUTOINCREMENT,"
                + " customerusername TEXT NOT NULL UNIQUE,"
                + " first_name TEXT NOT NULL,"
                + " last_name TEXT NOT NULL,"
                + " email TEXT NOT NULL UNIQUE"
                + ");";
        
        String cleanTableSQL = "DELETE FROM customer;";

        try (Connection conn = DriverManager.getConnection(DB_URL);
             Statement stmt = conn.createStatement()) {
            stmt.execute(createTableSQL);
            stmt.execute(cleanTableSQL);

            // Insert sample data
            String[] inserts = {
                "INSERT INTO customer(customerusername, first_name, last_name, email) VALUES('jdoe', 'John', 'Doe', 'john.doe@example.com');",
                "INSERT INTO customer(customerusername, first_name, last_name, email) VALUES('asmith', 'Anna', 'Smith', 'anna.smith@example.com');",
                "INSERT INTO customer(customerusername, first_name, last_name, email) VALUES('sjobs', 'Steve', 'Jobs', 's.jobs@example.com');",
                "INSERT INTO customer(customerusername, first_name, last_name, email) VALUES('bgates', 'Bill', 'Gates', 'bill.g@example.com');"
            };

            for (String insert : inserts) {
                stmt.execute(insert);
            }

        } catch (SQLException e) {
            System.err.println("Database setup error: " + e.getMessage());
        }
    }

    /**
     * Retrieves a customer from the database by their username using a prepared statement to prevent SQL injection.
     * @param username The username of the customer to retrieve.
     * @return a Customer object if found, otherwise null.
     */
    public static Customer getCustomerByUsername(String username) {
        // The SQL query with a '?' placeholder for the parameter.
        String sql = "SELECT customerusername, first_name, last_name, email FROM customer WHERE customerusername = ?";
        Customer customer = null;

        // Using try-with-resources to ensure the connection and statement are closed automatically.
        try (Connection conn = DriverManager.getConnection(DB_URL);
             PreparedStatement pstmt = conn.prepareStatement(sql)) {

            // Bind the username parameter to the placeholder in the prepared statement.
            // This is the key to preventing SQL injection.
            pstmt.setString(1, username);

            try (ResultSet rs = pstmt.executeQuery()) {
                // Check if a record was found
                if (rs.next()) {
                    customer = new Customer(
                            rs.getString("customerusername"),
                            rs.getString("first_name"),
                            rs.getString("last_name"),
                            rs.getString("email")
                    );
                }
            }
        } catch (SQLException e) {
            System.err.println("Query Error: " + e.getMessage());
        }
        return customer;
    }

    public static void main(String[] args) {
        // NOTE: To run this code, you need the SQLite JDBC driver.
        // Download it and add it to your classpath. e.g., for sqlite-jdbc-3.36.0.3.jar:
        // javac Task67.java
        // java -cp ".:sqlite-jdbc-3.36.0.3.jar" Task67
        
        setupDatabase();
        
        List<String> testUsernames = new ArrayList<>();

        if (args.length > 0) {
            // Use command-line argument if provided
            System.out.println("--- Running with provided command-line argument ---");
            testUsernames.add(args[0]);
        } else {
            // Otherwise, run the 5 built-in test cases
            System.out.println("--- Running 5 built-in test cases ---");
            testUsernames.add("jdoe");                 // Test Case 1: Existing user
            testUsernames.add("asmith");               // Test Case 2: Another existing user
            testUsernames.add("nonexistent");          // Test Case 3: Non-existent user
            testUsernames.add("sjobs");                // Test Case 4: A third existing user
            testUsernames.add("' OR 1=1; --");         // Test Case 5: SQL injection attempt (should fail safely)
        }

        for (String username : testUsernames) {
            System.out.println("\nSearching for username: '" + username + "'");
            Customer cust = getCustomerByUsername(username);
            if (cust != null) {
                System.out.println("Found: " + cust);
            } else {
                System.out.println("Result: Customer not found.");
            }
        }
    }
}
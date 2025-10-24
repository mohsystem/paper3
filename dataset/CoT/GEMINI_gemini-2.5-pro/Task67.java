import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;

// The main class for the task
public class Task67 {

    // A simple Customer data class
    static class Customer {
        private int id;
        private String username;
        private String fullName;
        private String email;

        public Customer(int id, String username, String fullName, String email) {
            this.id = id;
            this.username = username;
            this.fullName = fullName;
            this.email = email;
        }

        @Override
        public String toString() {
            return "Customer [ID=" + id + ", Username=" + username + ", FullName=" + fullName + ", Email=" + email + "]";
        }
    }

    // In-memory H2 database connection URL
    private static final String DB_URL = "jdbc:h2:mem:customerdb;DB_CLOSE_DELAY=-1";

    /**
     * Retrieves a customer from the database by their username using a secure parameterized query.
     *
     * @param conn The database connection object.
     * @param customerUsername The username of the customer to find.
     * @return A Customer object if found, otherwise null.
     */
    public static Customer getCustomerByUsername(Connection conn, String customerUsername) {
        // SQL query with a placeholder (?) to prevent SQL injection
        String sql = "SELECT id, username, fullname, email FROM customer WHERE username = ?";
        Customer customer = null;

        // Using try-with-resources to ensure PreparedStatement and ResultSet are closed automatically
        try (PreparedStatement pstmt = conn.prepareStatement(sql)) {
            
            // Bind the customerUsername parameter to the query's placeholder
            pstmt.setString(1, customerUsername);

            try (ResultSet rs = pstmt.executeQuery()) {
                // If a record is found, create a Customer object
                if (rs.next()) {
                    customer = new Customer(
                        rs.getInt("id"),
                        rs.getString("username"),
                        rs.getString("fullname"),
                        rs.getString("email")
                    );
                }
            }
        } catch (SQLException e) {
            System.err.println("Error querying for customer: " + e.getMessage());
        }
        return customer;
    }

    /**
     * Sets up the in-memory database: creates the table and inserts sample data.
     * @param conn The database connection object.
     */
    private static void setupDatabase(Connection conn) {
        try (Statement stmt = conn.createStatement()) {
            // Create customer table
            stmt.execute("CREATE TABLE customer (" +
                         "id INT PRIMARY KEY, " +
                         "username VARCHAR(255) NOT NULL UNIQUE, " +
                         "fullname VARCHAR(255), " +
                         "email VARCHAR(255))");
            
            // Insert sample data
            stmt.execute("INSERT INTO customer VALUES (1, 'jdoe', 'John Doe', 'john.doe@example.com')");
            stmt.execute("INSERT INTO customer VALUES (2, 'asmith', 'Alice Smith', 'alice.smith@example.com')");
            stmt.execute("INSERT INTO customer VALUES (3, 'bwhite', 'Bob White', 'bob.white@example.com')");
            stmt.execute("INSERT INTO customer VALUES (4, 'peterpan', 'Peter Pan', 'peter.pan@example.com')");
        } catch (SQLException e) {
            System.err.println("Database setup failed: " + e.getMessage());
        }
    }

    public static void main(String[] args) {
        // NOTE: H2 database driver (e.g., com.h2database:h2) must be in the classpath.
        try (Connection conn = DriverManager.getConnection(DB_URL)) {
            setupDatabase(conn);

            if (args.length > 0) {
                // If a command-line argument is provided, use it
                System.out.println("Searching for username from command line: " + args[0]);
                Customer customer = getCustomerByUsername(conn, args[0]);
                if (customer != null) {
                    System.out.println("Found: " + customer);
                } else {
                    System.out.println("Customer with username '" + args[0] + "' not found.");
                }
            } else {
                // Otherwise, run predefined test cases
                System.out.println("--- Running Predefined Test Cases ---");
                String[] testUsernames = {
                    "jdoe",             // Test Case 1: Existing user
                    "asmith",           // Test Case 2: Existing user
                    "nonexistent",      // Test Case 3: Non-existing user
                    "peterpan",         // Test Case 4: Existing user
                    "admin' OR '1'='1" // Test Case 5: SQL injection attempt
                };

                for (String username : testUsernames) {
                    System.out.println("\nSearching for username: " + username);
                    Customer customer = getCustomerByUsername(conn, username);
                    if (customer != null) {
                        System.out.println("Found: " + customer);
                    } else {
                        System.out.println("Customer with username '" + username + "' not found.");
                    }
                }
            }
        } catch (SQLException e) {
            System.err.println("Database connection error: " + e.getMessage());
        }
    }
}
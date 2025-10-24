import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.Objects;
import java.util.Optional;
import java.util.regex.Pattern;

// To compile and run this code, you need the SQLite JDBC driver.
// Download it from: https://github.com/xerial/sqlite-jdbc
// Then compile and run with:
// javac Task67.java
// java -cp ".:sqlite-jdbc-3.36.0.3.jar" Task67 [username]
// (Replace the .jar filename with the one you downloaded)

public class Task67 {

    private static final String DB_URL = "jdbc:sqlite::memory:";
    // Simple username validation: Alphanumeric, 3-20 characters.
    private static final Pattern USERNAME_PATTERN = Pattern.compile("^[a-zA-Z0-9]{3,20}$");

    // Customer data class
    static class Customer {
        private final int id;
        private final String username;
        private final String firstName;
        private final String lastName;
        private final String email;

        public Customer(int id, String username, String firstName, String lastName, String email) {
            this.id = id;
            this.username = username;
            this.firstName = firstName;
            this.lastName = lastName;
            this.email = email;
        }

        @Override
        public String toString() {
            return "Customer{" +
                    "id=" + id +
                    ", username='" + username + '\'' +
                    ", firstName='" + firstName + '\'' +
                    ", lastName='" + lastName + '\'' +
                    ", email='" + email + '\'' +
                    '}';
        }

        @Override
        public boolean equals(Object o) {
            if (this == o) return true;
            if (o == null || getClass() != o.getClass()) return false;
            Customer customer = (Customer) o;
            return id == customer.id &&
                   Objects.equals(username, customer.username) &&
                   Objects.equals(firstName, customer.firstName) &&
                   Objects.equals(lastName, customer.lastName) &&
                   Objects.equals(email, customer.email);
        }
    }

    /**
     * Sets up the database with a customer table and sample data.
     * @param conn The database connection.
     * @throws SQLException if a database access error occurs.
     */
    private static void setupDatabase(Connection conn) throws SQLException {
        String createTableSQL = "CREATE TABLE IF NOT EXISTS customer ("
                + "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                + "username TEXT NOT NULL UNIQUE,"
                + "first_name TEXT NOT NULL,"
                + "last_name TEXT NOT NULL,"
                + "email TEXT NOT NULL UNIQUE"
                + ");";

        String[] insertSQLs = {
                "INSERT INTO customer(username, first_name, last_name, email) VALUES('jdoe', 'John', 'Doe', 'john.doe@example.com');",
                "INSERT INTO customer(username, first_name, last_name, email) VALUES('asmith', 'Alice', 'Smith', 'alice.smith@example.com');",
                "INSERT INTO customer(username, first_name, last_name, email) VALUES('bwilliams', 'Bob', 'Williams', 'bob.williams@example.com');",
                "INSERT INTO customer(username, first_name, last_name, email) VALUES('mjones', 'Mary', 'Jones', 'mary.jones@example.com');"
        };
        
        try (Statement stmt = conn.createStatement()) {
            stmt.execute(createTableSQL);
            for (String sql : insertSQLs) {
                stmt.execute(sql);
            }
        }
    }

    /**
     * Retrieves customer information for a given username.
     * @param username The username to search for.
     * @return An Optional containing the Customer if found, otherwise an empty Optional.
     */
    public static Optional<Customer> getCustomerByUsername(String username) {
        if (username == null || !USERNAME_PATTERN.matcher(username).matches()) {
            System.err.println("Invalid username format provided.");
            return Optional.empty();
        }

        String sql = "SELECT id, username, first_name, last_name, email FROM customer WHERE username = ?";
        
        try (Connection conn = DriverManager.getConnection(DB_URL);
             PreparedStatement pstmt = conn.prepareStatement(sql)) {
            
            // This setup is for demonstration purposes. In a real application,
            // the database would be persistent and set up separately.
            if (conn.getMetaData().getTables(null, null, "customer", null).next() == false) {
                 setupDatabase(conn);
            }
            
            pstmt.setString(1, username);
            
            try (ResultSet rs = pstmt.executeQuery()) {
                if (rs.next()) {
                    Customer customer = new Customer(
                        rs.getInt("id"),
                        rs.getString("username"),
                        rs.getString("first_name"),
                        rs.getString("last_name"),
                        rs.getString("email")
                    );
                    return Optional.of(customer);
                }
            }
        } catch (SQLException e) {
            System.err.println("Database error: " + e.getMessage());
        }
        
        return Optional.empty();
    }
    
    public static void main(String[] args) {
        // Setup a single in-memory DB for all tests
        try (Connection conn = DriverManager.getConnection(DB_URL)) {
             setupDatabase(conn);
        } catch (SQLException e) {
             System.err.println("Failed to setup in-memory database for tests: " + e.getMessage());
             return;
        }


        if (args.length > 0) {
            System.out.println("Searching for username from command line argument: " + args[0]);
            Optional<Customer> customer = getCustomerByUsername(args[0]);
            customer.ifPresentOrElse(
                c -> System.out.println("Found: " + c),
                () -> System.out.println("Customer not found.")
            );
        } else {
            System.out.println("Running test cases...");
            String[] testUsernames = {"jdoe", "asmith", "bwilliams", "enonexistent", "mjones"};
            for (int i = 0; i < testUsernames.length; i++) {
                String username = testUsernames[i];
                System.out.println("\n--- Test Case " + (i + 1) + ": Searching for username '" + username + "' ---");
                Optional<Customer> customer = getCustomerByUsername(username);
                customer.ifPresentOrElse(
                    c -> System.out.println("Found: " + c),
                    () -> System.out.println("Customer not found.")
                );
            }
        }
    }
}
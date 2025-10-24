import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;

/*
Note: To compile and run this Java code, you need the SQLite JDBC driver.
1. Download the driver JAR file (e.g., 'sqlite-jdbc-x.x.x.jar') from https://github.com/xerial/sqlite-jdbc/releases
2. Compile: javac Task67.java
3. Run with the driver in the classpath: java -cp ".;sqlite-jdbc-x.x.x.jar" Task67 [optional_username]
   (On Linux/macOS, use ':' instead of ';': java -cp ".:sqlite-jdbc-x.x.x.jar" Task67 [optional_username])
*/
public class Task67 {

    private static final String DB_URL = "jdbc:sqlite:customers.db";

    // Inner class to represent a Customer
    public static class Customer {
        private int id;
        private String username;
        private String firstName;
        private String lastName;
        private String email;

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
    }

    // Function to set up the database and insert sample data
    public static void setupDatabase() {
        try (Connection conn = DriverManager.getConnection(DB_URL);
             Statement stmt = conn.createStatement()) {

            String createTableSql = "CREATE TABLE IF NOT EXISTS customer (" +
                                    "customerid INTEGER PRIMARY KEY AUTOINCREMENT, " +
                                    "customerusername TEXT NOT NULL UNIQUE, " +
                                    "firstname TEXT, " +
                                    "lastname TEXT, " +
                                    "email TEXT);";
            stmt.execute(createTableSql);

            // Use INSERT OR IGNORE to avoid errors on subsequent runs
            stmt.execute("INSERT OR IGNORE INTO customer(customerusername, firstname, lastname, email) VALUES('jdoe', 'John', 'Doe', 'john.doe@email.com');");
            stmt.execute("INSERT OR IGNORE INTO customer(customerusername, firstname, lastname, email) VALUES('asmith', 'Alice', 'Smith', 'alice.s@web.com');");
            stmt.execute("INSERT OR IGNORE INTO customer(customerusername, firstname, lastname, email) VALUES('bwhite', 'Bob', 'White', 'bwhite@mail.org');");
            stmt.execute("INSERT OR IGNORE INTO customer(customerusername, firstname, lastname, email) VALUES('cjones', 'Carol', 'Jones', 'carolj@email.com');");

        } catch (SQLException e) {
            System.err.println("Database setup failed: " + e.getMessage());
        }
    }

    /**
     * Retrieves customer information based on username.
     * @param customerUsername The username to search for.
     * @return A Customer object if found, otherwise null.
     */
    public static Customer getCustomerByUsername(String customerUsername) {
        String sql = "SELECT customerid, customerusername, firstname, lastname, email " +
                     "FROM customer WHERE customerusername = ?";
        Customer customer = null;

        try (Connection conn = DriverManager.getConnection(DB_URL);
             PreparedStatement pstmt = conn.prepareStatement(sql)) {

            pstmt.setString(1, customerUsername);
            ResultSet rs = pstmt.executeQuery();

            if (rs.next()) {
                customer = new Customer(
                    rs.getInt("customerid"),
                    rs.getString("customerusername"),
                    rs.getString("firstname"),
                    rs.getString("lastname"),
                    rs.getString("email")
                );
            }
        } catch (SQLException e) {
            System.err.println("Query failed: " + e.getMessage());
        }
        return customer;
    }

    public static void main(String[] args) {
        // Setup the database on start
        setupDatabase();
        
        System.out.println("--- Running 5 Test Cases ---");
        String[] testUsernames = {"asmith", "jdoe", "nonexistent", "bwhite", "imnotreal"};
        for (int i = 0; i < testUsernames.length; i++) {
            System.out.println("\nTest Case " + (i + 1) + ": Searching for '" + testUsernames[i] + "'");
            Customer cust = getCustomerByUsername(testUsernames[i]);
            if (cust != null) {
                System.out.println("Found: " + cust);
            } else {
                System.out.println("Customer not found.");
            }
        }
        System.out.println("----------------------------\n");

        if (args.length > 0) {
            String usernameFromArg = args[0];
            System.out.println("--- Running with Command-Line Argument ---");
            System.out.println("Searching for '" + usernameFromArg + "'");
            Customer cust = getCustomerByUsername(usernameFromArg);
            if (cust != null) {
                System.out.println("Found: " + cust);
            } else {
                System.out.println("Customer not found.");
            }
            System.out.println("------------------------------------------");
        }
    }
}
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.SQLException;

public class Task74 {

    /**
     * Creates and returns a connection to a PostgreSQL database.
     * For security, this method uses SSL by default.
     *
     * @param host     The database server host.
     * @param port     The database server port.
     * @param dbName   The name of the database.
     * @param user     The username for authentication.
     * @param password The password for authentication.
     * @return A Connection object or null if the connection fails.
     */
    public static Connection getPostgresConnection(String host, String port, String dbName, String user, String password) {
        // Note: The PostgreSQL JDBC driver JAR must be in the classpath.
        // Example: postgresql-42.5.0.jar
        
        Connection connection = null;
        // Secure connection string with SSL enabled
        String url = String.format("jdbc:postgresql://%s:%s/%s?ssl=true&sslmode=require", host, port, dbName);

        try {
            // Load the PostgreSQL JDBC driver
            Class.forName("org.postgresql.Driver");
            // Attempt to establish a connection
            connection = DriverManager.getConnection(url, user, password);
            System.out.println("Connection to " + dbName + " established successfully.");
        } catch (ClassNotFoundException e) {
            System.err.println("PostgreSQL JDBC Driver not found. Please include it in your classpath.");
            // e.printStackTrace();
        } catch (SQLException e) {
            System.err.println("Connection Failed! Check output console.");
            System.err.println("SQLState: " + e.getSQLState());
            System.err.println("Error Code: " + e.getErrorCode());
            System.err.println("Message: " + e.getMessage());
            // e.printStackTrace();
        }
        return connection;
    }

    public static void main(String[] args) {
        // --- Test Cases ---
        // These are placeholder credentials and will likely fail to connect.
        // Replace with your actual database details to test a successful connection.

        // Test Case 1: Valid-looking but likely incorrect credentials
        System.out.println("--- Test Case 1 ---");
        try (Connection conn1 = getPostgresConnection("localhost", "5432", "mydatabase", "myuser", "mypassword")) {
            if (conn1 != null) {
                System.out.println("Test Case 1: Connection object returned.");
            } else {
                System.out.println("Test Case 1: Failed to get connection object.");
            }
        } catch (SQLException e) {
            System.err.println("Test Case 1: Error closing connection: " + e.getMessage());
        }
        System.out.println();

        // Test Case 2: Invalid host
        System.out.println("--- Test Case 2 ---");
        try (Connection conn2 = getPostgresConnection("invalid-host", "5432", "testdb", "user2", "pass2")) {
            if (conn2 != null) {
                System.out.println("Test Case 2: Connection object returned.");
            } else {
                System.out.println("Test Case 2: Failed to get connection object.");
            }
        } catch (SQLException e) {
            System.err.println("Test Case 2: Error closing connection: " + e.getMessage());
        }
        System.out.println();

        // Test Case 3: Invalid port
        System.out.println("--- Test Case 3 ---");
        try (Connection conn3 = getPostgresConnection("localhost", "9999", "proddb", "user3", "pass3")) {
            if (conn3 != null) {
                System.out.println("Test Case 3: Connection object returned.");
            } else {
                System.out.println("Test Case 3: Failed to get connection object.");
            }
        } catch (SQLException e) {
            System.err.println("Test Case 3: Error closing connection: " + e.getMessage());
        }
        System.out.println();


        // Test Case 4: Null password
        System.out.println("--- Test Case 4 ---");
        try (Connection conn4 = getPostgresConnection("localhost", "5432", "analytics", "user4", null)) {
            if (conn4 != null) {
                System.out.println("Test Case 4: Connection object returned.");
            } else {
                System.out.println("Test Case 4: Failed to get connection object.");
            }
        } catch (SQLException e) {
            System.err.println("Test Case 4: Error closing connection: " + e.getMessage());
        }
        System.out.println();

        // Test Case 5: Empty parameters
        System.out.println("--- Test Case 5 ---");
        try (Connection conn5 = getPostgresConnection("", "", "", "", "")) {
            if (conn5 != null) {
                System.out.println("Test Case 5: Connection object returned.");
            } else {
                System.out.println("Test Case 5: Failed to get connection object.");
            }
        } catch (SQLException e) {
            System.err.println("Test Case 5: Error closing connection: " + e.getMessage());
        }
        System.out.println();
    }
}
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.SQLException;

public class Task74 {

    /**
     * Creates and returns a connection to a PostgreSQL database.
     * Note: You need to have the PostgreSQL JDBC driver in your classpath.
     * Example Maven dependency:
     * <dependency>
     *     <groupId>org.postgresql</groupId>
     *     <artifactId>postgresql</artifactId>
     *     <version>42.6.0</version>
     * </dependency>
     *
     * @param host     The database server host.
     * @param port     The database server port.
     * @param dbName   The name of the database.
     * @param user     The username for authentication.
     * @param password The password for authentication.
     * @return A Connection object or null if connection fails.
     */
    public static Connection createConnection(String host, int port, String dbName, String user, String password) {
        Connection connection = null;
        try {
            String url = "jdbc:postgresql://" + host + ":" + port + "/" + dbName;
            connection = DriverManager.getConnection(url, user, password);
        } catch (SQLException e) {
            // In a real application, you would log this error.
            // e.printStackTrace();
            return null;
        }
        return connection;
    }

    public static void main(String[] args) {
        // --- Test Cases ---

        // Test Case 1: Standard localhost connection (likely to fail without a running DB and correct credentials)
        runTestCase(1, "localhost", 5432, "testdb", "user", "password");

        // Test Case 2: Using IP address for localhost
        runTestCase(2, "127.0.0.1", 5432, "postgres", "postgres", "admin123");

        // Test Case 3: Different database and user
        runTestCase(3, "localhost", 5432, "mydatabase", "dbuser", "secret");

        // Test Case 4: Invalid host to simulate connection failure
        runTestCase(4, "nonexistent.host.com", 5432, "testdb", "user", "password");

        // Test Case 5: Invalid port
        runTestCase(5, "localhost", 9999, "testdb", "user", "password");
    }

    private static void runTestCase(int caseNum, String host, int port, String dbName, String user, String password) {
        System.out.println("--- Test Case " + caseNum + " ---");
        System.out.printf("Attempting to connect to postgresql://%s@%s:%d/%s%n", user, host, port, dbName);
        Connection conn = createConnection(host, port, dbName, user, password);
        if (conn != null) {
            System.out.println("Connection successful!");
            try {
                conn.close();
                System.out.println("Connection closed.");
            } catch (SQLException e) {
                System.err.println("Failed to close connection: " + e.getMessage());
            }
        } else {
            System.out.println("Connection failed.");
        }
        System.out.println();
    }
}
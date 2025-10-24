import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.SQLException;

/*
 * To compile and run this code, you need the PostgreSQL JDBC driver.
 * Download the JAR file (e.g., postgresql-42.x.x.jar) and include it in your classpath.
 * Example compilation and run:
 * javac Task74.java
 * java -cp .:postgresql-42.x.x.jar Task74
 */
public class Task74 {

    /**
     * Establishes a connection to a PostgreSQL database.
     *
     * @param url      The database URL in the format "jdbc:postgresql://host:port/dbname".
     * @param user     The username for the database connection.
     * @param password The password for the database connection.
     * @return A Connection object or null if the connection fails.
     *
     * Security Note: In a real application, credentials should be sourced securely
     * (e.g., from environment variables, a secrets manager) and not hardcoded.
     * Always use PreparedStatement with the returned connection to prevent SQL injection.
     */
    public static Connection connectToPostgres(String url, String user, String password) {
        Connection conn = null;
        try {
            // The driver is usually registered automatically with modern JDBC,
            // but Class.forName is a good practice for ensuring it's loaded.
            Class.forName("org.postgresql.Driver");
            conn = DriverManager.getConnection(url, user, password);
        } catch (SQLException e) {
            System.err.println("SQL Exception: " + e.getMessage());
            // In a real app, use a logger instead of printing to stderr.
        } catch (ClassNotFoundException e) {
            System.err.println("PostgreSQL JDBC Driver not found. Include it in your library path.");
            e.printStackTrace();
        }
        return conn;
    }

    public static void main(String[] args) {
        // --- IMPORTANT ---
        // Replace these placeholder values with your actual PostgreSQL database details.
        // For the test cases to work as expected, you need a running PostgreSQL server.
        final String HOST = "localhost";
        final String PORT = "5432";
        final String DBNAME = "your_db";
        final String USER = "your_user";
        final String PASSWORD = "your_password";

        // Test Case 1: Successful Connection
        System.out.println("--- Test Case 1: Attempting a successful connection... ---");
        String successUrl = String.format("jdbc:postgresql://%s:%s/%s", HOST, PORT, DBNAME);
        Connection conn1 = connectToPostgres(successUrl, USER, PASSWORD);
        if (conn1 != null) {
            System.out.println("Test Case 1: Connection successful!");
            try {
                conn1.close();
                System.out.println("Test Case 1: Connection closed.");
            } catch (SQLException e) {
                e.printStackTrace();
            }
        } else {
            System.out.println("Test Case 1: Connection failed. Please check your credentials and database status.");
        }
        System.out.println();

        // Test Case 2: Incorrect Password
        System.out.println("--- Test Case 2: Attempting connection with a wrong password... ---");
        Connection conn2 = connectToPostgres(successUrl, USER, "wrong_password");
        if (conn2 == null) {
            System.out.println("Test Case 2: Connection failed as expected.");
        } else {
            System.out.println("Test Case 2: Connection unexpectedly succeeded.");
            try {
                conn2.close();
            } catch (SQLException e) {
                e.printStackTrace();
            }
        }
        System.out.println();

        // Test Case 3: Incorrect Database Name
        System.out.println("--- Test Case 3: Attempting connection to a non-existent database... ---");
        String badDbUrl = String.format("jdbc:postgresql://%s:%s/non_existent_db", HOST, PORT);
        Connection conn3 = connectToPostgres(badDbUrl, USER, PASSWORD);
        if (conn3 == null) {
            System.out.println("Test Case 3: Connection failed as expected.");
        } else {
            System.out.println("Test Case 3: Connection unexpectedly succeeded.");
            try {
                conn3.close();
            } catch (SQLException e) {
                e.printStackTrace();
            }
        }
        System.out.println();
        
        // Test Case 4: Incorrect User
        System.out.println("--- Test Case 4: Attempting connection with a wrong user... ---");
        Connection conn4 = connectToPostgres(successUrl, "wrong_user", PASSWORD);
        if (conn4 == null) {
            System.out.println("Test Case 4: Connection failed as expected.");
        } else {
            System.out.println("Test Case 4: Connection unexpectedly succeeded.");
            try {
                conn4.close();
            } catch (SQLException e) {
                e.printStackTrace();
            }
        }
        System.out.println();

        // Test Case 5: Incorrect Host
        System.out.println("--- Test Case 5: Attempting connection to a non-existent host... ---");
        String badHostUrl = String.format("jdbc:postgresql://nonexistenthost:%s/%s", PORT, DBNAME);
        Connection conn5 = connectToPostgres(badHostUrl, USER, PASSWORD);
        if (conn5 == null) {
            System.out.println("Test Case 5: Connection failed as expected.");
        } else {
            System.out.println("Test Case 5: Connection unexpectedly succeeded.");
            try {
                conn5.close();
            } catch (SQLException e) {
                e.printStackTrace();
            }
        }
        System.out.println();
    }
}
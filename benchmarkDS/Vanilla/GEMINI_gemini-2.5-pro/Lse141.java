import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.SQLException;

// A simple representation of a Web App and its configuration.
// A real web app would use a framework like Spring Boot or Jakarta EE.
public class Lse141 {

    // Database configuration constants
    private static final String SERVER_NAME = "localhost:5432";
    private static final String DATABASE_NAME = "mydatabase";
    private static final String DATABASE_USER = "myuser";
    private static final String DATABASE_PASSWORD = "mypassword";

    /**
     * Returns the server name.
     * @return The database server name and port.
     */
    public String getServerName() {
        return SERVER_NAME;
    }

    /**
     * Returns the database name.
     * @return The name of the database.
     */
    public String getDatabaseName() {
        return DATABASE_NAME;
    }

    /**
     * Returns the database user.
     * @return The username for the database connection.
     */
    public String getDatabaseUser() {
        return DATABASE_USER;
    }

    /**
     * Returns the database password.
     * @return The password for the database user.
     */
    public String getDatabasePassword() {
        return DATABASE_PASSWORD;
    }

    /**
     * Attempts to return a database connection.
     * Note: This is a placeholder. For a real connection, you would need the
     * appropriate JDBC driver (e.g., for PostgreSQL, MySQL) in your classpath.
     * @return A Connection object or null if connection fails.
     */
    public Connection getDatabaseConnection() {
        String jdbcUrl = "jdbc:postgresql://" + getServerName() + "/" + getDatabaseName();
        try {
            // This line would throw ClassNotFoundException without the driver
            // Class.forName("org.postgresql.Driver");
            // This line would throw SQLException without a running DB
            // return DriverManager.getConnection(jdbcUrl, getDatabaseUser(), getDatabasePassword());
            System.out.println("Simulating connection to: " + jdbcUrl);
            return null; // Returning null as we don't have a real DB/driver setup.
        } catch (Exception e) {
            System.err.println("Connection failed: " + e.getMessage());
            return null;
        }
    }

    public static void main(String[] args) {
        Lse141 webApp = new Lse141();

        // 5 Test Cases
        System.out.println("--- Test Case 1: Get Server Name ---");
        System.out.println("Server: " + webApp.getServerName());
        System.out.println();

        System.out.println("--- Test Case 2: Get Database Name ---");
        System.out.println("Database: " + webApp.getDatabaseName());
        System.out.println();

        System.out.println("--- Test Case 3: Get User Credentials ---");
        System.out.println("User: " + webApp.getDatabaseUser());
        System.out.println("Password: " + webApp.getDatabasePassword());
        System.out.println();
        
        System.out.println("--- Test Case 4: Get Full Config Summary ---");
        System.out.printf("Server: %s, DB: %s, User: %s%n",
                webApp.getServerName(), webApp.getDatabaseName(), webApp.getDatabaseUser());
        System.out.println();

        System.out.println("--- Test Case 5: Attempt Database Connection ---");
        Connection conn = webApp.getDatabaseConnection();
        if (conn == null) {
            System.out.println("Connection object is null (as expected in this simulation).");
        } else {
            System.out.println("Connection successful (unexpected).");
        }
    }
}
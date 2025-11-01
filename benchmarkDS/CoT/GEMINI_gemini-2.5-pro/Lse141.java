import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.SQLException;

// Note: To run this code, you need a JDBC driver (e.g., for MySQL or PostgreSQL) in your classpath.
// This code demonstrates the structure but a real connection requires a running database server.

public class Lse141 {

    /**
     * Creates a placeholder "web app" instance.
     * In a real Java web application, this would be handled by a framework
     * like Spring Boot, Jakarta EE, or SparkJava.
     */
    public static void createWebApp() {
        System.out.println("Web app created (placeholder).");
        // In a real scenario, you would initialize your web framework here.
    }

    /**
     * Returns the database server name.
     * It's recommended to set this via an environment variable for security.
     * @return The server name (e.g., "localhost").
     */
    public static String getServerName() {
        // Reads from environment variable 'DB_SERVER', defaults to 'localhost'
        String server = System.getenv("DB_SERVER");
        return (server != null && !server.isEmpty()) ? server : "localhost";
    }

    /**
     * Returns the database name.
     * It's recommended to set this via an environment variable.
     * @return The database name.
     */
    public static String getDatabaseName() {
        // Reads from environment variable 'DB_NAME', defaults to 'mydatabase'
        String dbName = System.getenv("DB_NAME");
        return (dbName != null && !dbName.isEmpty()) ? dbName : "mydatabase";
    }

    /**
     * Returns the database user.
     * It's a critical security practice to get credentials from a secure source.
     * @return The database username.
     */
    public static String getDatabaseUser() {
        // Reads from environment variable 'DB_USER', defaults to 'user'
        String user = System.getenv("DB_USER");
        return (user != null && !user.isEmpty()) ? user : "user";
    }

    /**
     * Returns the database password.
     * WARNING: Hardcoding passwords or using insecure defaults is a major vulnerability.
     * Always use environment variables or a secret management system.
     * @return The database password.
     */
    public static String getDatabasePassword() {
        // Reads from environment variable 'DB_PASSWORD', defaults to 'password'
        String password = System.getenv("DB_PASSWORD");
        return (password != null && !password.isEmpty()) ? password : "password";
    }

    /**
     * Attempts to establish a connection to the database.
     * @return A Connection object or null if the connection fails.
     */
    public static Connection getDatabaseConnection() {
        // Example for MySQL: jdbc:mysql://localhost:3306/mydatabase
        String url = "jdbc:mysql://" + getServerName() + ":3306/" + getDatabaseName();
        String user = getDatabaseUser();
        String password = getDatabasePassword();
        Connection conn = null;

        try {
            // Assumes MySQL Connector/J driver is on the classpath
            Class.forName("com.mysql.cj.jdbc.Driver");
            conn = DriverManager.getConnection(url, user, password);
            System.out.println("Database connection successful!");
        } catch (ClassNotFoundException e) {
            System.err.println("JDBC Driver not found. Please add it to your classpath.");
        } catch (SQLException e) {
            System.err.println("Database connection failed: " + e.getMessage());
        }
        return conn;
    }

    public static void main(String[] args) {
        // Create the web app (placeholder)
        createWebApp();
        System.out.println("\n--- Running 5 Test Cases ---");
        System.out.println("Note: Values may be defaults if environment variables (e.g., DB_SERVER) are not set.\n");
        
        // Test Case 1: Get Server Name
        System.out.println("Test Case 1: Get Server Name");
        System.out.println("Server Name: " + getServerName());
        System.out.println("---------------------------------");
        
        // Test Case 2: Get Database Name
        System.out.println("Test Case 2: Get Database Name");
        System.out.println("Database Name: " + getDatabaseName());
        System.out.println("---------------------------------");

        // Test Case 3: Get Database User
        System.out.println("Test Case 3: Get Database User");
        System.out.println("Database User: " + getDatabaseUser());
        System.out.println("---------------------------------");

        // Test Case 4: Get Database Password (displaying length for security)
        System.out.println("Test Case 4: Get Database Password");
        String pass = getDatabasePassword();
        System.out.println("Database Password Length: " + (pass != null ? pass.length() : 0));
        System.out.println("---------------------------------");

        // Test Case 5: Attempt Database Connection
        System.out.println("Test Case 5: Attempt Database Connection");
        // Using try-with-resources to ensure the connection is closed automatically
        try (Connection conn = getDatabaseConnection()) {
            if (conn != null) {
                System.out.println("Connection object received and closed successfully.");
            } else {
                System.out.println("Failed to receive connection object.");
            }
        } catch (SQLException e) {
            System.err.println("Error while closing the connection: " + e.getMessage());
        }
        System.out.println("---------------------------------");
    }
}
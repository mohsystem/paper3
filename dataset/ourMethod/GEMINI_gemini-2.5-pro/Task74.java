import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.SQLException;
import java.util.Properties;

/**
 * To compile and run this code, you need the PostgreSQL JDBC driver.
 * Download the JAR from https://jdbc.postgresql.org/ and include it in your classpath.
 * Example command:
 * javac Task74.java
 * java -cp .:/path/to/postgresql-42.x.x.jar Task74
 * 
 * Before running, set the following environment variables:
 * export PG_HOST="localhost"
 * export PG_PORT="5432"
 * export PG_DBNAME="testdb"
 * export PG_USER="testuser"
 * export PG_PASSWORD="testpassword"
 */
public class Task74 {

    /**
     * Creates and returns a connection to a PostgreSQL database.
     *
     * @param host     The database server host.
     * @param port     The database server port.
     * @param dbname   The name of the database.
     * @param user     The username for authentication.
     * @param password The password for authentication.
     * @return A Connection object or null if connection fails.
     */
    public static Connection getConnection(String host, String port, String dbname, String user, String password) {
        // Rule #13: Avoid hardcoding credentials. The credentials are passed as parameters.
        if (host == null || port == null || dbname == null || user == null || password == null) {
            System.err.println("Error: All connection parameters must be provided.");
            return null;
        }

        Connection connection = null;
        try {
            // Construct the JDBC URL. Using sslmode=require is a security best practice.
            String url = String.format("jdbc:postgresql://%s:%s/%s", host, port, dbname);
            
            Properties props = new Properties();
            props.setProperty("user", user);
            props.setProperty("password", password);
            props.setProperty("sslmode", "require"); // Rule #4, #5: Enforce SSL usage

            // Establish the connection
            connection = DriverManager.getConnection(url, props);
            System.out.println("Connection to " + dbname + " established successfully.");
        } catch (SQLException e) {
            // Rule #14: Handle exceptions gracefully without leaking sensitive information.
            System.err.println("Connection Failed: " + e.getMessage());
            // Avoid printing the stack trace in production for security reasons
            // e.printStackTrace();
        }
        return connection;
    }

    public static void main(String[] args) {
        // --- Test Cases ---
        System.out.println("--- Running Test Cases ---");

        // Test Case 1: Successful connection using environment variables
        System.out.println("\n[Test Case 1: Successful Connection]");
        String dbHost = System.getenv("PG_HOST");
        String dbPort = System.getenv("PG_PORT");
        String dbName = System.getenv("PG_DBNAME");
        String dbUser = System.getenv("PG_USER");
        String dbPassword = System.getenv("PG_PASSWORD");

        // Rule #16: Check that environment variables are set
        if (dbHost == null || dbPort == null || dbName == null || dbUser == null || dbPassword == null) {
            System.out.println("Skipping Test Case 1: Please set PG_HOST, PG_PORT, PG_DBNAME, PG_USER, and PG_PASSWORD environment variables.");
        } else {
             // Rule #14: Use try-with-resources to ensure the connection is closed
            try (Connection conn = getConnection(dbHost, dbPort, dbName, dbUser, dbPassword)) {
                if (conn != null) {
                    System.out.println("Connection object is valid.");
                } else {
                    System.out.println("Failed to get connection object.");
                }
            } catch (SQLException e) {
                System.err.println("Error managing connection resource: " + e.getMessage());
            }
        }

        // Test Case 2: Connection with a wrong password
        System.out.println("\n[Test Case 2: Wrong Password]");
        if (dbHost != null) {
           try (Connection conn = getConnection(dbHost, dbPort, dbName, dbUser, "wrongpassword")) {
              // This block will likely not be reached if connection fails
           } catch (SQLException e) {
                System.err.println("Error managing connection resource: " + e.getMessage());
           }
        } else {
            System.out.println("Skipping Test Case 2: Environment variables not set.");
        }


        // Test Case 3: Connection to a non-existent database
        System.out.println("\n[Test Case 3: Non-existent Database]");
        if (dbHost != null) {
            try (Connection conn = getConnection(dbHost, dbPort, "nonexistentdb", dbUser, dbPassword)) {
               // This block will likely not be reached
            } catch (SQLException e) {
                System.err.println("Error managing connection resource: " + e.getMessage());
            }
        } else {
            System.out.println("Skipping Test Case 3: Environment variables not set.");
        }

        // Test Case 4: Connection to a wrong host
        System.out.println("\n[Test Case 4: Wrong Host]");
        if (dbHost != null) {
            try (Connection conn = getConnection("127.0.0.99", dbPort, dbName, dbUser, dbPassword)) {
                // This block will likely not be reached
            } catch (SQLException e) {
                System.err.println("Error managing connection resource: " + e.getMessage());
            }
        } else {
            System.out.println("Skipping Test Case 4: Environment variables not set.");
        }
        
        // Test Case 5: Connection with a null user
        System.out.println("\n[Test Case 5: Null User]");
        if (dbHost != null) {
            try (Connection conn = getConnection(dbHost, dbPort, dbName, null, dbPassword)) {
                // This block will likely not be reached
            } catch (SQLException e) {
                 System.err.println("Error managing connection resource: " + e.getMessage());
            }
        } else {
            System.out.println("Skipping Test Case 5: Environment variables not set.");
        }
    }
}

import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.SQLException;

public class Lse141 {
    private static final String SERVER_NAME = "localhost";
    private static final String DATABASE_NAME = "myDatabase";
    private static final String DATABASE_USER = "root";
    private static final String DATABASE_PASSWORD = "password123";
    
    // Function that returns a database connection
    public static Connection getDatabaseConnection() {
        Connection connection = null;
        try {
            String url = "jdbc:mysql://" + getServerName() + "/" + getDatabaseName();
            connection = DriverManager.getConnection(url, getDatabaseUser(), getDatabasePassword());
        } catch (SQLException e) {
            System.err.println("Database connection failed: " + e.getMessage());
        }
        return connection;
    }
    
    // Function that returns the server name
    public static String getServerName() {
        return SERVER_NAME;
    }
    
    // Function that returns the database name
    public static String getDatabaseName() {
        return DATABASE_NAME;
    }
    
    // Function that returns the database user
    public static String getDatabaseUser() {
        return DATABASE_USER;
    }
    
    // Function that returns the database password
    public static String getDatabasePassword() {
        return DATABASE_PASSWORD;
    }
    
    public static void main(String[] args) {
        // Test case 1: Get server name
        System.out.println("Test 1 - Server Name: " + getServerName());
        
        // Test case 2: Get database name
        System.out.println("Test 2 - Database Name: " + getDatabaseName());
        
        // Test case 3: Get database user
        System.out.println("Test 3 - Database User: " + getDatabaseUser());
        
        // Test case 4: Get database password
        System.out.println("Test 4 - Database Password: " + getDatabasePassword());
        
        // Test case 5: Get database connection
        Connection conn = getDatabaseConnection();
        if (conn != null) {
            System.out.println("Test 5 - Database Connection: Successful");
            try {
                conn.close();
            } catch (SQLException e) {
                System.err.println("Error closing connection: " + e.getMessage());
            }
        } else {
            System.out.println("Test 5 - Database Connection: Failed");
        }
    }
}

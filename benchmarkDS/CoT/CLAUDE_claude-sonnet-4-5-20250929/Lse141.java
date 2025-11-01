
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.SQLException;
import java.util.Properties;

public class Lse141 {
    // Secure configuration using environment variables
    private static final String DB_SERVER = System.getenv("DB_SERVER");
    private static final String DB_NAME = System.getenv("DB_NAME");
    private static final String DB_USER = System.getenv("DB_USER");
    private static final String DB_PASSWORD = System.getenv("DB_PASSWORD");
    
    // Returns database connection with proper error handling
    public static Connection getDatabaseConnection() throws SQLException {
        if (DB_SERVER == null || DB_NAME == null || DB_USER == null || DB_PASSWORD == null) {
            throw new SQLException("Database configuration not properly set in environment variables");
        }
        
        String url = "jdbc:mysql://" + DB_SERVER + "/" + DB_NAME + "?useSSL=true&requireSSL=true";
        Properties props = new Properties();
        props.setProperty("user", DB_USER);
        props.setProperty("password", DB_PASSWORD);
        
        return DriverManager.getConnection(url, props);
    }
    
    // Returns server name from environment variable
    public static String getServerName() {
        return DB_SERVER != null ? DB_SERVER : "localhost";
    }
    
    // Returns database name from environment variable
    public static String getDatabaseName() {
        return DB_NAME != null ? DB_NAME : "defaultdb";
    }
    
    // Returns database user from environment variable
    public static String getDatabaseUser() {
        return DB_USER != null ? DB_USER : "root";
    }
    
    // WARNING: This should only be used internally, never expose password via API
    private static String getDatabasePassword() {
        return DB_PASSWORD != null ? DB_PASSWORD : "";
    }
    
    public static void main(String[] args) {
        // Test cases - Note: Set environment variables before running
        System.out.println("Test Case 1: Get Server Name");
        System.out.println("Server: " + getServerName());
        
        System.out.println("\\nTest Case 2: Get Database Name");
        System.out.println("Database: " + getDatabaseName());
        
        System.out.println("\\nTest Case 3: Get Database User");
        System.out.println("User: " + getDatabaseUser());
        
        System.out.println("\\nTest Case 4: Connection Info");
        System.out.println("Connection configured: " + 
            (DB_SERVER != null && DB_NAME != null && DB_USER != null && DB_PASSWORD != null));
        
        System.out.println("\\nTest Case 5: Attempt Database Connection");
        try {
            Connection conn = getDatabaseConnection();
            System.out.println("Connection successful!");
            conn.close();
        } catch (SQLException e) {
            System.out.println("Connection failed: " + e.getMessage());
        }
    }
}

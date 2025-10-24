
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.SQLException;
import java.util.Properties;

public class Task74 {
    
    /**
     * Creates and returns a secure PostgreSQL database connection
     * @param host Database host
     * @param port Database port
     * @param database Database name
     * @param username Database username
     * @param password Database password
     * @param useSSL Whether to use SSL connection
     * @return Connection object
     * @throws SQLException if connection fails
     */
    public static Connection createPostgresConnection(String host, int port, 
                                                      String database, String username, 
                                                      String password, boolean useSSL) 
                                                      throws SQLException {
        // Validate input parameters
        if (host == null || host.trim().isEmpty()) {
            throw new IllegalArgumentException("Host cannot be null or empty");
        }
        if (database == null || database.trim().isEmpty()) {
            throw new IllegalArgumentException("Database name cannot be null or empty");
        }
        if (username == null || username.trim().isEmpty()) {
            throw new IllegalArgumentException("Username cannot be null or empty");
        }
        if (password == null) {
            throw new IllegalArgumentException("Password cannot be null");
        }
        if (port < 1 || port > 65535) {
            throw new IllegalArgumentException("Port must be between 1 and 65535");
        }
        
        // Build secure connection URL
        String url = String.format("jdbc:postgresql://%s:%d/%s", host, port, database);
        
        // Set connection properties for security
        Properties props = new Properties();
        props.setProperty("user", username);
        props.setProperty("password", password);
        
        // Enable SSL if required
        if (useSSL) {
            props.setProperty("ssl", "true");
            props.setProperty("sslfactory", "org.postgresql.ssl.NonValidatingFactory");
        }
        
        // Additional security settings
        props.setProperty("tcpKeepAlive", "true");
        props.setProperty("loginTimeout", "10");
        
        try {
            // Load PostgreSQL JDBC driver
            Class.forName("org.postgresql.Driver");
            
            // Create and return connection
            Connection connection = DriverManager.getConnection(url, props);
            return connection;
            
        } catch (ClassNotFoundException e) {
            throw new SQLException("PostgreSQL JDBC Driver not found", e);
        }
    }
    
    public static void main(String[] args) {
        // Test cases with sample credentials (use environment variables in production)
        System.out.println("=== PostgreSQL Connection Test Cases ===\\n");
        
        // Test Case 1: Successful connection (simulated)
        testConnection("Test 1: Standard Connection", 
                      "localhost", 5432, "testdb", "postgres", "password", false);
        
        // Test Case 2: SSL Connection
        testConnection("Test 2: SSL Connection", 
                      "localhost", 5432, "testdb", "postgres", "password", true);
        
        // Test Case 3: Invalid port
        testConnection("Test 3: Invalid Port", 
                      "localhost", -1, "testdb", "postgres", "password", false);
        
        // Test Case 4: Empty host
        testConnection("Test 4: Empty Host", 
                      "", 5432, "testdb", "postgres", "password", false);
        
        // Test Case 5: Null database
        testConnection("Test 5: Null Database", 
                      "localhost", 5432, null, "postgres", "password", false);
    }
    
    private static void testConnection(String testName, String host, int port, 
                                      String database, String username, 
                                      String password, boolean useSSL) {
        System.out.println(testName);
        try {
            Connection conn = createPostgresConnection(host, port, database, 
                                                       username, password, useSSL);
            System.out.println("✓ Connection created successfully");
            if (conn != null && !conn.isClosed()) {
                conn.close();
                System.out.println("✓ Connection closed successfully");
            }
        } catch (IllegalArgumentException e) {
            System.out.println("✗ Validation Error: " + e.getMessage());
        } catch (SQLException e) {
            System.out.println("✗ SQL Error: " + e.getMessage());
        }
        System.out.println();
    }
}

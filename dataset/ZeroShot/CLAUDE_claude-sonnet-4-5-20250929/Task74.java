
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.SQLException;
import java.util.Properties;

public class Task74 {
    public static Connection createPostgresConnection(String host, int port, String database, 
                                                     String username, String password) {
        Connection connection = null;
        try {
            String url = String.format("jdbc:postgresql://%s:%d/%s", host, port, database);
            
            Properties props = new Properties();
            props.setProperty("user", username);
            props.setProperty("password", password);
            props.setProperty("ssl", "true");
            props.setProperty("sslmode", "require");
            
            connection = DriverManager.getConnection(url, props);
            
            if (connection != null) {
                System.out.println("Connected to PostgreSQL database successfully!");
            }
        } catch (SQLException e) {
            System.err.println("Connection failed: " + e.getMessage());
        }
        return connection;
    }
    
    public static void main(String[] args) {
        // Test case 1: Local connection
        Connection conn1 = createPostgresConnection("localhost", 5432, "testdb", "user1", "pass1");
        if (conn1 != null) {
            try { conn1.close(); } catch (SQLException e) { e.printStackTrace(); }
        }
        
        // Test case 2: Custom port
        Connection conn2 = createPostgresConnection("localhost", 5433, "mydb", "admin", "admin123");
        if (conn2 != null) {
            try { conn2.close(); } catch (SQLException e) { e.printStackTrace(); }
        }
        
        // Test case 3: Remote connection
        Connection conn3 = createPostgresConnection("192.168.1.100", 5432, "proddb", "dbuser", "securepass");
        if (conn3 != null) {
            try { conn3.close(); } catch (SQLException e) { e.printStackTrace(); }
        }
        
        // Test case 4: Different database
        Connection conn4 = createPostgresConnection("localhost", 5432, "analytics", "analyst", "analyze123");
        if (conn4 != null) {
            try { conn4.close(); } catch (SQLException e) { e.printStackTrace(); }
        }
        
        // Test case 5: Another local instance
        Connection conn5 = createPostgresConnection("127.0.0.1", 5432, "devdb", "developer", "dev@2024");
        if (conn5 != null) {
            try { conn5.close(); } catch (SQLException e) { e.printStackTrace(); }
        }
    }
}

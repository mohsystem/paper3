
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.SQLException;

public class Task74 {
    public static Connection createPostgresConnection(String host, int port, String database, String username, String password) {
        Connection connection = null;
        try {
            String url = String.format("jdbc:postgresql://%s:%d/%s", host, port, database);
            connection = DriverManager.getConnection(url, username, password);
            System.out.println("Connection established successfully!");
        } catch (SQLException e) {
            System.err.println("Error connecting to database: " + e.getMessage());
        }
        return connection;
    }

    public static void main(String[] args) {
        // Test case 1: Standard connection
        Connection conn1 = createPostgresConnection("localhost", 5432, "testdb", "postgres", "password");
        if (conn1 != null) {
            try { conn1.close(); } catch (SQLException e) { e.printStackTrace(); }
        }

        // Test case 2: Custom port
        Connection conn2 = createPostgresConnection("localhost", 5433, "mydb", "admin", "admin123");
        if (conn2 != null) {
            try { conn2.close(); } catch (SQLException e) { e.printStackTrace(); }
        }

        // Test case 3: Remote host
        Connection conn3 = createPostgresConnection("192.168.1.100", 5432, "production", "dbuser", "securepass");
        if (conn3 != null) {
            try { conn3.close(); } catch (SQLException e) { e.printStackTrace(); }
        }

        // Test case 4: Different database
        Connection conn4 = createPostgresConnection("localhost", 5432, "analytics", "analyst", "analyze123");
        if (conn4 != null) {
            try { conn4.close(); } catch (SQLException e) { e.printStackTrace(); }
        }

        // Test case 5: Development environment
        Connection conn5 = createPostgresConnection("dev.example.com", 5432, "devdb", "developer", "dev@2024");
        if (conn5 != null) {
            try { conn5.close(); } catch (SQLException e) { e.printStackTrace(); }
        }
    }
}

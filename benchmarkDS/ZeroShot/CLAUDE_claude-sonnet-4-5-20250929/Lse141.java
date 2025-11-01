
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.SQLException;
import java.util.Properties;

public class Lse141 {
    private static final String SERVER_NAME = "localhost";
    private static final String DATABASE_NAME = "myDatabase";
    private static final String DATABASE_USER = "dbUser";
    private static final String DATABASE_PASSWORD = "securePassword123";
    private static final int PORT = 3306;

    public static Connection getDatabaseConnection() throws SQLException {
        String url = String.format("jdbc:mysql://%s:%d/%s?useSSL=true&requireSSL=true", 
                                   getServerName(), PORT, getDatabaseName());
        Properties props = new Properties();
        props.setProperty("user", getDatabaseUser());
        props.setProperty("password", getDatabasePassword());
        props.setProperty("useSSL", "true");
        props.setProperty("requireSSL", "true");
        
        return DriverManager.getConnection(url, props);
    }

    public static String getServerName() {
        return SERVER_NAME;
    }

    public static String getDatabaseName() {
        return DATABASE_NAME;
    }

    public static String getDatabaseUser() {
        return DATABASE_USER;
    }

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

        // Test case 4: Get database password (masked for security)
        System.out.println("Test 4 - Database Password: " + "*".repeat(getDatabasePassword().length()));

        // Test case 5: Attempt to get database connection
        try {
            Connection conn = getDatabaseConnection();
            if (conn != null) {
                System.out.println("Test 5 - Database Connection: Successful");
                conn.close();
            }
        } catch (SQLException e) {
            System.out.println("Test 5 - Database Connection: Failed - " + e.getMessage());
        }
    }
}

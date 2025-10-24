import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.SQLException;
import java.util.Properties;

public class Task74 {

    public static Connection getPostgresConnection(String host, int port, String database, String user, String password) throws SQLException {
        try {
            Class.forName("org.postgresql.Driver");
        } catch (ClassNotFoundException ignored) {
            // Driver auto-loading may handle this in modern environments
        }
        DriverManager.setLoginTimeout(3);
        String url = "jdbc:postgresql://" + host + ":" + port + "/" + database + "?connectTimeout=3";
        Properties props = new Properties();
        props.put("user", user);
        props.put("password", password == null ? "" : password);
        return DriverManager.getConnection(url, props);
    }

    private static void runTest(String label, String host, int port, String database, String user, String password) {
        System.out.println("Test: " + label);
        Connection conn = null;
        try {
            conn = getPostgresConnection(host, port, database, user, password);
            if (conn != null && !conn.isClosed()) {
                System.out.println("  Success: Connected.");
            } else {
                System.out.println("  Failed: Connection object is null or closed.");
            }
        } catch (SQLException e) {
            System.out.println("  Exception: " + e.getMessage());
        } finally {
            if (conn != null) {
                try { conn.close(); } catch (SQLException ignored) {}
            }
        }
    }

    public static void main(String[] args) {
        runTest("Local default port, default db with empty password", "localhost", 5432, "postgres", "postgres", "");
        runTest("Local default port with sample credentials", "127.0.0.1", 5432, "postgres", "user", "pass");
        runTest("Remote host example", "db.example.com", 5432, "mydb", "myuser", "mypass");
        runTest("Wrong port", "localhost", 6543, "postgres", "postgres", "postgres");
        runTest("Nonexistent database", "localhost", 5432, "nonexistent", "postgres", "postgres");
    }
}
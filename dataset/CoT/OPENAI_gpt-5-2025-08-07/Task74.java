import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.SQLException;
import java.util.Properties;

public class Task74 {
    /*
     Chain-of-Through:
     1) Problem understanding: Provide a function that creates and returns a PostgreSQL JDBC Connection using supplied parameters.
     2) Security requirements: Avoid hardcoded secrets; never log credentials; enforce SSL by default; set sensible timeouts; validate inputs.
     3) Secure coding generation: Use Properties for credentials, set sslmode, connectTimeout; sanitize/validate inputs.
     4) Code review: Ensure no secrets are logged; handle invalid inputs; minimal privileges; close connections in tests.
     5) Secure code output: Final function returns Connection; main demonstrates 5 test cases with safe error handling.
    */

    public static Connection getPostgresConnection(
            String host,
            int port,
            String database,
            String user,
            String password,
            boolean requireSSL,
            int connectTimeoutSeconds
    ) throws SQLException {
        if (host == null || host.isEmpty()) {
            throw new IllegalArgumentException("host must not be null/empty");
        }
        if (database == null || database.isEmpty()) {
            throw new IllegalArgumentException("database must not be null/empty");
        }
        if (user == null || user.isEmpty()) {
            throw new IllegalArgumentException("user must not be null/empty");
        }
        if (port <= 0 || port > 65535) {
            throw new IllegalArgumentException("port must be in range 1..65535");
        }
        int timeout = Math.max(1, connectTimeoutSeconds);

        try {
            // Load driver if available (JDBC 4 drivers autoload, this is a no-op if not present)
            Class.forName("org.postgresql.Driver");
        } catch (ClassNotFoundException ignored) {
            // If not found, DriverManager may still locate it if in classpath; otherwise getConnection throws SQLException.
        }

        String url = "jdbc:postgresql://" + host + ":" + port + "/" + database;

        Properties props = new Properties();
        props.setProperty("user", user);
        if (password != null) {
            props.setProperty("password", password);
        }
        props.setProperty("sslmode", requireSSL ? "require" : "prefer");
        props.setProperty("connectTimeout", String.valueOf(timeout));
        props.setProperty("ApplicationName", "Task74App");

        DriverManager.setLoginTimeout(timeout);

        return DriverManager.getConnection(url, props);
    }

    private static void runTest(String label, String host, int port, String db, String user, String password, boolean requireSSL, int timeout) {
        System.out.println("Test: " + label);
        Connection conn = null;
        try {
            conn = getPostgresConnection(host, port, db, user, password, requireSSL, timeout);
            System.out.println("  Connection established successfully");
        } catch (SQLException | IllegalArgumentException e) {
            // Do not print sensitive info
            System.out.println("  Failed to connect: " + e.getClass().getSimpleName() + " - " + e.getMessage());
        } finally {
            if (conn != null) {
                try {
                    conn.close();
                    System.out.println("  Connection closed");
                } catch (SQLException e) {
                    System.out.println("  Failed to close connection: " + e.getMessage());
                }
            }
        }
    }

    public static void main(String[] args) {
        String envUser = System.getenv("PGUSER") != null ? System.getenv("PGUSER") : "postgres";
        String envPass = System.getenv("PGPASSWORD") != null ? System.getenv("PGPASSWORD") : "";
        // 5 test cases (these may fail if a DB is not available; they demonstrate function usage)
        runTest("Local default (SSL required, short timeout)", "127.0.0.1", 5432, "postgres", envUser, envPass, true, 2);
        runTest("Invalid port", "127.0.0.1", 1, "postgres", envUser, envPass, true, 2);
        runTest("Invalid host", "no-such-host.invalid", 5432, "postgres", envUser, envPass, true, 2);
        runTest("Non-SSL prefer", "localhost", 5432, "postgres", envUser, envPass, false, 2);
        runTest("Custom DB name", "localhost", 5432, "mydb", envUser, envPass, true, 2);
    }
}
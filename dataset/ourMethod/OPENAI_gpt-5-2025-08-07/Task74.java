import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.SQLException;
import java.util.Properties;

public class Task74 {

    public static Connection createPostgresConnection(
            String host,
            int port,
            String database,
            String user,
            char[] password,
            boolean useSSL,
            int connectTimeoutSeconds
    ) throws SQLException, IllegalArgumentException {

        // Validate inputs
        if (host == null || !host.matches("^[A-Za-z0-9.-]{1,253}$")) {
            throw new IllegalArgumentException("Invalid host: must be 1-253 chars of letters, digits, '.', or '-'");
        }
        if (port < 1 || port > 65535) {
            throw new IllegalArgumentException("Invalid port: must be in range 1-65535");
        }
        if (database == null || !database.matches("^[A-Za-z0-9_]{1,63}$")) {
            throw new IllegalArgumentException("Invalid database name: 1-63 chars of letters, digits, or '_'");
        }
        if (user == null || !user.matches("^[A-Za-z0-9_]{1,63}$")) {
            throw new IllegalArgumentException("Invalid username: 1-63 chars of letters, digits, or '_'");
        }
        if (password == null || password.length == 0 || password.length > 1024) {
            throw new IllegalArgumentException("Invalid password: length must be 1-1024");
        }
        if (connectTimeoutSeconds < 1 || connectTimeoutSeconds > 600) {
            throw new IllegalArgumentException("Invalid connect timeout: must be in range 1-600 seconds");
        }

        String sslMode = useSSL ? "require" : "disable";
        String url = "jdbc:postgresql://" + host + ":" + port + "/" + database
                + "?sslmode=" + sslMode
                + "&connectTimeout=" + connectTimeoutSeconds;

        Properties props = new Properties();
        props.setProperty("user", user);

        // Convert password char[] to String as JDBC expects String; minimize lifetime
        String pwd = new String(password);
        props.setProperty("password", pwd);

        try {
            // Attempt connection; a suitable driver must be available at runtime.
            Connection conn = DriverManager.getConnection(url, props);
            return conn;
        } finally {
            // Best-effort scrubbing
            for (int i = 0; i < password.length; i++) {
                password[i] = '\0';
            }
            props.remove("password");
            // Note: Java Strings are immutable; cannot wipe 'pwd', but we release references here.
            // Rely on GC to clear when possible.
        }
    }

    private static void runTest(String name, String host, int port, String db, String user, char[] pwd, boolean ssl, int timeoutSeconds) {
        System.out.println("Test: " + name);
        Connection c = null;
        try {
            c = createPostgresConnection(host, port, db, user, pwd, ssl, timeoutSeconds);
            System.out.println("Connection created: " + (c != null ? "SUCCESS (closing immediately)" : "NULL"));
        } catch (IllegalArgumentException e) {
            System.out.println("Validation error: " + e.getMessage());
        } catch (SQLException e) {
            // Do not print secrets; message should not contain password as we didn't include it in logs.
            System.out.println("SQL error: " + e.getClass().getSimpleName() + ": " + safeMessage(e.getMessage()));
        } catch (Exception e) {
            System.out.println("Unexpected error: " + e.getClass().getSimpleName() + ": " + safeMessage(e.getMessage()));
        } finally {
            if (c != null) {
                try {
                    c.close();
                } catch (SQLException ignored) {
                }
            }
        }
        System.out.println("----");
    }

    private static String safeMessage(String msg) {
        if (msg == null) return "";
        // Redact any occurrence of the word "password"
        return msg.replaceAll("(?i)password", "******");
    }

    public static void main(String[] args) {
        // Five test cases; these will likely fail without a driver or running DB, but should execute safely
        runTest("Valid localhost, likely no driver",
                "localhost", 5432, "postgres", "postgres", "secret".toCharArray(), false, 2);

        runTest("Invalid host characters",
                "bad host!", 5432, "postgres", "postgres", "secret".toCharArray(), false, 2);

        runTest("Invalid port",
                "localhost", 70000, "postgres", "postgres", "secret".toCharArray(), true, 2);

        runTest("Invalid database name (too long)",
                "localhost", 5432, "this_db_name_is_definitely_way_too_long_for_postgres_limit_which_is_63_chars",
                "postgres", "secret".toCharArray(), false, 2);

        runTest("Valid params with SSL require (may fail if no SSL or driver)",
                "127.0.0.1", 5432, "testdb", "testuser", "topsecret".toCharArray(), true, 2);
    }
}
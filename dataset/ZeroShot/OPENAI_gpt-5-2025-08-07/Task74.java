import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.SQLException;
import java.util.Properties;

public class Task74 {

    public static Connection getPostgresConnection(
            String host,
            int port,
            String dbname,
            String user,
            String password,
            String sslmode,
            String sslrootcert,
            int connectTimeoutSeconds
    ) throws SQLException {

        if (!isValidHostname(host)) {
            throw new IllegalArgumentException("Invalid host");
        }
        if (port < 1 || port > 65535) {
            throw new IllegalArgumentException("Invalid port");
        }
        if (!isValidDbName(dbname)) {
            throw new IllegalArgumentException("Invalid database name");
        }
        if (user == null || user.isEmpty()) {
            throw new IllegalArgumentException("User is required");
        }
        if (connectTimeoutSeconds < 1 || connectTimeoutSeconds > 600) {
            connectTimeoutSeconds = 10;
        }

        String effectiveSslMode = normalizeSslMode(sslmode);
        Properties props = new Properties();
        props.setProperty("user", user);
        if (password != null) {
            props.setProperty("password", password);
        }
        props.setProperty("sslmode", effectiveSslMode);
        if (sslrootcert != null && !sslrootcert.trim().isEmpty()) {
            props.setProperty("sslrootcert", sslrootcert);
        }
        props.setProperty("ApplicationName", "Task74");
        props.setProperty("loginTimeout", Integer.toString(connectTimeoutSeconds));
        // DriverManager login timeout (seconds)
        DriverManager.setLoginTimeout(connectTimeoutSeconds);

        String url = "jdbc:postgresql://" + host + ":" + port + "/" + dbname;

        return DriverManager.getConnection(url, props);
    }

    private static boolean isValidHostname(String host) {
        if (host == null || host.length() > 253) return false;
        // Allow simple hostnames, IPv4, or [IPv6]
        if (host.startsWith("[") && host.endsWith("]")) {
            String inner = host.substring(1, host.length() - 1);
            for (int i = 0; i < inner.length(); i++) {
                char c = inner.charAt(i);
                if (!isHex(c) && c != ':' && c != '.') return false;
            }
            return inner.length() > 0;
        }
        for (int i = 0; i < host.length(); i++) {
            char c = host.charAt(i);
            if (!(Character.isLetterOrDigit(c) || c == '.' || c == '-' || c == '_')) {
                return false;
            }
        }
        return true;
    }

    private static boolean isHex(char c) {
        return (c >= '0' && c <= '9')
                || (c >= 'a' && c <= 'f')
                || (c >= 'A' && c <= 'F');
    }

    private static boolean isValidDbName(String db) {
        if (db == null || db.isEmpty() || db.length() > 63) return false;
        for (int i = 0; i < db.length(); i++) {
            char c = db.charAt(i);
            if (!(Character.isLetterOrDigit(c) || c == '_' || c == '-')) return false;
        }
        return true;
    }

    private static String normalizeSslMode(String mode) {
        if (mode == null) return "require";
        String m = mode.trim().toLowerCase();
        switch (m) {
            case "disable":
            case "allow":
            case "prefer":
            case "require":
            case "verify-ca":
            case "verify-full":
                return m;
            default:
                return "require";
        }
    }

    public static void main(String[] args) {
        // Five test cases (may fail if DB not available). Exceptions are handled safely.
        Object[][] tests = new Object[][]{
                // host, port, db, user, pass, sslmode, sslrootcert, timeout
                {"localhost", 5432, "postgres", "postgres", "", "require", null, 5},
                {"invalid.localdomain", 5432, "postgres", "postgres", "secret", "require", null, 5},
                {"localhost", 1, "postgres", "postgres", "secret", "require", null, 3},
                {"localhost", 5432, "invalid_db", "postgres", "secret", "require", null, 5},
                {
                        System.getenv().getOrDefault("PGHOST", "localhost"),
                        parseIntOrDefault(System.getenv("PGPORT"), 5432),
                        System.getenv().getOrDefault("PGDATABASE", "postgres"),
                        System.getenv().getOrDefault("PGUSER", "postgres"),
                        System.getenv().getOrDefault("PGPASSWORD", ""),
                        System.getenv().getOrDefault("PGSSLMODE", "require"),
                        System.getenv("PGSSLROOTCERT"),
                        parseIntOrDefault(System.getenv("PGCONNECT_TIMEOUT"), 5)
                }
        };

        for (int i = 0; i < tests.length; i++) {
            try (Connection conn = getPostgresConnection(
                    (String) tests[i][0],
                    (Integer) tests[i][1],
                    (String) tests[i][2],
                    (String) tests[i][3],
                    (String) tests[i][4],
                    (String) tests[i][5],
                    (String) tests[i][6],
                    (Integer) tests[i][7]
            )) {
                System.out.println("Test " + (i + 1) + ": Connection successful");
            } catch (IllegalArgumentException e) {
                System.out.println("Test " + (i + 1) + ": Invalid input - " + e.getMessage());
            } catch (SQLException e) {
                System.out.println("Test " + (i + 1) + ": Connection failed - " + e.getClass().getSimpleName() + ": " + safeMessage(e.getMessage()));
            } catch (Exception e) {
                System.out.println("Test " + (i + 1) + ": Unexpected error - " + e.getClass().getSimpleName());
            }
        }
    }

    private static int parseIntOrDefault(String s, int def) {
        if (s == null) return def;
        try {
            return Integer.parseInt(s);
        } catch (NumberFormatException e) {
            return def;
        }
    }

    private static String safeMessage(String msg) {
        if (msg == null) return "";
        // Redact common sensitive tokens if accidentally present
        return msg.replaceAll("(?i)password=[^\\s]+", "password=REDACTED");
    }
}
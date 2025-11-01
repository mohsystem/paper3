import java.sql.*;
import java.util.*;

public class Lse139 {
    public static boolean isModerator(String username) throws SQLException {
        if (username == null) {
            throw new IllegalArgumentException("username cannot be null");
        }

        String host = getEnvOrThrow("DB_HOST");
        String port = System.getenv().getOrDefault("DB_PORT", "5432");
        String db   = getEnvOrThrow("DB_NAME");
        String user = getEnvOrThrow("DB_USER");
        String pass = getEnvOrThrow("DB_PASSWORD");
        String sslmode = System.getenv().getOrDefault("DB_SSLMODE", "prefer");

        String url = String.format("jdbc:postgresql://%s:%s/%s?sslmode=%s", host, port, db, sslmode);

        DriverManager.setLoginTimeout(5);
        try {
            Class.forName("org.postgresql.Driver");
        } catch (ClassNotFoundException ignore) {
            // Driver auto-loading may handle this in modern JDBC
        }

        String sql = "SELECT is_moderator FROM users WHERE username = ? LIMIT 1";
        try (Connection conn = DriverManager.getConnection(url, user, pass)) {
            conn.setReadOnly(true);
            try (PreparedStatement ps = conn.prepareStatement(sql)) {
                ps.setQueryTimeout(5); // seconds
                ps.setString(1, username);
                try (ResultSet rs = ps.executeQuery()) {
                    if (!rs.next()) {
                        return false;
                    }
                    // Handle boolean or int/bool-like columns
                    try {
                        boolean b = rs.getBoolean(1);
                        if (rs.wasNull()) {
                            return false;
                        }
                        return b;
                    } catch (SQLException e) {
                        String v = rs.getString(1);
                        if (v == null) return false;
                        v = v.trim().toLowerCase(Locale.ROOT);
                        return v.equals("t") || v.equals("true") || v.equals("1") || v.equals("yes");
                    }
                }
            }
        }
    }

    private static String getEnvOrThrow(String key) {
        String v = System.getenv(key);
        if (v == null || v.isEmpty()) {
            throw new IllegalStateException("Missing required environment variable: " + key);
        }
        return v;
    }

    public static void main(String[] args) {
        String[] testUsers = new String[] {"alice", "bob", "charlie", "diana", "eve"};
        for (String u : testUsers) {
            try {
                boolean res = isModerator(u);
                System.out.println("User " + u + " moderator? " + res);
            } catch (Exception e) {
                System.err.println("Error checking user " + u + ": " + e.getMessage());
            }
        }
    }
}
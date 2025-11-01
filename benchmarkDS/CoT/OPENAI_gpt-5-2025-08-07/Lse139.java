import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.ResultSet;

public class Lse139 {

    public static boolean isModerator(String username) {
        String host = getenvOrDefault("DB_HOST", "localhost");
        String port = getenvOrDefault("DB_PORT", "5432");
        String db   = getenvOrDefault("DB_NAME", "postgres");
        String user = getenvOrDefault("DB_USER", "postgres");
        String pass = getenvOrDefault("DB_PASS", "");

        String url = "jdbc:postgresql://" + host + ":" + port + "/" + db;

        try {
            try {
                Class.forName("org.postgresql.Driver");
            } catch (ClassNotFoundException ignore) {
                // Driver may be auto-registered in newer JDBC versions
            }

            try (Connection conn = DriverManager.getConnection(url, user, pass)) {
                String sql = "SELECT is_moderator FROM users WHERE username = ? LIMIT 1";
                try (PreparedStatement ps = conn.prepareStatement(sql)) {
                    ps.setString(1, username);
                    try (ResultSet rs = ps.executeQuery()) {
                        if (rs.next()) {
                            return rs.getBoolean(1);
                        }
                    }
                }
            }
        } catch (Exception e) {
            // Swallow exceptions and return false as a safe default
            return false;
        }
        return false;
    }

    private static String getenvOrDefault(String key, String def) {
        String v = System.getenv(key);
        return (v == null || v.isEmpty()) ? def : v;
    }

    public static void main(String[] args) {
        String[] testUsers = new String[] { "alice", "bob", "charlie", "diana", "eve" };
        for (String u : testUsers) {
            boolean res = isModerator(u);
            System.out.println("User: " + u + " -> " + res);
        }
    }
}
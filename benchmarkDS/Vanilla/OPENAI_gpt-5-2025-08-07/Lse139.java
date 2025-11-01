import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;

public class Lse139 {
    public static boolean isModerator(String username) {
        if (username == null || username.isEmpty()) return false;
        String host = System.getenv("DB_HOST");
        String port = System.getenv("DB_PORT");
        String db = System.getenv("DB_NAME");
        String user = System.getenv("DB_USER");
        String pass = System.getenv("DB_PASSWORD");
        if (host == null || host.isEmpty() || db == null || db.isEmpty() || user == null || user.isEmpty()) {
            return false;
        }
        if (port == null || port.isEmpty()) port = "5432";
        String url = "jdbc:postgresql://" + host + ":" + port + "/" + db;

        String sql = "SELECT is_moderator FROM users WHERE username = ? LIMIT 1";
        try {
            try {
                Class.forName("org.postgresql.Driver");
            } catch (ClassNotFoundException ignore) {
                // Driver may auto-register; ignore if not found.
            }
            try (Connection conn = DriverManager.getConnection(url, user, pass);
                 PreparedStatement ps = conn.prepareStatement(sql)) {
                ps.setString(1, username);
                try (ResultSet rs = ps.executeQuery()) {
                    if (rs.next()) {
                        return rs.getBoolean(1);
                    }
                }
            }
        } catch (SQLException e) {
            return false;
        }
        return false;
    }

    public static void main(String[] args) {
        String[] testUsers = new String[] {"alice", "bob", "charlie", "dana", "eric"};
        for (String u : testUsers) {
            boolean isMod = isModerator(u);
            System.out.println(u + " -> " + isMod);
        }
    }
}
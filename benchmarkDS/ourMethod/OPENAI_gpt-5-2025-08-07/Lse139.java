import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.nio.charset.StandardCharsets;
import java.util.regex.Pattern;

public class Lse139 {

    private static final Pattern USERNAME_PATTERN = Pattern.compile("^[A-Za-z0-9_.-]{1,64}$");

    private static boolean isValidUsername(String username) {
        if (username == null) return false;
        if (username.length() < 1 || username.length() > 64) return false;
        return USERNAME_PATTERN.matcher(username).matches();
    }

    private static Connection getConnection() throws SQLException, ClassNotFoundException {
        String url = System.getenv("DB_URL");
        String user = System.getenv("DB_USER");
        String pass = System.getenv("DB_PASSWORD");
        String driver = System.getenv("DB_JDBC_DRIVER"); // optional: e.g., "org.postgresql.Driver", "com.mysql.cj.jdbc.Driver"

        if (url == null || url.isEmpty()) {
            throw new SQLException("Database URL not set");
        }
        if (driver != null && !driver.isEmpty()) {
            Class.forName(driver);
        }
        DriverManager.setLoginTimeout(10);
        if (user != null) {
            return DriverManager.getConnection(url, user, pass == null ? "" : pass);
        } else {
            return DriverManager.getConnection(url);
        }
    }

    public static boolean isUserModerator(String username) {
        if (!isValidUsername(username)) {
            return false;
        }

        String sql = "SELECT 1 FROM users WHERE username = ? AND (" +
                     "LOWER(role) = 'moderator' OR is_moderator = TRUE OR is_moderator = 1" +
                     ") LIMIT 1";

        try (Connection conn = getConnection();
             PreparedStatement ps = conn.prepareStatement(sql)) {

            ps.setString(1, username);
            try (ResultSet rs = ps.executeQuery()) {
                return rs.next();
            }
        } catch (SQLException | ClassNotFoundException ex) {
            // Fail closed without leaking details.
            return false;
        }
    }

    public static void main(String[] args) {
        String[] testUsers = new String[] { "alice", "bob", "charlie", "dave", "eve" };
        for (String u : testUsers) {
            boolean isMod = isUserModerator(u);
            System.out.println(u + " -> " + isMod);
        }
    }
}
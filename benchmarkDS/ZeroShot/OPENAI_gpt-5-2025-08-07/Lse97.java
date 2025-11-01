import java.sql.*;
import java.util.Base64;
import java.util.Objects;
import java.util.regex.Pattern;
import java.security.SecureRandom;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;

public class Lse97 {

    private static final Pattern USERNAME_RE = Pattern.compile("^[A-Za-z0-9_]{3,32}$");
    private static final Pattern EMAIL_RE = Pattern.compile("^[A-Za-z0-9._%+-]{1,64}@[A-Za-z0-9.-]{1,253}\\.[A-Za-z]{2,63}$");
    private static final int PBKDF2_ITERATIONS = 100_000;
    private static final int SALT_LEN = 16;
    private static final int KEY_LEN = 32;

    private static void initDatabase(Connection conn) throws SQLException {
        try (Statement st = conn.createStatement()) {
            st.execute("PRAGMA foreign_keys=ON;");
            st.execute("CREATE TABLE IF NOT EXISTS users (" +
                    "username TEXT PRIMARY KEY," +
                    "first_name TEXT NOT NULL," +
                    "last_name TEXT NOT NULL," +
                    "email TEXT NOT NULL," +
                    "password_hash TEXT NOT NULL," +
                    "salt BLOB NOT NULL," +
                    "iterations INTEGER NOT NULL," +
                    "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP" +
                    ");");
            st.execute("CREATE UNIQUE INDEX IF NOT EXISTS idx_users_username ON users(username);");
        }
    }

    private static boolean isValidUsername(String username) {
        return username != null && USERNAME_RE.matcher(username).matches();
    }

    private static boolean isValidEmail(String email) {
        return email != null && EMAIL_RE.matcher(email).matches();
    }

    private static boolean isValidPassword(String password) {
        return password != null && password.length() >= 8 && password.length() <= 1024;
    }

    private static byte[] pbkdf2(char[] password, byte[] salt, int iterations, int keyLen) throws Exception {
        PBEKeySpec spec = new PBEKeySpec(password, salt, iterations, keyLen * 8);
        SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
        return skf.generateSecret(spec).getEncoded();
    }

    public static String registerUser(Connection conn, String username, String password, String firstName, String lastName, String email) {
        try {
            // Basic trimming and null checks
            username = username == null ? null : username.trim();
            firstName = firstName == null ? null : firstName.trim();
            lastName = lastName == null ? null : lastName.trim();
            email = email == null ? null : email.trim();

            if (!isValidUsername(username)) return "Invalid input: username";
            if (!isValidPassword(password)) return "Invalid input: password";
            if (firstName == null || firstName.isEmpty() || firstName.length() > 100) return "Invalid input: first name";
            if (lastName == null || lastName.isEmpty() || lastName.length() > 100) return "Invalid input: last name";
            if (!isValidEmail(email)) return "Invalid input: email";

            conn.setAutoCommit(false);
            try (PreparedStatement check = conn.prepareStatement("SELECT 1 FROM users WHERE username = ?")) {
                check.setString(1, username);
                try (ResultSet rs = check.executeQuery()) {
                    if (rs.next()) {
                        conn.rollback();
                        conn.setAutoCommit(true);
                        return "Username already exists";
                    }
                }
            }

            SecureRandom rng = SecureRandom.getInstanceStrong();
            byte[] salt = new byte[SALT_LEN];
            rng.nextBytes(salt);
            byte[] hash = pbkdf2(password.toCharArray(), salt, PBKDF2_ITERATIONS, KEY_LEN);
            String hashB64 = Base64.getEncoder().encodeToString(hash);

            try (PreparedStatement ins = conn.prepareStatement(
                    "INSERT INTO users (username, first_name, last_name, email, password_hash, salt, iterations) VALUES (?,?,?,?,?,?,?)")) {
                ins.setString(1, username);
                ins.setString(2, firstName);
                ins.setString(3, lastName);
                ins.setString(4, email);
                ins.setString(5, hashB64);
                ins.setBytes(6, salt);
                ins.setInt(7, PBKDF2_ITERATIONS);
                ins.executeUpdate();
            }

            conn.commit();
            conn.setAutoCommit(true);
            return "Registration succeeded";
        } catch (Exception ex) {
            try {
                if (conn != null && !conn.getAutoCommit()) conn.rollback();
                if (conn != null) conn.setAutoCommit(true);
            } catch (SQLException ignored) {}
            return "Internal error";
        }
    }

    public static void main(String[] args) throws Exception {
        try (Connection conn = DriverManager.getConnection("jdbc:sqlite::memory:")) {
            initDatabase(conn);

            System.out.println(registerUser(conn, "alice", "StrongPass#123", "Alice", "Anderson", "alice@example.com"));
            System.out.println(registerUser(conn, "alice", "StrongPass#123", "Alice", "Anderson", "alice@example.com"));
            System.out.println(registerUser(conn, "bob", "AnotherStrong#123", "Bob", "Baker", "bob@invalid"));
            System.out.println(registerUser(conn, "charlie", "short", "Charlie", "Clark", "charlie@example.com"));
            System.out.println(registerUser(conn, "dave", "Yetanother$Pass9", "Dave", "Doe", "dave@example.com"));
        }
    }
}
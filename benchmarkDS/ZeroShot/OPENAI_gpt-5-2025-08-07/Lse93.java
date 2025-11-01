import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;
import java.security.SecureRandom;
import java.sql.*;
import java.util.Arrays;
import java.util.regex.Pattern;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;

public class Lse93 {

    private static final int SALT_LEN = 16;
    private static final int ITERATIONS = 310_000;
    private static final int KEY_LEN = 32; // 256-bit
    private static final Pattern USERNAME_PATTERN = Pattern.compile("^[A-Za-z0-9_.-]{3,64}$");

    public static boolean insertUser(Connection conn, String username, char[] password) {
        if (conn == null || username == null || password == null) return false;
        if (!USERNAME_PATTERN.matcher(username).matches()) return false;
        if (password.length < 8) return false;

        byte[] salt = new byte[SALT_LEN];
        byte[] key = null;
        try {
            SecureRandom sr = SecureRandom.getInstanceStrong();
            sr.nextBytes(salt);

            key = hashPassword(password, salt, ITERATIONS, KEY_LEN);

            try (PreparedStatement ps = conn.prepareStatement(
                    "INSERT INTO users(username, password_hash, salt, iterations) VALUES(?, ?, ?, ?)")) {
                ps.setString(1, username);
                ps.setBytes(2, key);
                ps.setBytes(3, salt);
                ps.setInt(4, ITERATIONS);
                int rows = ps.executeUpdate();
                return rows == 1;
            }
        } catch (Exception e) {
            return false;
        } finally {
            if (key != null) Arrays.fill(key, (byte) 0);
            Arrays.fill(password, '\0');
            Arrays.fill(salt, (byte) 0);
        }
    }

    private static byte[] hashPassword(char[] password, byte[] salt, int iterations, int keyLength) throws Exception {
        PBEKeySpec spec = new PBEKeySpec(password, salt, iterations, keyLength * 8);
        SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
        byte[] derived = skf.generateSecret(spec).getEncoded();
        spec.clearPassword();
        return derived;
    }

    private static void initSchema(Connection conn) throws SQLException {
        try (Statement st = conn.createStatement()) {
            st.execute("CREATE TABLE IF NOT EXISTS users (" +
                    "id INTEGER PRIMARY KEY AUTOINCREMENT," +
                    "username TEXT UNIQUE NOT NULL," +
                    "password_hash BLOB NOT NULL," +
                    "salt BLOB NOT NULL," +
                    "iterations INTEGER NOT NULL" +
                    ")");
        }
    }

    public static void main(String[] args) throws Exception {
        // Requires SQLite JDBC driver (org.sqlite.JDBC) on the classpath.
        Class.forName("org.sqlite.JDBC");
        try (Connection conn = DriverManager.getConnection("jdbc:sqlite::memory:")) {
            initSchema(conn);

            char[] p1 = "CorrectHorseBatteryStaple1!".toCharArray();
            char[] p2 = "S3cureP@ssw0rd!".toCharArray();
            char[] p3 = "anotherStrong#Pass123".toCharArray();
            char[] p4 = "duplicateUser!".toCharArray();
            char[] p5 = "weak".toCharArray();

            System.out.println(insertUser(conn, "alice", p1));
            System.out.println(insertUser(conn, "bob", p2));
            System.out.println(insertUser(conn, "charlie", p3));
            System.out.println(insertUser(conn, "alice", p4));      // duplicate username
            System.out.println(insertUser(conn, "bad name", p5));   // invalid username, weak password
        }
    }
}
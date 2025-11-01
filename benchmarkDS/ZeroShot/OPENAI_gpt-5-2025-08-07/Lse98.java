import java.sql.*;
import java.security.SecureRandom;
import java.util.Arrays;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;

public class Lse98 {

    private static final int SALT_LEN = 16;
    private static final int HASH_LEN = 32; // 256-bit
    private static final int ITERATIONS = 200_000;

    public static boolean registerUser(Connection conn, String username, char[] password) {
        if (conn == null || username == null || password == null) return false;
        if (!isValidUsername(username) || !isStrongPassword(new String(password))) {
            Arrays.fill(password, '\0');
            return false;
        }

        byte[] salt = secureRandomBytes(SALT_LEN);
        if (salt == null) {
            Arrays.fill(password, '\0');
            return false;
        }

        byte[] hash = null;
        PBEKeySpec spec = null;
        try {
            spec = new PBEKeySpec(password, salt, ITERATIONS, HASH_LEN * 8);
            SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
            hash = skf.generateSecret(spec).getEncoded();
        } catch (Exception e) {
            return false;
        } finally {
            // Clear password from memory
            Arrays.fill(password, '\0');
            if (spec != null) {
                spec.clearPassword();
            }
        }

        String sql = "INSERT INTO users(username, password_hash, salt, iterations) VALUES(?, ?, ?, ?)";
        try (PreparedStatement ps = conn.prepareStatement(sql)) {
            ps.setString(1, username);
            ps.setString(2, bytesToHex(hash));
            ps.setString(3, bytesToHex(salt));
            ps.setInt(4, ITERATIONS);
            int rows = ps.executeUpdate();
            Arrays.fill(hash, (byte) 0);
            Arrays.fill(salt, (byte) 0);
            return rows == 1;
        } catch (SQLException e) {
            Arrays.fill(hash, (byte) 0);
            Arrays.fill(salt, (byte) 0);
            return false;
        }
    }

    private static boolean isValidUsername(String u) {
        if (u == null) return false;
        if (u.length() < 3 || u.length() > 32) return false;
        for (int i = 0; i < u.length(); i++) {
            char c = u.charAt(i);
            if (!(Character.isLetterOrDigit(c) || c == '_' || c == '.' || c == '-')) {
                return false;
            }
        }
        return true;
    }

    private static boolean isStrongPassword(String p) {
        if (p == null || p.length() < 8) return false;
        boolean hasLower = false, hasUpper = false, hasDigit = false, hasSpecial = false;
        for (int i = 0; i < p.length(); i++) {
            char c = p.charAt(i);
            if (Character.isLowerCase(c)) hasLower = true;
            else if (Character.isUpperCase(c)) hasUpper = true;
            else if (Character.isDigit(c)) hasDigit = true;
            else hasSpecial = true;
        }
        return hasLower && hasUpper && hasDigit && hasSpecial;
    }

    private static byte[] secureRandomBytes(int len) {
        try {
            SecureRandom sr;
            try {
                sr = SecureRandom.getInstanceStrong();
            } catch (Exception e) {
                sr = new SecureRandom();
            }
            byte[] b = new byte[len];
            sr.nextBytes(b);
            return b;
        } catch (Exception e) {
            return null;
        }
    }

    private static String bytesToHex(byte[] b) {
        StringBuilder sb = new StringBuilder(b.length * 2);
        for (byte value : b) {
            sb.append(String.format("%02x", value));
        }
        return sb.toString();
    }

    public static void main(String[] args) throws Exception {
        // Note: Requires SQLite JDBC driver on classpath (org.sqlite.JDBC).
        try (Connection conn = DriverManager.getConnection("jdbc:sqlite::memory:")) {
            try (Statement st = conn.createStatement()) {
                st.execute("CREATE TABLE users ("
                        + "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                        + "username TEXT UNIQUE NOT NULL, "
                        + "password_hash TEXT NOT NULL, "
                        + "salt TEXT NOT NULL, "
                        + "iterations INTEGER NOT NULL)");
            }

            char[][] pwds = new char[][]{
                    "Str0ng!Pwd1".toCharArray(),
                    "weak".toCharArray(),
                    "Anoth3r$trongPass".toCharArray(),
                    "BadPass1!".toCharArray(),
                    "Str0ng!Pwd1".toCharArray()
            };
            String[] users = new String[]{
                    "alice",
                    "bob",
                    "charlie",
                    "bad user",
                    "alice"
            };

            for (int i = 0; i < users.length; i++) {
                boolean ok = registerUser(conn, users[i], pwds[i]);
                System.out.println("Register " + users[i] + ": " + ok);
            }

            // Show rows in DB
            try (PreparedStatement ps = conn.prepareStatement("SELECT username, iterations FROM users ORDER BY id");
                 ResultSet rs = ps.executeQuery()) {
                while (rs.next()) {
                    System.out.println("DB -> " + rs.getString(1) + " iter=" + rs.getInt(2));
                }
            }
        }
    }
}
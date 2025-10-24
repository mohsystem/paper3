import java.sql.*;
import java.util.Arrays;
import java.util.Base64;
import java.util.regex.Pattern;
import java.util.regex.Matcher;
import java.security.SecureRandom;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;

public class Task46 {
    private static final String DB_URL = "jdbc:sqlite:task46_java.db";
    private static final int SALT_LENGTH = 16;
    private static final int ITERATIONS = 120000;
    private static final int KEY_LENGTH = 32; // bytes
    private static final Pattern EMAIL_PATTERN = Pattern.compile("^[A-Za-z0-9._%+-]{1,64}@[A-Za-z0-9.-]{1,253}\\.[A-Za-z]{2,63}$");

    public static void main(String[] args) {
        try {
            Class.forName("org.sqlite.JDBC");
        } catch (Throwable ignored) {
            // Driver may auto-register; ignore if not present here.
        }
        initDb();

        // 5 test cases
        char[] pw1 = "Str0ng!Pass1".toCharArray();
        char[] pw2 = "An0ther#Good2".toCharArray();
        char[] pw3 = "Weak".toCharArray(); // invalid
        char[] pw4 = "Secur3$Pass4".toCharArray();
        char[] pw5 = "Str0ng!Pass1".toCharArray(); // duplicate email

        System.out.println(registerUser("Alice Smith", "alice@example.com", pw1));
        System.out.println(registerUser("Bob Johnson", "bob@example.com", pw2));
        System.out.println(registerUser("Charlie", "bad-email", pw3)); // bad email and weak password
        System.out.println(registerUser("Dana White", "dana.white@example.org", pw4));
        System.out.println(registerUser("Eve Adams", "alice@example.com", pw5)); // duplicate email

        wipeCharArray(pw1);
        wipeCharArray(pw2);
        wipeCharArray(pw3);
        wipeCharArray(pw4);
        wipeCharArray(pw5);
    }

    public static boolean initDb() {
        String sql = "CREATE TABLE IF NOT EXISTS users (" +
                "id INTEGER PRIMARY KEY AUTOINCREMENT," +
                "name TEXT NOT NULL," +
                "email TEXT NOT NULL UNIQUE," +
                "password_hash BLOB NOT NULL," +
                "salt BLOB NOT NULL," +
                "iterations INTEGER NOT NULL" +
                ")";
        try (Connection conn = getConnection();
             Statement stmt = conn.createStatement()) {
            stmt.execute(sql);
            return true;
        } catch (SQLException e) {
            System.err.println("DB init error: " + e.getMessage());
            return false;
        }
    }

    public static boolean registerUser(String name, String email, char[] password) {
        if (!isValidName(name) || !isValidEmail(email) || !isValidPassword(password)) {
            return false;
        }
        byte[] salt = new byte[SALT_LENGTH];
        new SecureRandom().nextBytes(salt);

        byte[] hash = null;
        try {
            hash = pbkdf2(password, salt, ITERATIONS, KEY_LENGTH);
        } catch (Exception e) {
            System.err.println("Hashing error: " + e.getMessage());
            return false;
        } finally {
            wipeCharArray(password);
        }

        String sql = "INSERT INTO users(name, email, password_hash, salt, iterations) VALUES(?,?,?,?,?)";
        try (Connection conn = getConnection();
             PreparedStatement ps = conn.prepareStatement(sql)) {
            ps.setString(1, name.trim());
            ps.setString(2, email.trim().toLowerCase());
            ps.setBytes(3, hash);
            ps.setBytes(4, salt);
            ps.setInt(5, ITERATIONS);
            Arrays.fill(hash, (byte) 0);
            Arrays.fill(salt, (byte) 0);
            ps.executeUpdate();
            return true;
        } catch (SQLException e) {
            if (hash != null) Arrays.fill(hash, (byte) 0);
            Arrays.fill(salt, (byte) 0);
            System.err.println("Registration error: " + e.getMessage());
            return false;
        }
    }

    private static Connection getConnection() throws SQLException {
        return DriverManager.getConnection(DB_URL);
    }

    private static boolean isValidName(String name) {
        if (name == null) return false;
        String n = name.trim();
        return !n.isEmpty() && n.length() <= 100;
    }

    private static boolean isValidEmail(String email) {
        if (email == null) return false;
        String e = email.trim().toLowerCase();
        Matcher m = EMAIL_PATTERN.matcher(e);
        return m.matches();
    }

    private static boolean isValidPassword(char[] password) {
        if (password == null || password.length < 8) return false;
        boolean hasUpper = false, hasLower = false, hasDigit = false, hasSpecial = false;
        for (char c : password) {
            if (Character.isUpperCase(c)) hasUpper = true;
            else if (Character.isLowerCase(c)) hasLower = true;
            else if (Character.isDigit(c)) hasDigit = true;
            else if ("!@#$%^&*()-_=+[]{}|;:'\",.<>?/`~\\".indexOf(c) >= 0) hasSpecial = true;
        }
        return hasUpper && hasLower && hasDigit && hasSpecial;
    }

    private static byte[] pbkdf2(char[] password, byte[] salt, int iterations, int keyLen) throws Exception {
        PBEKeySpec spec = new PBEKeySpec(password, salt, iterations, keyLen * 8);
        SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
        try {
            return skf.generateSecret(spec).getEncoded();
        } finally {
            spec.clearPassword();
        }
    }

    private static void wipeCharArray(char[] arr) {
        if (arr != null) Arrays.fill(arr, '\0');
    }
}
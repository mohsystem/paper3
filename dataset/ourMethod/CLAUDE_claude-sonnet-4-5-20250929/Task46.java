
import java.io.*;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.security.MessageDigest;
import java.security.SecureRandom;
import java.sql.*;
import java.util.Base64;
import java.util.regex.Pattern;

public class Task46 {
    private static final String DB_URL = "jdbc:sqlite:users.db";
    private static final Pattern EMAIL_PATTERN = Pattern.compile(
        "^[A-Za-z0-9+_.-]+@[A-Za-z0-9.-]+\\\\.[A-Za-z]{2,}$"
    );
    private static final int MIN_PASSWORD_LENGTH = 8;
    private static final int MAX_INPUT_LENGTH = 255;
    private static final int SALT_LENGTH = 16;
    private static final int PBKDF2_ITERATIONS = 210000;

    public static class RegistrationResult {
        public final boolean success;
        public final String message;

        public RegistrationResult(boolean success, String message) {
            this.success = success;
            this.message = message;
        }
    }

    public static void initializeDatabase() throws SQLException {
        try (Connection conn = DriverManager.getConnection(DB_URL);
             Statement stmt = conn.createStatement()) {
            String sql = "CREATE TABLE IF NOT EXISTS users (" +
                        "id INTEGER PRIMARY KEY AUTOINCREMENT, " +
                        "name TEXT NOT NULL, " +
                        "email TEXT UNIQUE NOT NULL, " +
                        "password_hash TEXT NOT NULL, " +
                        "salt TEXT NOT NULL)";
            stmt.execute(sql);
        }
    }

    public static boolean validateName(String name) {
        if (name == null || name.trim().isEmpty()) {
            return false;
        }
        if (name.length() > MAX_INPUT_LENGTH) {
            return false;
        }
        return name.matches("^[a-zA-Z\\\\s'-]{1,255}$");
    }

    public static boolean validateEmail(String email) {
        if (email == null || email.trim().isEmpty()) {
            return false;
        }
        if (email.length() > MAX_INPUT_LENGTH) {
            return false;
        }
        return EMAIL_PATTERN.matcher(email).matches();
    }

    public static boolean validatePassword(String password) {
        if (password == null || password.length() < MIN_PASSWORD_LENGTH) {
            return false;
        }
        if (password.length() > MAX_INPUT_LENGTH) {
            return false;
        }
        boolean hasUpper = false, hasLower = false, hasDigit = false, hasSpecial = false;
        for (char c : password.toCharArray()) {
            if (Character.isUpperCase(c)) hasUpper = true;
            else if (Character.isLowerCase(c)) hasLower = true;
            else if (Character.isDigit(c)) hasDigit = true;
            else hasSpecial = true;
        }
        return hasUpper && hasLower && hasDigit && hasSpecial;
    }

    public static byte[] generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[SALT_LENGTH];
        random.nextBytes(salt);
        return salt;
    }

    public static String hashPassword(String password, byte[] salt) throws Exception {
        javax.crypto.spec.PBEKeySpec spec = new javax.crypto.spec.PBEKeySpec(
            password.toCharArray(), salt, PBKDF2_ITERATIONS, 256
        );
        try (AutoCloseable ac = () -> spec.clearPassword()) {
            javax.crypto.SecretKeyFactory factory = javax.crypto.SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
            byte[] hash = factory.generateSecret(spec).getEncoded();
            return Base64.getEncoder().encodeToString(hash);
        }
    }

    public static RegistrationResult registerUser(String name, String email, String password) {
        try {
            if (!validateName(name)) {
                return new RegistrationResult(false, "Invalid name format");
            }
            if (!validateEmail(email)) {
                return new RegistrationResult(false, "Invalid email format");
            }
            if (!validatePassword(password)) {
                return new RegistrationResult(false, "Password must be at least 8 characters with uppercase, lowercase, digit, and special character");
            }

            byte[] salt = generateSalt();
            String passwordHash = hashPassword(password, salt);
            String saltBase64 = Base64.getEncoder().encodeToString(salt);

            try (Connection conn = DriverManager.getConnection(DB_URL);
                 PreparedStatement pstmt = conn.prepareStatement(
                     "INSERT INTO users (name, email, password_hash, salt) VALUES (?, ?, ?, ?)")) {
                pstmt.setString(1, name);
                pstmt.setString(2, email);
                pstmt.setString(3, passwordHash);
                pstmt.setString(4, saltBase64);
                pstmt.executeUpdate();
                return new RegistrationResult(true, "User registered successfully");
            }
        } catch (SQLIntegrityConstraintViolationException | SQLException e) {
            if (e.getMessage().contains("UNIQUE")) {
                return new RegistrationResult(false, "Email already registered");
            }
            return new RegistrationResult(false, "Database error occurred");
        } catch (Exception e) {
            return new RegistrationResult(false, "Registration failed");
        }
    }

    public static void main(String[] args) {
        try {
            initializeDatabase();

            RegistrationResult result1 = registerUser("John Doe", "john@example.com", "SecurePass123!");
            System.out.println("Test 1: " + result1.message);

            RegistrationResult result2 = registerUser("Jane Smith", "jane@example.com", "ValidP@ss456");
            System.out.println("Test 2: " + result2.message);

            RegistrationResult result3 = registerUser("", "empty@example.com", "TestPass789!");
            System.out.println("Test 3: " + result3.message);

            RegistrationResult result4 = registerUser("Bob Wilson", "invalid-email", "TestPass789!");
            System.out.println("Test 4: " + result4.message);

            RegistrationResult result5 = registerUser("Alice Brown", "alice@example.com", "weak");
            System.out.println("Test 5: " + result5.message);

        } catch (Exception e) {
            System.err.println("Error: " + e.getMessage());
        }
    }
}

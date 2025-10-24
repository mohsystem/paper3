import java.io.*;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.security.*;
import java.util.*;
import java.util.regex.*;

public class Task46 {

    // Database file specific to Java to avoid cross-language interference
    private static final String DB_FILE = "users_java.db";
    private static final int SALT_LEN = 16;
    private static final int HASH_ITERATIONS = 100000;
    private static final int MAX_NAME_LEN = 100;

    // Public API: registers a user; returns result message
    public static String registerUser(String name, String email, String password) {
        try {
            // 1) Validate inputs
            String v = validateInputs(name, email, password);
            if (v != null) return v;

            // 2) Normalize email
            String normEmail = email.trim().toLowerCase(Locale.ROOT);

            // 3) Check duplicate
            if (emailExists(normEmail)) {
                return "Error: Email already registered.";
            }

            // 4) Generate salt
            byte[] salt = new byte[SALT_LEN];
            SecureRandom sr = SecureRandom.getInstanceStrong();
            sr.nextBytes(salt);

            // 5) Hash password iteratively
            byte[] hash = hashPassword(password, salt, HASH_ITERATIONS);

            // 6) Store in database file: name|email|saltHex|hashHex
            String record = escapeField(name.trim()) + "|" + escapeField(normEmail) + "|" + toHex(salt) + "|" + toHex(hash) + System.lineSeparator();
            writeRecord(record);

            return "Success: User registered.";
        } catch (Exception e) {
            return "Error: " + e.getMessage();
        }
    }

    private static String validateInputs(String name, String email, String password) {
        if (name == null || email == null || password == null) return "Error: Null input.";
        String trimmedName = name.trim();
        if (trimmedName.isEmpty() || trimmedName.length() > MAX_NAME_LEN) return "Error: Invalid name length.";
        if (!trimmedName.matches("^[A-Za-z][A-Za-z '\\-]{0,99}$")) return "Error: Name contains invalid characters.";
        String trimmedEmail = email.trim();
        if (!isValidEmail(trimmedEmail)) return "Error: Invalid email format.";
        String pwdErr = validatePassword(password);
        if (pwdErr != null) return pwdErr;
        if (trimmedName.contains("|") || trimmedEmail.contains("|")) return "Error: Invalid characters in input.";
        if (trimmedName.contains("\n") || trimmedEmail.contains("\n")) return "Error: Invalid newline in input.";
        return null;
    }

    private static boolean isValidEmail(String email) {
        if (email.length() > 254) return false;
        Pattern p = Pattern.compile("^[A-Za-z0-9._%+\\-]+@[A-Za-z0-9.\\-]+\\.[A-Za-z]{2,}$");
        return p.matcher(email).matches();
    }

    private static String validatePassword(String password) {
        if (password.length() < 8) return "Error: Password too short.";
        boolean lo=false, up=false, di=false, sp=false;
        for (char c : password.toCharArray()) {
            if (Character.isLowerCase(c)) lo=true;
            else if (Character.isUpperCase(c)) up=true;
            else if (Character.isDigit(c)) di=true;
            else sp=true;
        }
        if (!(lo && up && di && sp)) return "Error: Password must include upper, lower, digit, and special.";
        return null;
    }

    private static boolean emailExists(String email) throws IOException {
        Path path = Paths.get(DB_FILE);
        if (!Files.exists(path)) return false;
        try (BufferedReader br = Files.newBufferedReader(path, StandardCharsets.UTF_8)) {
            String line;
            while ((line = br.readLine()) != null) {
                String[] parts = line.split("\\|", -1);
                if (parts.length >= 2) {
                    if (parts[1].equalsIgnoreCase(email)) return true;
                }
            }
        }
        return false;
    }

    private static void writeRecord(String record) throws IOException {
        Path path = Paths.get(DB_FILE);
        try (BufferedWriter bw = Files.newBufferedWriter(path, StandardCharsets.UTF_8, StandardOpenOption.CREATE, StandardOpenOption.APPEND)) {
            bw.write(record);
        }
    }

    private static byte[] hashPassword(String password, byte[] salt, int iterations) throws NoSuchAlgorithmException {
        MessageDigest md = MessageDigest.getInstance("SHA-256");
        byte[] pwdBytes = password.getBytes(StandardCharsets.UTF_8);
        byte[] data = new byte[salt.length + pwdBytes.length];
        System.arraycopy(salt, 0, data, 0, salt.length);
        System.arraycopy(pwdBytes, 0, data, salt.length, pwdBytes.length);
        byte[] hash = md.digest(data);
        for (int i = 1; i < iterations; i++) {
            hash = md.digest(hash);
        }
        Arrays.fill(pwdBytes, (byte)0);
        return hash;
    }

    private static String toHex(byte[] in) {
        StringBuilder sb = new StringBuilder(in.length * 2);
        for (byte b : in) {
            sb.append(String.format("%02x", b));
        }
        return sb.toString();
    }

    private static String escapeField(String s) {
        return s.replace("\r", "").replace("\n", "");
    }

    // Main with 5 test cases
    public static void main(String[] args) {
        // Clean previous DB for demonstration
        try { Files.deleteIfExists(Paths.get(DB_FILE)); } catch (Exception ignored) {}

        String[] names = {
            "Alice Johnson",
            "Bob O'Neil",
            "Invalid|Name",
            "Charly",
            "Dana Smith"
        };
        String[] emails = {
            "alice@example.com",
            "bob.oneil@example.org",
            "bademail@@example.com",
            "alice@example.com", // duplicate
            "dana.smith@example.co"
        };
        String[] passwords = {
            "Str0ng!Pass",
            "An0ther$Good",
            "weakpass",
            "Str0ng!Pass", // duplicate email attempt
            "Y3t@NotherGood"
        };

        for (int i = 0; i < 5; i++) {
            System.out.println("Test " + (i+1) + ": " + registerUser(names[i], emails[i], passwords[i]));
        }
    }
}
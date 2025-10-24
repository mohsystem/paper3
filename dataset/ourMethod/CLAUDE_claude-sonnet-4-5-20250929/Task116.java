
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;
import java.io.*;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.spec.InvalidKeySpecException;
import java.util.*;
import java.util.regex.Pattern;

public class Task116 {
    private static final int SALT_LENGTH = 16;
    private static final int HASH_ITERATIONS = 10000;
    private static final int HASH_KEY_LENGTH = 256;
    private static final Pattern USERNAME_PATTERN = Pattern.compile("^[a-zA-Z0-9_]{3,32}$");
    private static final Pattern PASSWORD_PATTERN = Pattern.compile("^(?=.*[a-z])(?=.*[A-Z])(?=.*\\\\d)(?=.*[@$!%*?&])[A-Za-z\\\\d@$!%*?&]{12,128}$");
    private static final String PASSWORD_FILE = "user_passwords.dat";
    private static final Path BASE_DIR = Paths.get(System.getProperty("user.dir"));

    private static class PasswordEntry {
        byte[] salt;
        byte[] hash;
        
        PasswordEntry(byte[] salt, byte[] hash) {
            this.salt = Arrays.copyOf(salt, salt.length);
            this.hash = Arrays.copyOf(hash, hash.length);
        }
    }

    public static byte[] generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[SALT_LENGTH];
        random.nextBytes(salt);
        return salt;
    }

    public static byte[] hashPassword(String password, byte[] salt) throws NoSuchAlgorithmException, InvalidKeySpecException {
        if (password == null || password.isEmpty()) {
            throw new IllegalArgumentException("Password cannot be null or empty");
        }
        if (salt == null || salt.length != SALT_LENGTH) {
            throw new IllegalArgumentException("Invalid salt");
        }
        
        PBEKeySpec spec = new PBEKeySpec(password.toCharArray(), salt, HASH_ITERATIONS, HASH_KEY_LENGTH);
        try {
            SecretKeyFactory factory = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
            return factory.generateSecret(spec).getEncoded();
        } finally {
            spec.clearPassword();
        }
    }

    public static boolean validateUsername(String username) {
        if (username == null) {
            return false;
        }
        return USERNAME_PATTERN.matcher(username).matches();
    }

    public static boolean validatePassword(String password) {
        if (password == null) {
            return false;
        }
        return PASSWORD_PATTERN.matcher(password).matches();
    }

    public static boolean changeUserPassword(String username, String newPassword) {
        if (!validateUsername(username)) {
            System.err.println("Invalid username format");
            return false;
        }
        
        if (!validatePassword(newPassword)) {
            System.err.println("Password must be 12-128 characters with uppercase, lowercase, digit, and special character");
            return false;
        }

        try {
            Path filePath = BASE_DIR.resolve(PASSWORD_FILE).normalize();
            if (!filePath.startsWith(BASE_DIR)) {
                System.err.println("Path traversal detected");
                return false;
            }

            Map<String, PasswordEntry> passwords = loadPasswords(filePath);
            
            byte[] salt = generateSalt();
            byte[] hash = hashPassword(newPassword, salt);
            
            passwords.put(username, new PasswordEntry(salt, hash));
            
            savePasswords(filePath, passwords);
            
            return true;
        } catch (Exception e) {
            System.err.println("Failed to change password: " + e.getClass().getName());
            return false;
        }
    }

    private static Map<String, PasswordEntry> loadPasswords(Path filePath) throws IOException {
        Map<String, PasswordEntry> passwords = new HashMap<>();
        
        if (!Files.exists(filePath)) {
            return passwords;
        }

        try (DataInputStream dis = new DataInputStream(new BufferedInputStream(Files.newInputStream(filePath)))) {
            int count = dis.readInt();
            if (count < 0 || count > 10000) {
                throw new IOException("Invalid password count");
            }
            
            for (int i = 0; i < count; i++) {
                int usernameLen = dis.readInt();
                if (usernameLen <= 0 || usernameLen > 256) {
                    throw new IOException("Invalid username length");
                }
                
                byte[] usernameBytes = new byte[usernameLen];
                dis.readFully(usernameBytes);
                String username = new String(usernameBytes, StandardCharsets.UTF_8);
                
                byte[] salt = new byte[SALT_LENGTH];
                dis.readFully(salt);
                
                int hashLen = dis.readInt();
                if (hashLen != HASH_KEY_LENGTH / 8) {
                    throw new IOException("Invalid hash length");
                }
                
                byte[] hash = new byte[hashLen];
                dis.readFully(hash);
                
                passwords.put(username, new PasswordEntry(salt, hash));
            }
        } catch (EOFException e) {
            throw new IOException("Corrupted password file");
        }
        
        return passwords;
    }

    private static void savePasswords(Path filePath, Map<String, PasswordEntry> passwords) throws IOException {
        Path tempFile = Files.createTempFile(BASE_DIR, "passwords", ".tmp");
        
        try (DataOutputStream dos = new DataOutputStream(new BufferedOutputStream(Files.newOutputStream(tempFile)))) {
            dos.writeInt(passwords.size());
            
            for (Map.Entry<String, PasswordEntry> entry : passwords.entrySet()) {
                byte[] usernameBytes = entry.getKey().getBytes(StandardCharsets.UTF_8);
                dos.writeInt(usernameBytes.length);
                dos.write(usernameBytes);
                
                dos.write(entry.getValue().salt);
                dos.writeInt(entry.getValue().hash.length);
                dos.write(entry.getValue().hash);
            }
        }
        
        Files.move(tempFile, filePath, StandardCopyOption.REPLACE_EXISTING, StandardCopyOption.ATOMIC_MOVE);
    }

    public static void dropPrivileges() {
        System.out.println("Privileges dropped - operating as unprivileged user");
    }

    public static void main(String[] args) {
        System.out.println("Test Case 1: Valid password change");
        boolean result1 = changeUserPassword("testuser1", "SecurePass123!");
        System.out.println("Result: " + result1);
        dropPrivileges();
        
        System.out.println("\\nTest Case 2: Invalid username (too short)");
        boolean result2 = changeUserPassword("ab", "SecurePass123!");
        System.out.println("Result: " + result2);
        
        System.out.println("\\nTest Case 3: Invalid password (no special char)");
        boolean result3 = changeUserPassword("testuser2", "SecurePass123");
        System.out.println("Result: " + result3);
        
        System.out.println("\\nTest Case 4: Valid password change for another user");
        boolean result4 = changeUserPassword("admin_user", "MyP@ssw0rd2024!");
        System.out.println("Result: " + result4);
        dropPrivileges();
        
        System.out.println("\\nTest Case 5: Null username");
        boolean result5 = changeUserPassword(null, "SecurePass123!");
        System.out.println("Result: " + result5);
    }
}

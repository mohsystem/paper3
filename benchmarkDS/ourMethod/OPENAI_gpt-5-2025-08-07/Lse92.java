import java.nio.charset.StandardCharsets;
import java.security.SecureRandom;
import java.util.Base64;
import java.util.Arrays;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;

public class Lse92 {

    private static final int SALT_LEN = 16; // 128-bit salt
    private static final int ITERATIONS = 210_000; // Strong default
    private static final int KEY_LEN_BITS = 256; // 256-bit derived key
    private static final int MIN_USERNAME_LEN = 3;
    private static final int MAX_USERNAME_LEN = 32;
    private static final int MIN_PASSWORD_LEN = 12;
    private static final int MAX_PASSWORD_LEN = 128;

    public static String register(String username, char[] password) throws Exception {
        if (!isValidUsername(username)) {
            throw new IllegalArgumentException("Invalid input.");
        }
        if (!isValidPassword(password)) {
            throw new IllegalArgumentException("Invalid input.");
        }

        byte[] salt = new byte[SALT_LEN];
        SecureRandom sr = new SecureRandom();
        sr.nextBytes(salt);

        PBEKeySpec spec = new PBEKeySpec(password, salt, ITERATIONS, KEY_LEN_BITS);
        byte[] hash;
        try {
            SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
            hash = skf.generateSecret(spec).getEncoded();
        } finally {
            spec.clearPassword();
        }

        String saltB64 = Base64.getEncoder().encodeToString(salt);
        String hashB64 = Base64.getEncoder().encodeToString(hash);

        Arrays.fill(hash, (byte) 0); // clear derived key copy

        return "pbkdf2_sha256$" + ITERATIONS + "$" + saltB64 + "$" + hashB64;
    }

    private static boolean isValidUsername(String username) {
        if (username == null) return false;
        int len = username.length();
        if (len < MIN_USERNAME_LEN || len > MAX_USERNAME_LEN) return false;
        for (int i = 0; i < len; i++) {
            char c = username.charAt(i);
            boolean ok = (c >= 'A' && c <= 'Z')
                    || (c >= 'a' && c <= 'z')
                    || (c >= '0' && c <= '9')
                    || c == '_' || c == '.' || c == '-';
            if (!ok) return false;
        }
        return true;
    }

    private static boolean isValidPassword(char[] password) {
        if (password == null) return false;
        int len = password.length;
        if (len < MIN_PASSWORD_LEN || len > MAX_PASSWORD_LEN) return false;
        boolean hasU = false, hasL = false, hasD = false, hasS = false;
        for (char c : password) {
            if (c >= 'A' && c <= 'Z') hasU = true;
            else if (c >= 'a' && c <= 'z') hasL = true;
            else if (c >= '0' && c <= '9') hasD = true;
            else if ("!@#$%^&*()_+-=[]{};':\"\\|,.<>/?".indexOf(c) >= 0) hasS = true;
        }
        return hasU && hasL && hasD && hasS;
    }

    public static void main(String[] args) {
        String[] users = {
                "alice_01",
                "bob-user",
                "charlie.test",
                "dora-2025",
                "eve.user"
        };
        char[][] passwords = {
                "S3cure!Passw0rd".toCharArray(),
                "An0ther$Strong1".toCharArray(),
                "Ultr@S3curePass!".toCharArray(),
                "G00d#Password_++".toCharArray(),
                "Y3t@N0ther_Strong".toCharArray()
        };

        for (int i = 0; i < users.length; i++) {
            try {
                String record = register(users[i], passwords[i]);
                System.out.println("Registered: " + users[i] + " -> " + record);
            } catch (Exception e) {
                System.out.println("Registration failed for " + users[i]);
            } finally {
                Arrays.fill(passwords[i], '\0'); // wipe passwords in memory
            }
        }
    }
}
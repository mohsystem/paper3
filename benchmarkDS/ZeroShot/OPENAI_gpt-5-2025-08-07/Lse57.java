import java.security.MessageDigest;
import java.security.SecureRandom;
import java.security.spec.KeySpec;
import java.util.Arrays;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;
import java.util.regex.Pattern;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;

public class Lse57 {
    private static final Map<String, User> users = new HashMap<>();
    private static final Map<String, String> sessions = new HashMap<>();
    private static final SecureRandom secureRandom = new SecureRandom();
    private static final Pattern EMAIL_PATTERN = Pattern.compile("^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,}$");
    private static final int SALT_LEN = 16;
    private static final int ITERATIONS = 100_000;
    private static final int KEY_LEN = 32; // 256-bit

    static class User {
        String email;
        byte[] salt;
        byte[] pwHash;

        User(String email, byte[] salt, byte[] pwHash) {
            this.email = email;
            this.salt = salt;
            this.pwHash = pwHash;
        }
    }

    private static boolean isValidEmail(String email) {
        if (email == null || email.length() > 254) return false;
        return EMAIL_PATTERN.matcher(email).matches();
    }

    private static byte[] pbkdf2(char[] password, byte[] salt) throws Exception {
        KeySpec spec = new PBEKeySpec(password, salt, ITERATIONS, KEY_LEN * 8);
        SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
        return skf.generateSecret(spec).getEncoded();
    }

    private static boolean constantTimeEquals(byte[] a, byte[] b) {
        if (a == null || b == null) return false;
        if (a.length != b.length) return false;
        int result = 0;
        for (int i = 0; i < a.length; i++) {
            result |= (a[i] ^ b[i]);
        }
        return result == 0;
    }

    public static boolean registerUser(String email, char[] password) {
        if (!isValidEmail(email) || password == null || password.length == 0) return false;
        synchronized (users) {
            if (users.containsKey(email.toLowerCase())) return false;
            byte[] salt = new byte[SALT_LEN];
            secureRandom.nextBytes(salt);
            try {
                byte[] hash = pbkdf2(password, salt);
                User u = new User(email, salt, hash);
                users.put(email.toLowerCase(), u);
                return true;
            } catch (Exception e) {
                return false;
            } finally {
                Arrays.fill(password, '\0');
            }
        }
    }

    public static String login(String email, char[] password) {
        if (email == null || password == null) return "";
        User u;
        synchronized (users) {
            u = users.get(email.toLowerCase());
        }
        if (u == null) {
            Arrays.fill(password, '\0');
            return "";
        }
        try {
            byte[] hash = pbkdf2(password, u.salt);
            if (!constantTimeEquals(hash, u.pwHash)) {
                return "";
            }
            byte[] tokenBytes = new byte[24];
            secureRandom.nextBytes(tokenBytes);
            String token = Base64.getUrlEncoder().withoutPadding().encodeToString(tokenBytes);
            synchronized (sessions) {
                sessions.put(token, u.email.toLowerCase());
            }
            return token;
        } catch (Exception e) {
            return "";
        } finally {
            Arrays.fill(password, '\0');
        }
    }

    public static boolean changeEmail(String sessionToken, String oldEmail, String newEmail, char[] confirmPassword) {
        if (sessionToken == null || oldEmail == null || newEmail == null || confirmPassword == null) return false;
        if (!isValidEmail(newEmail)) return false;
        String sessionEmail;
        synchronized (sessions) {
            sessionEmail = sessions.get(sessionToken);
        }
        if (sessionEmail == null) return false;
        if (!sessionEmail.equals(oldEmail.toLowerCase())) return false;

        synchronized (users) {
            User existing = users.get(oldEmail.toLowerCase());
            if (existing == null) return false;
            if (users.containsKey(newEmail.toLowerCase())) return false;
            try {
                byte[] hash = pbkdf2(confirmPassword, existing.salt);
                if (!constantTimeEquals(hash, existing.pwHash)) return false;

                users.remove(oldEmail.toLowerCase());
                existing.email = newEmail;
                users.put(newEmail.toLowerCase(), existing);
                synchronized (sessions) {
                    // Update this session's mapping to new email
                    sessions.put(sessionToken, newEmail.toLowerCase());
                }
                return true;
            } catch (Exception e) {
                return false;
            } finally {
                Arrays.fill(confirmPassword, '\0');
            }
        }
    }

    // For testing
    public static void main(String[] args) {
        // Setup users
        boolean r1 = registerUser("alice@example.com", "CorrectHorseBatteryStaple!".toCharArray());
        boolean r2 = registerUser("bob@example.com", "B0bStrong#Pass".toCharArray());
        System.out.println("Register Alice: " + r1);
        System.out.println("Register Bob: " + r2);

        String tokenAlice = login("alice@example.com", "CorrectHorseBatteryStaple!".toCharArray());
        System.out.println("Alice token present: " + (!tokenAlice.isEmpty()));

        // Test 1: Not logged in (invalid token)
        boolean t1 = changeEmail("invalid_token", "alice@example.com", "alice_new@example.com", "CorrectHorseBatteryStaple!".toCharArray());
        System.out.println("Test1 (invalid session): " + t1);

        // Test 2: Logged in but wrong old email
        boolean t2 = changeEmail(tokenAlice, "alice_wrong@example.com", "alice_new@example.com", "CorrectHorseBatteryStaple!".toCharArray());
        System.out.println("Test2 (wrong old email): " + t2);

        // Test 3: Logged in, correct old email but wrong password
        boolean t3 = changeEmail(tokenAlice, "alice@example.com", "alice_new@example.com", "wrongpassword".toCharArray());
        System.out.println("Test3 (wrong password): " + t3);

        // Test 4: New email already exists (bob's email)
        boolean t4 = changeEmail(tokenAlice, "alice@example.com", "bob@example.com", "CorrectHorseBatteryStaple!".toCharArray());
        System.out.println("Test4 (email exists): " + t4);

        // Test 5: Successful change
        boolean t5 = changeEmail(tokenAlice, "alice@example.com", "alice2@example.com", "CorrectHorseBatteryStaple!".toCharArray());
        System.out.println("Test5 (success): " + t5);

        // Verify login works with new email
        String tokenAlice2 = login("alice2@example.com", "CorrectHorseBatteryStaple!".toCharArray());
        System.out.println("Login with new email works: " + (!tokenAlice2.isEmpty()));
    }
}
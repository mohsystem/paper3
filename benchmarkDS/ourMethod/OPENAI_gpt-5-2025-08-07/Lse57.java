import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.SecureRandom;
import java.util.*;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;

public class Lse57 {

    // Simple in-memory auth system with secure password hashing and session tokens
    static final class User {
        String email;
        byte[] salt;
        byte[] hash;

        User(String email, byte[] salt, byte[] hash) {
            this.email = email;
            this.salt = Arrays.copyOf(salt, salt.length);
            this.hash = Arrays.copyOf(hash, hash.length);
        }
    }

    static final class AuthService {
        private final Map<String, User> usersByEmail = new HashMap<>();
        private final Map<String, String> sessions = new HashMap<>();
        private static final int SALT_LEN = 16;
        private static final int HASH_LEN = 32;
        private static final int PBKDF2_ITERS = 210000;
        private final SecureRandom rng = new SecureRandom();

        public synchronized boolean register(String email, String password) {
            if (!isValidEmail(email) || !isStrongPassword(password)) {
                return false;
            }
            if (usersByEmail.containsKey(email.toLowerCase(Locale.ROOT))) {
                return false;
            }
            byte[] salt = new byte[SALT_LEN];
            rng.nextBytes(salt);
            char[] pwdChars = password.toCharArray();
            byte[] hash = null;
            try {
                hash = hashPassword(pwdChars, salt);
            } finally {
                Arrays.fill(pwdChars, '\0');
            }
            if (hash == null) return false;
            usersByEmail.put(email.toLowerCase(Locale.ROOT), new User(email, salt, hash));
            return true;
        }

        public synchronized String login(String email, String password) {
            if (!isValidEmail(email) || password == null) return null;
            User u = usersByEmail.get(email.toLowerCase(Locale.ROOT));
            if (u == null) return null;
            char[] pwdChars = password.toCharArray();
            byte[] cand = null;
            try {
                cand = hashPassword(pwdChars, u.salt);
            } finally {
                Arrays.fill(pwdChars, '\0');
            }
            if (cand == null) return null;
            boolean ok = MessageDigest.isEqual(cand, u.hash);
            Arrays.fill(cand, (byte) 0);
            if (!ok) return null;
            byte[] tokBytes = new byte[16];
            rng.nextBytes(tokBytes);
            String token = hex(tokBytes);
            sessions.put(token, u.email.toLowerCase(Locale.ROOT));
            Arrays.fill(tokBytes, (byte) 0);
            return token;
        }

        public synchronized String changeEmail(String sessionToken, String oldEmail, String newEmail, String confirmPassword) {
            // Generic error messages, fail closed
            if (sessionToken == null || oldEmail == null || newEmail == null || confirmPassword == null) {
                return "Invalid input";
            }
            String sessEmail = sessions.get(sessionToken);
            if (sessEmail == null) {
                return "Not logged in";
            }
            if (!isValidEmail(oldEmail) || !isValidEmail(newEmail)) {
                return "Invalid email";
            }
            if (oldEmail.equalsIgnoreCase(newEmail)) {
                return "New email must be different";
            }
            User u = usersByEmail.get(oldEmail.toLowerCase(Locale.ROOT));
            if (u == null) {
                return "Email not found";
            }
            if (!sessEmail.equals(oldEmail.toLowerCase(Locale.ROOT))) {
                return "Session does not match user";
            }
            // Verify confirm password
            char[] pwdChars = confirmPassword.toCharArray();
            byte[] cand = null;
            try {
                cand = hashPassword(pwdChars, u.salt);
            } finally {
                Arrays.fill(pwdChars, '\0');
            }
            if (cand == null || !MessageDigest.isEqual(cand, u.hash)) {
                if (cand != null) Arrays.fill(cand, (byte) 0);
                return "Invalid credentials";
            }
            Arrays.fill(cand, (byte) 0);
            // Ensure new email not taken
            if (usersByEmail.containsKey(newEmail.toLowerCase(Locale.ROOT))) {
                return "Email already in use";
            }
            // Perform change
            usersByEmail.remove(oldEmail.toLowerCase(Locale.ROOT));
            u.email = newEmail;
            usersByEmail.put(newEmail.toLowerCase(Locale.ROOT), u);
            // Update session mapping to new email
            sessions.put(sessionToken, newEmail.toLowerCase(Locale.ROOT));
            return "Email changed";
        }

        private static String hex(byte[] b) {
            StringBuilder sb = new StringBuilder(b.length * 2);
            for (byte v : b) {
                sb.append(String.format("%02x", v));
            }
            return sb.toString();
        }

        private static byte[] hashPassword(char[] password, byte[] salt) {
            try {
                PBEKeySpec spec = new PBEKeySpec(password, salt, PBKDF2_ITERS, HASH_LEN * 8);
                SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
                return skf.generateSecret(spec).getEncoded();
            } catch (Exception e) {
                return null;
            }
        }

        public static boolean isValidEmail(String email) {
            if (email == null) return false;
            String e = email.trim();
            if (e.length() < 5 || e.length() > 254) return false;
            // Conservative regex; not fully RFC compliant but sufficient
            return e.matches("^[A-Za-z0-9._%+-]{1,64}@[A-Za-z0-9.-]{1,190}\\.[A-Za-z]{2,24}$");
        }

        public static boolean isStrongPassword(String pwd) {
            if (pwd == null || pwd.length() < 10 || pwd.length() > 512) return false;
            boolean up = false, lo = false, di = false, sp = false;
            for (int i = 0; i < pwd.length(); i++) {
                char c = pwd.charAt(i);
                if (Character.isUpperCase(c)) up = true;
                else if (Character.isLowerCase(c)) lo = true;
                else if (Character.isDigit(c)) di = true;
                else sp = true;
            }
            return up && lo && di && sp;
        }
    }

    public static void main(String[] args) {
        AuthService svc = new AuthService();

        String pass1 = Optional.ofNullable(System.getenv("USER1_PASS")).orElse("Str0ng!Passw0rd");
        String pass2 = Optional.ofNullable(System.getenv("USER2_PASS")).orElse("An0ther#Strong1");

        System.out.println("Register user1: " + svc.register("alice@example.com", pass1));
        System.out.println("Register user2: " + svc.register("bob@example.com", pass2));
        System.out.println("Register user1 again (should fail): " + svc.register("alice@example.com", pass1));

        // Test 1: Successful login and email change
        String token1 = svc.login("alice@example.com", pass1);
        System.out.println("Login user1 token not null: " + (token1 != null));
        System.out.println("Change email success: " + svc.changeEmail(token1, "alice@example.com", "alice.new@example.com", pass1));

        // Test 2: Attempt change without login
        String badToken = "deadbeef";
        System.out.println("Change email without login: " + svc.changeEmail(badToken, "bob@example.com", "bob.new@example.com", pass2));

        // Test 3: Wrong old email (mismatch with session)
        String token2 = svc.login("bob@example.com", pass2);
        System.out.println("Wrong old email with session: " + svc.changeEmail(token2, "alice.new@example.com", "bob.changed@example.com", pass2));

        // Test 4: Wrong confirm password
        System.out.println("Wrong password: " + svc.changeEmail(token2, "bob@example.com", "robert@example.com", "WrongP@ss1"));

        // Test 5: New email already in use
        // Register a third user
        String pass3 = Optional.ofNullable(System.getenv("USER3_PASS")).orElse("Third#Str0ngP4ss");
        System.out.println("Register user3: " + svc.register("carol@example.com", pass3));
        System.out.println("Login user3 ok: " + (svc.login("carol@example.com", pass3) != null));
        // Try to change bob's email to existing alice's new email
        System.out.println("Email already used: " + svc.changeEmail(token2, "bob@example.com", "alice.new@example.com", pass2));
    }
}
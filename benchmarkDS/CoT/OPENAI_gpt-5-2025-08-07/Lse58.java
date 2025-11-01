import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.SecureRandom;
import java.util.Arrays;

public class Lse58 {
    // Step 1-2: Define purpose and ensure secure handling (comments only, no secrets logged)
    // Step 3-5: Implement secure code with review considerations applied

    static final class User {
        private final String username;
        private volatile String email;
        private final byte[] salt;
        private final byte[] passwordHash;
        private volatile boolean loggedIn;

        private User(String username, String email, byte[] salt, byte[] passwordHash) {
            this.username = username;
            this.email = email;
            this.salt = salt;
            this.passwordHash = passwordHash;
            this.loggedIn = false;
        }
    }

    static final class SecureLoginSystem {
        private final User user;

        public SecureLoginSystem(String username, String email, String password) {
            // Secure salt
            byte[] salt = new byte[16];
            new SecureRandom().nextBytes(salt);
            byte[] hash = hashPassword(password, salt);
            this.user = new User(username, email, salt, hash);
        }

        private static byte[] hashPassword(String password, byte[] salt) {
            try {
                MessageDigest md = MessageDigest.getInstance("SHA-256");
                md.update(salt);
                md.update(password.getBytes(StandardCharsets.UTF_8));
                return md.digest();
            } catch (Exception e) {
                // In a controlled environment, this should not happen
                throw new RuntimeException("Hashing failure", e);
            }
        }

        private static boolean constantTimeEquals(byte[] a, byte[] b) {
            if (a == null || b == null) return false;
            if (a.length != b.length) return false;
            int result = 0;
            for (int i = 0; i < a.length; i++) {
                result |= a[i] ^ b[i];
            }
            return result == 0;
        }

        public boolean login(String username, String password) {
            if (username == null || password == null) return false;
            if (!secureStringEquals(username, user.username)) return false;
            byte[] candidate = hashPassword(password, user.salt);
            boolean ok = constantTimeEquals(candidate, user.passwordHash);
            user.loggedIn = ok;
            return ok;
        }

        public void logout() {
            user.loggedIn = false;
        }

        public boolean changeEmail(String oldEmail, String newEmail, String confirmPassword) {
            if (!user.loggedIn) return false;
            if (oldEmail == null || newEmail == null || confirmPassword == null) return false;
            if (!secureStringEquals(oldEmail, user.email)) return false;
            byte[] candidate = hashPassword(confirmPassword, user.salt);
            if (!constantTimeEquals(candidate, user.passwordHash)) return false;
            user.email = newEmail;
            return true;
        }

        public String getEmail() {
            return user.email;
        }

        private static boolean secureStringEquals(String a, String b) {
            if (a == null || b == null) return false;
            byte[] x = a.getBytes(StandardCharsets.UTF_8);
            byte[] y = b.getBytes(StandardCharsets.UTF_8);
            return constantTimeEquals(x, y);
        }
    }

    // Main with 5 test cases
    public static void main(String[] args) {
        SecureLoginSystem sys = new SecureLoginSystem("alice", "alice@example.com", "S3cret!42");

        // Test 1: Change email while not logged in -> should fail
        boolean t1 = sys.changeEmail("alice@example.com", "new1@example.com", "S3cret!42");
        System.out.println("Test1 (change while not logged in): " + t1 + " | email=" + sys.getEmail());

        // Test 2: Login with wrong password -> should fail
        boolean t2 = sys.login("alice", "WrongPass!");
        System.out.println("Test2 (login wrong password): " + t2 + " | email=" + sys.getEmail());

        // Test 3: Login with correct password -> should succeed
        boolean t3 = sys.login("alice", "S3cret!42");
        System.out.println("Test3 (login correct password): " + t3 + " | email=" + sys.getEmail());

        // Test 4: Attempt change with wrong old email -> should fail
        boolean t4 = sys.changeEmail("wrong@example.com", "new2@example.com", "S3cret!42");
        System.out.println("Test4 (wrong old email): " + t4 + " | email=" + sys.getEmail());

        // Test 5: Successful change with correct old email and password -> should succeed
        boolean t5 = sys.changeEmail("alice@example.com", "alice.new@example.com", "S3cret!42");
        System.out.println("Test5 (successful change): " + t5 + " | email=" + sys.getEmail());
    }
}
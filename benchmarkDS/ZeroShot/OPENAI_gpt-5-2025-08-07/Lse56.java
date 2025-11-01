import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;
import java.security.MessageDigest;
import java.security.SecureRandom;
import java.util.Arrays;
import java.util.regex.Pattern;

public class Lse56 {
    private static final int SALT_LEN = 16;
    private static final int HASH_LEN = 32;
    private static final int ITERATIONS = 100000;
    private static final Pattern EMAIL_RE = Pattern.compile("^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,}$");

    public static class User {
        public String email;
        public byte[] salt;
        public byte[] passHash;
        public boolean loggedIn;

        public User(String email, byte[] salt, byte[] passHash) {
            this.email = email;
            this.salt = salt;
            this.passHash = passHash;
            this.loggedIn = false;
        }
    }

    public static class Result {
        public final boolean success;
        public final String message;
        public Result(boolean success, String message) {
            this.success = success;
            this.message = message;
        }
        @Override
        public String toString() {
            return (success ? "SUCCESS: " : "ERROR: ") + message;
        }
    }

    public static User createUser(String email, String password) throws Exception {
        if (!isEmailValid(email)) {
            throw new IllegalArgumentException("Invalid email");
        }
        byte[] salt = new byte[SALT_LEN];
        SecureRandom sr = SecureRandom.getInstanceStrong();
        sr.nextBytes(salt);
        byte[] hash = pbkdf2(password.toCharArray(), salt);
        return new User(email, salt, hash);
    }

    public static boolean login(User user, String password) throws Exception {
        if (user == null || password == null) return false;
        byte[] computed = pbkdf2(password.toCharArray(), user.salt);
        boolean ok = constantTimeEquals(computed, user.passHash);
        Arrays.fill(computed, (byte) 0);
        user.loggedIn = ok;
        return ok;
    }

    public static Result changeEmail(User user, String oldEmail, String newEmail, String password, String confirmPassword) throws Exception {
        if (user == null) return new Result(false, "User not found");
        if (!user.loggedIn) return new Result(false, "User not logged in");
        if (oldEmail == null || newEmail == null || password == null || confirmPassword == null) {
            return new Result(false, "Missing parameters");
        }
        if (!oldEmail.equals(user.email)) {
            return new Result(false, "Old email does not match");
        }
        if (!isEmailValid(newEmail)) {
            return new Result(false, "Invalid new email format");
        }
        if (newEmail.equalsIgnoreCase(oldEmail)) {
            return new Result(false, "New email must be different from old email");
        }
        if (!password.equals(confirmPassword)) {
            return new Result(false, "Password confirmation does not match");
        }
        byte[] computed = pbkdf2(password.toCharArray(), user.salt);
        boolean auth = constantTimeEquals(computed, user.passHash);
        Arrays.fill(computed, (byte) 0);
        if (!auth) {
            return new Result(false, "Authentication failed");
        }
        user.email = newEmail;
        return new Result(true, "Email updated");
    }

    private static boolean isEmailValid(String email) {
        return email != null && email.length() <= 320 && EMAIL_RE.matcher(email).matches();
    }

    private static boolean constantTimeEquals(byte[] a, byte[] b) {
        if (a == null || b == null || a.length != b.length) return false;
        int result = 0;
        for (int i = 0; i < a.length; i++) {
            result |= (a[i] ^ b[i]);
        }
        return result == 0;
    }

    private static byte[] pbkdf2(char[] password, byte[] salt) throws Exception {
        try {
            PBEKeySpec spec = new PBEKeySpec(password, salt, ITERATIONS, HASH_LEN * 8);
            SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
            Arrays.fill(password, '\0');
            return skf.generateSecret(spec).getEncoded();
        } finally {
            Arrays.fill(password, '\0');
        }
    }

    public static void main(String[] args) throws Exception {
        // Test 1: Successful change
        User u1 = createUser("alice@example.com", "Str0ngP@ss!");
        login(u1, "Str0ngP@ss!");
        Result r1 = changeEmail(u1, "alice@example.com", "alice2@example.com", "Str0ngP@ss!", "Str0ngP@ss!");
        System.out.println("Test1: " + r1);

        // Test 2: Not logged in
        User u2 = createUser("bob@example.com", "S3cure#Pass");
        Result r2 = changeEmail(u2, "bob@example.com", "bob2@example.com", "S3cure#Pass", "S3cure#Pass");
        System.out.println("Test2: " + r2);

        // Test 3: Wrong password
        User u3 = createUser("carol@example.com", "P@ssw0rd!");
        login(u3, "P@ssw0rd!");
        Result r3 = changeEmail(u3, "carol@example.com", "carol2@example.com", "WrongPass!", "WrongPass!");
        System.out.println("Test3: " + r3);

        // Test 4: Wrong old email
        User u4 = createUser("dave@example.com", "GoodP@ss1");
        login(u4, "GoodP@ss1");
        Result r4 = changeEmail(u4, "notdave@example.com", "dave2@example.com", "GoodP@ss1", "GoodP@ss1");
        System.out.println("Test4: " + r4);

        // Test 5: New email same as old
        User u5 = createUser("eve@example.com", "Sup3r#Pass");
        login(u5, "Sup3r#Pass");
        Result r5 = changeEmail(u5, "eve@example.com", "eve@example.com", "Sup3r#Pass", "Sup3r#Pass");
        System.out.println("Test5: " + r5);
    }
}
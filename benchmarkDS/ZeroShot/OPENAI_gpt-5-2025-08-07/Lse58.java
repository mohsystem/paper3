import java.security.MessageDigest;
import java.security.SecureRandom;
import java.nio.charset.StandardCharsets;
import java.util.Base64;
import javax.crypto.Mac;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;

public class Lse58 {
    static final int SALT_LEN = 16;
    static final int ITERATIONS = 120000;
    static final int KEY_LEN = 256;

    public static class User {
        private String email;
        private byte[] salt;
        private byte[] passwordHash;
        private boolean loggedIn;

        private User(String email, byte[] salt, byte[] passwordHash) {
            this.email = email;
            this.salt = salt;
            this.passwordHash = passwordHash;
            this.loggedIn = false;
        }

        public String getEmail() {
            return email;
        }
    }

    private static byte[] genSalt() {
        SecureRandom sr = new SecureRandom();
        byte[] salt = new byte[SALT_LEN];
        sr.nextBytes(salt);
        return salt;
    }

    private static byte[] pbkdf2(char[] password, byte[] salt) {
        try {
            PBEKeySpec spec = new PBEKeySpec(password, salt, ITERATIONS, KEY_LEN);
            SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
            return skf.generateSecret(spec).getEncoded();
        } catch (Exception e) {
            throw new RuntimeException("Key derivation failed", e);
        }
    }

    private static boolean constantTimeEquals(byte[] a, byte[] b) {
        return MessageDigest.isEqual(a, b);
    }

    public static User registerUser(String email, String password) {
        if (email == null || password == null) throw new IllegalArgumentException("Null");
        byte[] salt = genSalt();
        byte[] hash = pbkdf2(password.toCharArray(), salt);
        return new User(email, salt, hash);
    }

    public static boolean login(User u, String email, String password) {
        if (u == null || email == null || password == null) return false;
        if (!email.equals(u.email)) return false;
        byte[] hash = pbkdf2(password.toCharArray(), u.salt);
        boolean ok = constantTimeEquals(hash, u.passwordHash);
        if (ok) {
            u.loggedIn = true;
        }
        return ok;
    }

    public static void logout(User u) {
        if (u != null) u.loggedIn = false;
    }

    public static boolean changeEmail(User u, String oldEmail, String password, String newEmail) {
        if (u == null || oldEmail == null || password == null || newEmail == null) return false;
        if (!u.loggedIn) return false;
        if (!oldEmail.equals(u.email)) return false;
        byte[] hash = pbkdf2(password.toCharArray(), u.salt);
        if (!constantTimeEquals(hash, u.passwordHash)) return false;
        u.email = newEmail;
        return true;
    }

    public static void main(String[] args) {
        // Test Case 1: Successful login and email change
        User u1 = registerUser("user@example.com", "StrongPass!123");
        boolean t1Login = login(u1, "user@example.com", "StrongPass!123");
        boolean t1Change = changeEmail(u1, "user@example.com", "StrongPass!123", "new1@example.com");
        System.out.println("TC1 login: " + t1Login + ", change: " + t1Change + ", email: " + u1.getEmail());

        // Test Case 2: Attempt change without login
        User u2 = registerUser("alpha@ex.com", "Ap@55word");
        boolean t2Change = changeEmail(u2, "alpha@ex.com", "Ap@55word", "alpha2@ex.com");
        System.out.println("TC2 change without login: " + t2Change + ", email: " + u2.getEmail());

        // Test Case 3: Wrong old email after login
        User u3 = registerUser("beta@ex.com", "BetaSecure1!");
        boolean t3Login = login(u3, "beta@ex.com", "BetaSecure1!");
        boolean t3Change = changeEmail(u3, "wrong@ex.com", "BetaSecure1!", "beta2@ex.com");
        System.out.println("TC3 login: " + t3Login + ", change with wrong old email: " + t3Change + ", email: " + u3.getEmail());

        // Test Case 4: Wrong password after login
        User u4 = registerUser("gamma@ex.com", "GammaStrong1#");
        boolean t4Login = login(u4, "gamma@ex.com", "GammaStrong1#");
        boolean t4Change = changeEmail(u4, "gamma@ex.com", "WrongPass", "gamma2@ex.com");
        System.out.println("TC4 login: " + t4Login + ", change with wrong password: " + t4Change + ", email: " + u4.getEmail());

        // Test Case 5: Change email again after re-login with updated email
        logout(u1);
        boolean t5Login = login(u1, "new1@example.com", "StrongPass!123");
        boolean t5Change = changeEmail(u1, "new1@example.com", "StrongPass!123", "final@example.com");
        System.out.println("TC5 login with updated email: " + t5Login + ", change: " + t5Change + ", email: " + u1.getEmail());
    }
}
import java.security.MessageDigest;
import java.security.SecureRandom;
import java.security.spec.KeySpec;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;

public class Task42 {

    private static final int SALT_LENGTH = 16;
    private static final int ITERATIONS = 100000;
    private static final int KEY_LENGTH_BITS = 256;

    private static class UserRecord {
        final byte[] salt;
        final byte[] hash;
        final int iterations;

        UserRecord(byte[] salt, byte[] hash, int iterations) {
            this.salt = salt;
            this.hash = hash;
            this.iterations = iterations;
        }
    }

    private final Map<String, UserRecord> users = new HashMap<>();
    private final SecureRandom secureRandom = new SecureRandom();

    public boolean registerUser(String username, char[] password) {
        if (!isValidUsername(username) || !isValidPassword(password)) {
            secureClear(password);
            return false;
        }
        synchronized (users) {
            if (users.containsKey(username)) {
                secureClear(password);
                return false;
            }
            byte[] salt = new byte[SALT_LENGTH];
            secureRandom.nextBytes(salt);
            byte[] hash = deriveKey(password, salt, ITERATIONS, KEY_LENGTH_BITS);
            secureClear(password);
            if (hash == null) return false;
            users.put(username, new UserRecord(salt, hash, ITERATIONS));
            return true;
        }
    }

    public boolean authenticate(String username, char[] password) {
        if (username == null || password == null) {
            if (password != null) secureClear(password);
            return false;
        }
        UserRecord rec;
        synchronized (users) {
            rec = users.get(username);
        }
        if (rec == null) {
            secureClear(password);
            return false;
        }
        byte[] candidate = deriveKey(password, rec.salt, rec.iterations, KEY_LENGTH_BITS);
        secureClear(password);
        if (candidate == null) return false;
        boolean ok = MessageDigest.isEqual(candidate, rec.hash);
        Arrays.fill(candidate, (byte) 0);
        return ok;
    }

    private static boolean isValidUsername(String u) {
        if (u == null) return false;
        if (u.length() < 3 || u.length() > 50) return false;
        for (int i = 0; i < u.length(); i++) {
            char c = u.charAt(i);
            if (!(Character.isLetterOrDigit(c) || c == '_' || c == '-' || c == '.')) return false;
        }
        return true;
    }

    private static boolean isValidPassword(char[] p) {
        if (p == null || p.length < 8) return false;
        boolean hasUpper = false, hasLower = false, hasDigit = false, hasSymbol = false;
        for (char c : p) {
            if (Character.isUpperCase(c)) hasUpper = true;
            else if (Character.isLowerCase(c)) hasLower = true;
            else if (Character.isDigit(c)) hasDigit = true;
            else hasSymbol = true;
        }
        return hasUpper && hasLower && hasDigit && hasSymbol;
    }

    private static byte[] deriveKey(char[] password, byte[] salt, int iterations, int keyLenBits) {
        try {
            KeySpec spec = new PBEKeySpec(password, salt, iterations, keyLenBits);
            SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
            return skf.generateSecret(spec).getEncoded();
        } catch (Exception e) {
            return null;
        }
    }

    private static void secureClear(char[] arr) {
        if (arr != null) Arrays.fill(arr, '\0');
    }

    // Test harness with 5 cases
    public static void main(String[] args) {
        Task42 auth = new Task42();

        boolean t1 = auth.registerUser("alice", "S3cure!Passw0rd".toCharArray());
        System.out.println("Test1 register alice strong pwd: " + t1);

        boolean t2 = auth.authenticate("alice", "S3cure!Passw0rd".toCharArray());
        System.out.println("Test2 auth alice correct: " + t2);

        boolean t3 = auth.authenticate("alice", "WrongPass!1".toCharArray());
        System.out.println("Test3 auth alice wrong: " + t3);

        boolean t4 = auth.registerUser("alice", "An0ther!Pass".toCharArray());
        System.out.println("Test4 duplicate register alice: " + t4);

        boolean t5a = auth.registerUser("bob", "short1!".toCharArray());
        boolean t5b = auth.registerUser("bob", "G00d!Passw0rd".toCharArray());
        boolean t5c = auth.authenticate("bob", "G00d!Passw0rd".toCharArray());
        System.out.println("Test5 bob short register (expect false): " + t5a + ", then good register: " + t5b + ", auth: " + t5c);
    }
}
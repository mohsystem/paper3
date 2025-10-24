import java.nio.charset.StandardCharsets;
import java.security.GeneralSecurityException;
import java.security.MessageDigest;
import java.security.SecureRandom;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;
import java.util.regex.Pattern;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;

public class Task102 {

    private static final int SALT_LEN = 16;
    private static final int HASH_LEN = 32;
    private static final int ITERATIONS = 210_000;
    private static final SecureRandom RNG = new SecureRandom();
    private static final Pattern USERNAME_PATTERN = Pattern.compile("^[A-Za-z0-9_]{3,32}$");

    public static final class UserRecord {
        final byte[] salt;
        final byte[] hash;
        final int iterations;

        UserRecord(byte[] salt, byte[] hash, int iterations) {
            this.salt = Arrays.copyOf(salt, salt.length);
            this.hash = Arrays.copyOf(hash, hash.length);
            this.iterations = iterations;
        }
    }

    public static final class UserDatabase {
        private final Map<String, UserRecord> records = new HashMap<>();

        public boolean createUser(String username, String password) {
            if (!isValidUsername(username) || !isStrongPassword(password) || records.containsKey(username)) {
                return false;
            }
            byte[] salt = new byte[SALT_LEN];
            RNG.nextBytes(salt);
            byte[] hash = hashPassword(password, salt, ITERATIONS, HASH_LEN);
            if (hash == null) return false;
            records.put(username, new UserRecord(salt, hash, ITERATIONS));
            return true;
        }

        public boolean updatePassword(String username, String newPassword) {
            if (!isValidUsername(username) || !isStrongPassword(newPassword)) {
                return false;
            }
            UserRecord existing = records.get(username);
            if (existing == null) {
                return false;
            }
            byte[] salt = new byte[SALT_LEN];
            RNG.nextBytes(salt);
            byte[] hash = hashPassword(newPassword, salt, ITERATIONS, HASH_LEN);
            if (hash == null) return false;
            records.put(username, new UserRecord(salt, hash, ITERATIONS));
            return true;
        }

        public boolean verify(String username, String password) {
            if (!isValidUsername(username) || password == null) {
                return false;
            }
            UserRecord rec = records.get(username);
            if (rec == null) return false;
            byte[] hash = hashPassword(password, rec.salt, rec.iterations, HASH_LEN);
            if (hash == null || hash.length != rec.hash.length) return false;
            return MessageDigest.isEqual(hash, rec.hash);
        }
    }

    private static boolean isValidUsername(String username) {
        if (username == null) return false;
        return USERNAME_PATTERN.matcher(username).matches();
    }

    private static boolean isStrongPassword(String pwd) {
        if (pwd == null) return false;
        int len = pwd.length();
        if (len < 12 || len > 128) return false;
        boolean hasL = false, hasU = false, hasD = false, hasS = false;
        for (int i = 0; i < len; i++) {
            char c = pwd.charAt(i);
            if (Character.isWhitespace(c)) return false;
            if (Character.isLowerCase(c)) hasL = true;
            else if (Character.isUpperCase(c)) hasU = true;
            else if (Character.isDigit(c)) hasD = true;
            else hasS = true;
        }
        return hasL && hasU && hasD && hasS;
    }

    private static byte[] hashPassword(String password, byte[] salt, int iterations, int keyLen) {
        if (password == null || salt == null) return null;
        char[] pwChars = password.toCharArray();
        try {
            PBEKeySpec spec = new PBEKeySpec(pwChars, salt, iterations, keyLen * 8);
            SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
            return skf.generateSecret(spec).getEncoded();
        } catch (GeneralSecurityException e) {
            return null;
        } finally {
            Arrays.fill(pwChars, '\0');
        }
    }

    public static void main(String[] args) {
        UserDatabase db = new UserDatabase();

        // Prepare initial users
        boolean c1 = db.createUser("alice", "Start#Pass1234");
        boolean c2 = db.createUser("bob_01", "Init$Secure5678");
        boolean c3 = db.createUser("charlie_2", "My$trongPassw0rd!");
        boolean c4 = db.createUser("dora", "DoraThe#Expl0rer");
        boolean c5 = db.createUser("eve_user", "S3cure&EveUser!");

        System.out.println("Create users: " + (c1 && c2 && c3 && c4 && c5));

        // Test 1: Successful update and verify
        boolean t1u = db.updatePassword("alice", "NewStrong#Pass1234");
        boolean t1v = db.verify("alice", "NewStrong#Pass1234");
        System.out.println("Test1 update+verify: " + (t1u && t1v));

        // Test 2: Weak password rejected
        boolean t2u = db.updatePassword("alice", "short");
        System.out.println("Test2 weak password rejected: " + (!t2u));

        // Test 3: Non-existing user update
        boolean t3u = db.updatePassword("unknown_user", "Valid#Password123");
        System.out.println("Test3 non-existing user: " + (!t3u));

        // Test 4: Update bob and verify wrong password fails
        boolean t4u = db.updatePassword("bob_01", "Another$trongP4ss!");
        boolean t4v = db.verify("bob_01", "WrongPassword!234");
        System.out.println("Test4 update ok, wrong verify fails: " + (t4u && !t4v));

        // Test 5: Long strong password
        String longPass = "A1!" + "xY9#" .repeat(20) + "Zz@2";
        boolean t5u = db.updatePassword("charlie_2", longPass);
        boolean t5v = db.verify("charlie_2", longPass);
        System.out.println("Test5 long password update+verify: " + (t5u && t5v));
    }
}
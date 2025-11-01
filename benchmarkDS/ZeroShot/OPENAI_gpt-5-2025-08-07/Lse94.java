import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.spec.InvalidKeySpecException;
import java.util.Base64;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;

class Lse94 {

    static class UserRecord {
        final String username;
        final String saltB64;
        final String hashB64;

        UserRecord(String username, String saltB64, String hashB64) {
            this.username = username;
            this.saltB64 = saltB64;
            this.hashB64 = hashB64;
        }
    }

    static class InMemoryDatabase {
        private final Map<String, UserRecord> users = new ConcurrentHashMap<>();

        public boolean insert(UserRecord rec) {
            if (rec == null || rec.username == null) return false;
            return users.putIfAbsent(rec.username, rec) == null;
        }

        public UserRecord get(String username) {
            return users.get(username);
        }
    }

    public static byte[] generateSalt(int size) {
        if (size <= 0) throw new IllegalArgumentException("Salt size must be positive");
        SecureRandom sr;
        try {
            sr = SecureRandom.getInstanceStrong();
        } catch (NoSuchAlgorithmException e) {
            sr = new SecureRandom();
        }
        byte[] salt = new byte[size];
        sr.nextBytes(salt);
        return salt;
    }

    public static String hashPassword(char[] password, byte[] salt, int iterations, int keyLengthBytes)
            throws NoSuchAlgorithmException, InvalidKeySpecException {
        if (password == null || password.length == 0) throw new IllegalArgumentException("Empty password");
        if (salt == null || salt.length == 0) throw new IllegalArgumentException("Empty salt");
        if (iterations < 10000) throw new IllegalArgumentException("Iterations too low");
        if (keyLengthBytes < 16) throw new IllegalArgumentException("Key length too short");

        PBEKeySpec spec = new PBEKeySpec(password, salt, iterations, keyLengthBytes * 8);
        try {
            SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
            byte[] derived = skf.generateSecret(spec).getEncoded();
            return Base64.getEncoder().encodeToString(derived);
        } finally {
            spec.clearPassword();
        }
    }

    public static boolean registerUser(String username, char[] password, InMemoryDatabase db) {
        if (db == null) throw new IllegalArgumentException("db is null");
        if (username == null || username.isBlank()) return false;
        if (password == null || password.length == 0) return false;
        byte[] salt = null;
        try {
            salt = generateSalt(16);
            String saltB64 = Base64.getEncoder().encodeToString(salt);
            String hashB64 = hashPassword(password, salt, 100_000, 32);
            UserRecord rec = new UserRecord(username, saltB64, hashB64);
            return db.insert(rec);
        } catch (Exception e) {
            return false;
        } finally {
            // Zero out password in memory
            java.util.Arrays.fill(password, '\0');
            if (salt != null) java.util.Arrays.fill(salt, (byte) 0);
        }
    }

    public static void main(String[] args) {
        InMemoryDatabase db = new InMemoryDatabase();

        String[] usernames = {"alice", "bob", "charlie", "diana", "eve"};
        String[] passwords = {"S3cure!Pass1", "An0ther$Pass2", "Str0ng#Key3", "My$Passw0rd4", "Ultr@Safe5"};

        for (int i = 0; i < 5; i++) {
            boolean ok = registerUser(usernames[i], passwords[i].toCharArray(), db);
            System.out.println("Insert user " + usernames[i] + ": " + (ok ? "OK" : "FAIL"));
        }

        // Show stored records (no plaintext passwords)
        for (String u : usernames) {
            UserRecord r = db.get(u);
            if (r != null) {
                System.out.println(u + " -> salt=" + r.saltB64 + ", hash=" + r.hashB64);
            }
        }
    }
}
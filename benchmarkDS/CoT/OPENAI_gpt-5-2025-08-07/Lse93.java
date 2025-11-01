import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;
import java.security.SecureRandom;
import java.util.Arrays;
import java.util.Base64;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.atomic.AtomicInteger;

/*
Chain-of-Through in comments:
1) Problem understanding: Implement secure registration: accept username and password, hash password, store username and hashed password (with salt) in a "database".
2) Security requirements: Validate inputs, use strong KDF (PBKDF2WithHmacSHA256 with high iterations and random salt), avoid storing raw passwords, use in-memory map as DB with uniqueness, clear sensitive data from memory when possible.
3) Secure coding generation: Use SecureRandom, parameter validation, constant-safe encodings, avoid printing secrets, clear password char[].
4) Code review: Ensure no plaintext password is stored, salts are random, iteration count high, inputs validated, thread safety for DB operations.
5) Secure code output: Final code below.
*/

public class Lse93 {
    private static final int SALT_LEN = 16;
    private static final int ITERATIONS = 200_000;
    private static final int KEY_LEN_BITS = 256;

    private static final ConcurrentHashMap<String, UserRecord> DB = new ConcurrentHashMap<>();
    private static final AtomicInteger ID_GEN = new AtomicInteger(1);
    private static final SecureRandom RNG = new SecureRandom();

    private static class UserRecord {
        final int id;
        final String username;
        final byte[] salt;
        final byte[] hash;
        UserRecord(int id, String username, byte[] salt, byte[] hash) {
            this.id = id;
            this.username = username;
            this.salt = salt;
            this.hash = hash;
        }
    }

    public static boolean registerUser(String username, char[] password) {
        if (!isValidUsername(username) || !isValidPassword(password)) {
            wipe(password);
            return false;
        }
        byte[] salt = new byte[SALT_LEN];
        RNG.nextBytes(salt);
        byte[] derived = null;
        try {
            PBEKeySpec spec = new PBEKeySpec(password, salt, ITERATIONS, KEY_LEN_BITS);
            SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
            derived = skf.generateSecret(spec).getEncoded();
            spec.clearPassword();
        } catch (Exception e) {
            wipe(password);
            Arrays.fill(salt, (byte)0);
            return false;
        }
        wipe(password);

        UserRecord rec = new UserRecord(ID_GEN.getAndIncrement(), username, salt, derived);
        UserRecord existing = DB.putIfAbsent(username, rec);
        if (existing != null) {
            // Username already exists
            Arrays.fill(salt, (byte)0);
            Arrays.fill(derived, (byte)0);
            return false;
        }
        return true;
    }

    private static boolean isValidUsername(String u) {
        if (u == null) return false;
        if (u.length() < 3 || u.length() > 64) return false;
        for (int i = 0; i < u.length(); i++) {
            char c = u.charAt(i);
            if (!(Character.isLetterOrDigit(c) || c == '_' || c == '.' || c == '-')) {
                return false;
            }
        }
        return true;
    }

    private static boolean isValidPassword(char[] p) {
        if (p == null || p.length < 8 || p.length > 1024) return false;
        boolean hasU=false, hasL=false, hasD=false, hasS=false;
        for (char c : p) {
            if (Character.isUpperCase(c)) hasU = true;
            else if (Character.isLowerCase(c)) hasL = true;
            else if (Character.isDigit(c)) hasD = true;
            else hasS = true;
        }
        return hasU && hasL && hasD && hasS;
    }

    private static void wipe(char[] arr) {
        if (arr != null) Arrays.fill(arr, '\0');
    }

    // Helper for demo only (do not expose in production)
    private static String summarizeUser(String username) {
        UserRecord r = DB.get(username);
        if (r == null) return "not found";
        String saltB64 = Base64.getEncoder().encodeToString(r.salt);
        String hashB64 = Base64.getEncoder().encodeToString(r.hash);
        return "id=" + r.id + ", user=" + r.username + ", salt(b64)=" + saltB64 + ", hash(b64)=" + hashB64;
    }

    public static void main(String[] args) {
        // 5 test cases
        System.out.println("T1 alice: " + registerUser("alice", "StrongP@ssw0rd!".toCharArray()));
        System.out.println("T2 bob (weak): " + registerUser("bob", "weak".toCharArray()));
        System.out.println("T3 alice duplicate: " + registerUser("alice", "AnotherStr0ng!".toCharArray()));
        System.out.println("T4 charlie: " + registerUser("charlie", "Cmplx#Pass123".toCharArray()));
        System.out.println("T5 bad username: " + registerUser("bad user", "G00d#Pass!".toCharArray()));

        // Print summaries to verify storage (no plaintext)
        System.out.println(summarizeUser("alice"));
        System.out.println(summarizeUser("bob"));
        System.out.println(summarizeUser("charlie"));
        System.out.println(summarizeUser("bad user"));
    }
}
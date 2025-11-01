import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.SecureRandom;
import java.util.Arrays;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;
import java.util.regex.Pattern;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;

public class Lse55 {

    // User record
    static final class User {
        final byte[] salt;
        final byte[] hash;
        final int iterations;
        final String role;

        User(byte[] salt, byte[] hash, int iterations, String role) {
            this.salt = Arrays.copyOf(salt, salt.length);
            this.hash = Arrays.copyOf(hash, hash.length);
            this.iterations = iterations;
            this.role = role;
        }
    }

    // In-memory stores
    private static final Map<String, User> USERS = new HashMap<>();
    private static final Map<String, String> SESSIONS = new HashMap<>();

    // Security constants
    private static final int ITERATIONS = 210000;
    private static final int SALT_LEN = 16;
    private static final int KEY_LEN = 32; // 256-bit
    private static final SecureRandom RNG = new SecureRandom();

    // Validation patterns
    private static final Pattern USERNAME_RE = Pattern.compile("^[A-Za-z0-9_.-]{3,64}$");
    private static final Pattern UPPER = Pattern.compile(".*[A-Z].*");
    private static final Pattern LOWER = Pattern.compile(".*[a-z].*");
    private static final Pattern DIGIT = Pattern.compile(".*[0-9].*");
    private static final Pattern SYMBOL = Pattern.compile(".*[^A-Za-z0-9].*");

    // Public functions simulating endpoints
    public static String login(String requestId) {
        // GET /login
        String req = (requestId == null) ? "" : requestId;
        if (req.length() > 128) {
            return "400 Bad Request";
        }
        return "Login endpoint (GET). Please POST credentials to do_login.";
    }

    public static String do_login(String username, char[] password) {
        // POST /do_login
        try {
            if (!isValidUsername(username) || password == null || password.length == 0) {
                return "";
            }
            User u = USERS.get(username);
            if (u == null) {
                return "";
            }
            byte[] computed = pbkdf2(password, u.salt, u.iterations, KEY_LEN);
            boolean ok = MessageDigest.isEqual(computed, u.hash);
            Arrays.fill(computed, (byte) 0);
            if (!ok) {
                return "";
            }
            String token = generateToken(32);
            SESSIONS.put(token, username);
            return token;
        } catch (Exception e) {
            return "";
        } finally {
            if (password != null) {
                Arrays.fill(password, '\0');
            }
        }
    }

    public static String index(String sessionToken) {
        // GET /
        String user = SESSIONS.get(sessionToken == null ? "" : sessionToken);
        if (user == null) {
            return "401 Unauthorized";
        }
        User u = USERS.get(user);
        if (u == null) {
            return "401 Unauthorized";
        }
        return "Welcome, " + safe(user) + " (role=" + safe(u.role) + ")";
    }

    public static String user_page(String sessionToken) {
        // GET /user
        String user = SESSIONS.get(sessionToken == null ? "" : sessionToken);
        if (user == null) {
            return "401 Unauthorized";
        }
        return "User page for " + safe(user);
    }

    public static String admin_page(String sessionToken) {
        // GET /admin
        String user = SESSIONS.get(sessionToken == null ? "" : sessionToken);
        if (user == null) {
            return "401 Unauthorized";
        }
        User u = USERS.get(user);
        if (u == null) {
            return "401 Unauthorized";
        }
        if (!"admin".equals(u.role)) {
            return "403 Forbidden";
        }
        return "Admin page for " + safe(user);
    }

    // Registration (internal helper for tests)
    private static boolean registerUser(String username, char[] password, String role) {
        if (!isValidUsername(username) || !isValidRole(role) || password == null) {
            if (password != null) Arrays.fill(password, '\0');
            return false;
        }
        if (!passwordPolicy(password)) {
            Arrays.fill(password, '\0');
            return false;
        }
        try {
            byte[] salt = new byte[SALT_LEN];
            RNG.nextBytes(salt);
            byte[] hash = pbkdf2(password, salt, ITERATIONS, KEY_LEN);
            USERS.put(username, new User(salt, hash, ITERATIONS, role));
            return true;
        } catch (Exception e) {
            return false;
        } finally {
            Arrays.fill(password, '\0');
        }
    }

    // Helpers
    private static boolean isValidUsername(String u) {
        if (u == null) return false;
        if (u.length() < 3 || u.length() > 64) return false;
        return USERNAME_RE.matcher(u).matches();
    }

    private static boolean isValidRole(String r) {
        return "user".equals(r) || "admin".equals(r);
    }

    private static boolean passwordPolicy(char[] pw) {
        if (pw == null) return false;
        if (pw.length < 12 || pw.length > 128) return false;
        String s = new String(pw);
        return UPPER.matcher(s).matches()
                && LOWER.matcher(s).matches()
                && DIGIT.matcher(s).matches()
                && SYMBOL.matcher(s).matches();
    }

    private static String generateToken(int bytes) {
        byte[] buf = new byte[bytes];
        RNG.nextBytes(buf);
        StringBuilder sb = new StringBuilder(buf.length * 2);
        for (byte b : buf) sb.append(String.format("%02x", b));
        Arrays.fill(buf, (byte) 0);
        return sb.toString();
    }

    private static byte[] pbkdf2(char[] password, byte[] salt, int iterations, int keyLen) throws Exception {
        PBEKeySpec spec = new PBEKeySpec(password, salt, iterations, keyLen * 8);
        SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
        byte[] key = skf.generateSecret(spec).getEncoded();
        spec.clearPassword();
        return key;
    }

    private static String safe(String s) {
        if (s == null) return "";
        // Basic output sanitization
        return s.replaceAll("[^A-Za-z0-9_.\\-@ ]", "?");
    }

    // Test harness
    public static void main(String[] args) {
        System.out.println("== GET /login ==");
        System.out.println(login("req-1"));

        // Generate strong passwords for tests
        char[] pwUserA = genStrongPassword(16);
        char[] pwAdminA = genStrongPassword(18);
        char[] pwUserB = genStrongPassword(20);

        // Test 1: Register userA and login successfully
        System.out.println("\n== Test 1: userA successful login ==");
        registerUser("userA", Arrays.copyOf(pwUserA, pwUserA.length), "user");
        String tok1 = do_login("userA", Arrays.copyOf(pwUserA, pwUserA.length));
        System.out.println("Token: " + (tok1.isEmpty() ? "(login failed)" : tok1));
        System.out.println("Index: " + index(tok1));
        System.out.println("User page: " + user_page(tok1));
        System.out.println("Admin page: " + admin_page(tok1)); // should be forbidden

        // Test 2: Wrong password
        System.out.println("\n== Test 2: wrong password ==");
        char[] wrong = "WrongPass123!".toCharArray();
        String tok2 = do_login("userA", wrong);
        System.out.println("Token: " + (tok2.isEmpty() ? "(login failed)" : tok2));

        // Test 3: Admin registration and login
        System.out.println("\n== Test 3: adminA successful admin access ==");
        registerUser("adminA", Arrays.copyOf(pwAdminA, pwAdminA.length), "admin");
        String tok3 = do_login("adminA", Arrays.copyOf(pwAdminA, pwAdminA.length));
        System.out.println("Token: " + (tok3.isEmpty() ? "(login failed)" : tok3));
        System.out.println("Admin page: " + admin_page(tok3));

        // Test 4: Invalid token
        System.out.println("\n== Test 4: invalid token access ==");
        String invalidTok = "deadbeef";
        System.out.println("Index: " + index(invalidTok));
        System.out.println("Admin page: " + admin_page(invalidTok));

        // Test 5: Another user
        System.out.println("\n== Test 5: userB normal access, admin denied ==");
        registerUser("userB", Arrays.copyOf(pwUserB, pwUserB.length), "user");
        String tok5 = do_login("userB", Arrays.copyOf(pwUserB, pwUserB.length));
        System.out.println("Token: " + (tok5.isEmpty() ? "(login failed)" : tok5));
        System.out.println("Index: " + index(tok5));
        System.out.println("Admin page: " + admin_page(tok5));

        // Cleanup secrets
        Arrays.fill(pwUserA, '\0');
        Arrays.fill(pwAdminA, '\0');
        Arrays.fill(pwUserB, '\0');
    }

    private static char[] genStrongPassword(int length) {
        if (length < 12) length = 12;
        final String U = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        final String L = "abcdefghijklmnopqrstuvwxyz";
        final String D = "0123456789";
        final String S = "!@#$%^&*()-_=+[]{}:,.?/";
        final String ALL = U + L + D + S;

        char[] pw = new char[length];
        pw[0] = U.charAt(RNG.nextInt(U.length()));
        pw[1] = L.charAt(RNG.nextInt(L.length()));
        pw[2] = D.charAt(RNG.nextInt(D.length()));
        pw[3] = S.charAt(RNG.nextInt(S.length()));
        for (int i = 4; i < length; i++) {
            pw[i] = ALL.charAt(RNG.nextInt(ALL.length()));
        }
        // Shuffle
        for (int i = pw.length - 1; i > 0; i--) {
            int j = RNG.nextInt(i + 1);
            char tmp = pw[i];
            pw[i] = pw[j];
            pw[j] = tmp;
        }
        return pw;
    }
}
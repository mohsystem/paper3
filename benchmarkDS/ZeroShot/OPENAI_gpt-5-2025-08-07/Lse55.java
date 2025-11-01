import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.spec.KeySpec;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;
import java.util.regex.Pattern;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;

public class Lse55 {
    private static final Pattern USERNAME_PATTERN = Pattern.compile("^[A-Za-z0-9_]{1,32}$");
    private static final int PBKDF2_ITERATIONS = 100_000;
    private static final int KEY_LENGTH_BITS = 256;

    static class Session {
        boolean authenticated = false;
        String username = null;
        String role = null;
        String csrfToken = null;
    }

    static class UserRecord {
        final String username;
        final String role;
        final byte[] salt;
        final byte[] hash;

        UserRecord(String username, String role, byte[] salt, byte[] hash) {
            this.username = username;
            this.role = role;
            this.salt = salt.clone();
            this.hash = hash.clone();
        }
    }

    private static final Map<String, UserRecord> USERS = new HashMap<>();

    private static byte[] pbkdf2(char[] password, byte[] salt) {
        try {
            KeySpec spec = new PBEKeySpec(password, salt, PBKDF2_ITERATIONS, KEY_LENGTH_BITS);
            SecretKeyFactory f = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
            return f.generateSecret(spec).getEncoded();
        } catch (Exception e) {
            throw new IllegalStateException("KDF error", e);
        }
    }

    private static void addUser(String username, String plainPassword, String role, String saltText) {
        byte[] salt = saltText.getBytes(java.nio.charset.StandardCharsets.UTF_8);
        byte[] hash = pbkdf2(plainPassword.toCharArray(), salt);
        USERS.put(username, new UserRecord(username, role, salt, hash));
    }

    private static boolean constantTimeEquals(byte[] a, byte[] b) {
        if (a == null || b == null) return false;
        if (a.length != b.length) return false;
        int r = 0;
        for (int i = 0; i < a.length; i++) {
            r |= (a[i] ^ b[i]);
        }
        return r == 0;
    }

    private static String generateCsrfToken() {
        byte[] buf = new byte[32];
        new SecureRandom().nextBytes(buf);
        return Base64.getUrlEncoder().withoutPadding().encodeToString(buf);
    }

    public static String login(Session session, String method) {
        if (session == null) return "400 Bad Request";
        if (!"GET".equals(method)) return "405 Method Not Allowed";
        session.csrfToken = generateCsrfToken();
        return "Login Page (GET): Provide username, password, and CSRF token. CSRF=" + session.csrfToken;
    }

    public static String do_login(Session session, String method, String username, String password, String csrfToken) {
        if (session == null) return "400 Bad Request";
        if (!"POST".equals(method)) return "405 Method Not Allowed";
        if (csrfToken == null || session.csrfToken == null || !constantTimeEquals(csrfToken.getBytes(), session.csrfToken.getBytes())) {
            return "403 Forbidden: Invalid CSRF token";
        }
        if (username == null || password == null) return "400 Bad Request";
        if (!USERNAME_PATTERN.matcher(username).matches()) return "400 Bad Request: Invalid username format";

        UserRecord rec = USERS.get(username);
        if (rec == null) {
            return "401 Unauthorized: Invalid credentials";
        }
        byte[] computed = pbkdf2(password.toCharArray(), rec.salt);
        if (!constantTimeEquals(computed, rec.hash)) {
            return "401 Unauthorized: Invalid credentials";
        }
        session.authenticated = true;
        session.username = username;
        session.role = rec.role;
        session.csrfToken = generateCsrfToken(); // rotate token after login
        return "200 OK: Login successful for " + username;
    }

    public static String index(Session session, String method) {
        if (session == null) return "400 Bad Request";
        if (!"GET".equals(method)) return "405 Method Not Allowed";
        if (session.authenticated) {
            return "Index (GET): Welcome, " + session.username + "!";
        }
        return "Index (GET): Welcome, guest. Please login.";
    }

    public static String user_page(Session session, String method) {
        if (session == null) return "400 Bad Request";
        if (!"GET".equals(method)) return "405 Method Not Allowed";
        if (!session.authenticated) return "401 Unauthorized: Please login";
        if (!"user".equals(session.role) && !"admin".equals(session.role)) return "403 Forbidden";
        return "User Page (GET): Hello " + session.username + " (role=" + session.role + ")";
    }

    public static String admin_page(Session session, String method) {
        if (session == null) return "400 Bad Request";
        if (!"GET".equals(method)) return "405 Method Not Allowed";
        if (!session.authenticated) return "401 Unauthorized: Please login";
        if (!"admin".equals(session.role)) return "403 Forbidden: Admins only";
        return "Admin Page (GET): Welcome admin " + session.username;
    }

    public static void main(String[] args) throws NoSuchAlgorithmException {
        // Initialize users securely (hashing with PBKDF2)
        addUser("alice", "password123!", "user", "NaClAlice");
        addUser("admin", "Admin#2025", "admin", "NaClAdmin");

        // Test case 1: GET login page
        Session s1 = new Session();
        System.out.println("TC1-login: " + login(s1, "GET"));

        // Test case 2: POST login with wrong CSRF
        Session s2 = new Session();
        System.out.println("TC2-login: " + login(s2, "GET"));
        System.out.println("TC2-do_login: " + do_login(s2, "POST", "alice", "password123!", "badtoken"));
        System.out.println("TC2-index: " + index(s2, "GET"));

        // Test case 3: Wrong password
        Session s3 = new Session();
        System.out.println("TC3-login: " + login(s3, "GET"));
        System.out.println("TC3-do_login: " + do_login(s3, "POST", "alice", "wrongpass", s3.csrfToken));
        System.out.println("TC3-index: " + index(s3, "GET"));

        // Test case 4: User login success then attempt admin page
        Session s4 = new Session();
        System.out.println("TC4-login: " + login(s4, "GET"));
        System.out.println("TC4-do_login: " + do_login(s4, "POST", "alice", "password123!", s4.csrfToken));
        System.out.println("TC4-user_page: " + user_page(s4, "GET"));
        System.out.println("TC4-admin_page: " + admin_page(s4, "GET"));

        // Test case 5: Admin login success and access admin page
        Session s5 = new Session();
        System.out.println("TC5-login: " + login(s5, "GET"));
        System.out.println("TC5-do_login: " + do_login(s5, "POST", "admin", "Admin#2025", s5.csrfToken));
        System.out.println("TC5-index: " + index(s5, "GET"));
        System.out.println("TC5-admin_page: " + admin_page(s5, "GET"));
    }
}
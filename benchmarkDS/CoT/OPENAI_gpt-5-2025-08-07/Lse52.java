import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.Base64;
import java.nio.charset.StandardCharsets;
import java.util.concurrent.ConcurrentHashMap;
import java.util.Map;

public class Lse52 {

    // Simple in-memory session store
    private static final Map<String, Session> sessions = new ConcurrentHashMap<>();

    // Simple in-memory user store: username -> salted SHA-256 hash
    private static final Map<String, String> userStore = new ConcurrentHashMap<>();

    private static final String SALT = "E8D4C9B4-Strong-Static-Salt-For-Demo-Only";

    private static class Session {
        String username; // null when not authenticated
        String csrfToken;
    }

    // Initialize demo credentials securely hashed
    static {
        userStore.put("admin", hashPassword("admin", "s3cureP@ss!"));
    }

    // 1) Index page
    public static String index() {
        return "<html><body>" +
               "<h1>Welcome</h1>" +
               "<a href=\"/login\">Login</a>" +
               "</body></html>";
    }

    // 2) Create/init a session and CSRF token
    public static String initSession() {
        String sessionId = genToken(32);
        Session s = new Session();
        s.username = null;
        s.csrfToken = genToken(32);
        sessions.put(sessionId, s);
        return sessionId;
    }

    // 3) Login page with CSRF token
    public static String loginPage(String sessionId) {
        Session s = sessions.get(sessionId);
        if (s == null) {
            return "ERROR: Invalid session";
        }
        String csrf = s.csrfToken;
        return "<html><body>" +
               "<h1>Login</h1>" +
               "<form method=\"POST\" action=\"/do_login\">" +
               "<input type=\"hidden\" name=\"csrf\" value=\"" + escapeHtml(csrf) + "\"/>" +
               "<label>Username: <input type=\"text\" name=\"username\"/></label><br/>" +
               "<label>Password: <input type=\"password\" name=\"password\"/></label><br/>" +
               "<button type=\"submit\">Login</button>" +
               "</form>" +
               "</body></html>";
    }

    // 4) do_login action
    public static String doLogin(String sessionId, String username, String password, String csrfSubmitted) {
        Session s = sessions.get(sessionId);
        if (s == null) {
            return "ERROR: Invalid session";
        }
        if (csrfSubmitted == null || !safeEquals(csrfSubmitted, s.csrfToken)) {
            return "ERROR: CSRF validation failed";
        }
        if (username == null || password == null) {
            return "ERROR: Missing credentials";
        }
        String storedHash = userStore.get(username);
        String providedHash = hashPassword(username, password);
        if (storedHash != null && safeEquals(storedHash, providedHash)) {
            s.username = username;
            // rotate CSRF token after login
            s.csrfToken = genToken(32);
            return "REDIRECT:/secret";
        } else {
            return "ERROR: Invalid username or password";
        }
    }

    // 5) Secret page
    public static String secretPage(String sessionId) {
        Session s = sessions.get(sessionId);
        if (s == null) {
            return "ERROR: Invalid session";
        }
        if (s.username == null) {
            return "REDIRECT:/login";
        }
        return "<html><body><h1>Secret settings</h1></body></html>";
    }

    // Utility: secure random token (base64url without padding)
    private static String genToken(int bytes) {
        try {
            SecureRandom sr = SecureRandom.getInstanceStrong();
            byte[] buf = new byte[bytes];
            sr.nextBytes(buf);
            return Base64.getUrlEncoder().withoutPadding().encodeToString(buf);
        } catch (Exception e) {
            SecureRandom sr = new SecureRandom();
            byte[] buf = new byte[bytes];
            sr.nextBytes(buf);
            return Base64.getUrlEncoder().withoutPadding().encodeToString(buf);
        }
    }

    // Utility: hash password with SHA-256(salt || username || ":" || password)
    private static String hashPassword(String username, String password) {
        try {
            MessageDigest md = MessageDigest.getInstance("SHA-256");
            md.update(SALT.getBytes(StandardCharsets.UTF_8));
            md.update(username.getBytes(StandardCharsets.UTF_8));
            md.update((byte) ':');
            md.update(password.getBytes(StandardCharsets.UTF_8));
            byte[] digest = md.digest();
            StringBuilder sb = new StringBuilder(digest.length * 2);
            for (byte b : digest) {
                sb.append(String.format("%02x", b));
            }
            return sb.toString();
        } catch (NoSuchAlgorithmException e) {
            throw new RuntimeException("SHA-256 not available", e);
        }
    }

    // Utility: constant-time string equality
    private static boolean safeEquals(String a, String b) {
        if (a == null || b == null) return false;
        byte[] x = a.getBytes(StandardCharsets.UTF_8);
        byte[] y = b.getBytes(StandardCharsets.UTF_8);
        if (x.length != y.length) return false;
        int diff = 0;
        for (int i = 0; i < x.length; i++) {
            diff |= (x[i] ^ y[i]);
        }
        return diff == 0;
    }

    // Utility: minimal HTML escaping for attribute contexts
    private static String escapeHtml(String s) {
        StringBuilder out = new StringBuilder();
        for (char c : s.toCharArray()) {
            switch (c) {
                case '&': out.append("&amp;"); break;
                case '<': out.append("&lt;"); break;
                case '>': out.append("&gt;"); break;
                case '"': out.append("&quot;"); break;
                case '\'': out.append("&#x27;"); break;
                default: out.append(c);
            }
        }
        return out.toString();
    }

    // Main with 5 test cases
    public static void main(String[] args) {
        // Test 1: View index page
        System.out.println("TEST1 Index:");
        System.out.println(index());

        // Prepare a session
        String sid = initSession();

        // Test 2: View login page (includes CSRF)
        System.out.println("\nTEST2 Login Page:");
        System.out.println(loginPage(sid));
        String csrf = sessions.get(sid).csrfToken; // for testing, we fetch from session

        // Test 3: Attempt login with wrong password
        System.out.println("\nTEST3 Wrong Password:");
        System.out.println(doLogin(sid, "admin", "wrong", csrf));

        // Test 4: Attempt login with wrong CSRF
        System.out.println("\nTEST4 Wrong CSRF:");
        System.out.println(doLogin(sid, "admin", "s3cureP@ss!", "badtoken"));

        // Test 5: Successful login then access secret
        System.out.println("\nTEST5 Successful Login and Secret:");
        System.out.println(doLogin(sid, "admin", "s3cureP@ss!", csrf));
        System.out.println(secretPage(sid));
    }
}
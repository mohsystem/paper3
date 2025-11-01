import java.security.SecureRandom;
import java.util.Base64;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;

public class Lse55 {

    // Step 1: Problem understanding - Simulate HTTP-like functions with RBAC and sessions.
    // Step 2: Security requirements - Use secure token generation, constant-time comparison, avoid leaking details.
    // Step 3: Secure coding generation - Implement functions with method checks, role checks, token expiry, and safe comparisons.
    // Step 4: Code review - Ensure no sensitive logging, consistent method validation, null-safe handling.
    // Step 5: Secure code output - Finalized secure sample code.

    private static final Map<String, String> USER_PASSWORDS;
    private static final Map<String, String> USER_ROLES;
    private static final Map<String, Session> SESSIONS = Collections.synchronizedMap(new HashMap<>());
    private static final long SESSION_TTL_MS = 15 * 60 * 1000; // 15 minutes
    private static final SecureRandom SECURE_RANDOM = new SecureRandom();
    // Dummy password to avoid user enumeration timing differences
    private static final String DUMMY_PASSWORD = "dummy_password_fallback_value";

    static {
        Map<String, String> up = new HashMap<>();
        up.put("user", "userpass");
        up.put("admin", "adminpass");
        USER_PASSWORDS = Collections.unmodifiableMap(up);

        Map<String, String> ur = new HashMap<>();
        ur.put("user", "user");
        ur.put("admin", "admin");
        USER_ROLES = Collections.unmodifiableMap(ur);
    }

    private static class Session {
        final String username;
        final String role;
        final long expiry;

        Session(String username, String role, long expiry) {
            this.username = username;
            this.role = role;
            this.expiry = expiry;
        }

        boolean isExpired() {
            return System.currentTimeMillis() > expiry;
        }
    }

    private static String generateToken() {
        byte[] bytes = new byte[32];
        SECURE_RANDOM.nextBytes(bytes);
        return Base64.getUrlEncoder().withoutPadding().encodeToString(bytes);
    }

    private static boolean constantTimeEquals(String a, String b) {
        if (a == null || b == null) return false;
        byte[] x = a.getBytes(java.nio.charset.StandardCharsets.UTF_8);
        byte[] y = b.getBytes(java.nio.charset.StandardCharsets.UTF_8);
        int len = Math.max(x.length, y.length);
        int diff = x.length ^ y.length;
        for (int i = 0; i < len; i++) {
            byte xb = i < x.length ? x[i] : 0;
            byte yb = i < y.length ? y[i] : 0;
            diff |= xb ^ yb;
        }
        return diff == 0;
    }

    private static Session getValidSession(String token) {
        if (token == null) return null;
        Session s = SESSIONS.get(token);
        if (s == null) return null;
        if (s.isExpired()) {
            SESSIONS.remove(token);
            return null;
        }
        return s;
    }

    // The first function: login (GET)
    public static String login(String method) {
        if (!"GET".equals(method)) {
            return "405 Method Not Allowed";
        }
        return "Login Page: Please provide username and password via do_login (POST).";
    }

    // The second function: do_login (POST) returns a session token or error message
    public static String do_login(String method, String username, String password) {
        if (!"POST".equals(method)) {
            return null;
        }
        // Prevent null inputs
        if (username == null || password == null) {
            // Always perform a comparison against dummy to keep timing consistent
            constantTimeEquals(password == null ? "" : password, DUMMY_PASSWORD);
            return null;
        }
        String stored = USER_PASSWORDS.get(username);
        String toCompare = (stored != null) ? stored : DUMMY_PASSWORD;

        boolean ok = constantTimeEquals(password, toCompare) && stored != null;
        if (!ok) {
            return null;
        }
        String role = USER_ROLES.getOrDefault(username, "user");
        String token = generateToken();
        long expiry = System.currentTimeMillis() + SESSION_TTL_MS;
        SESSIONS.put(token, new Session(username, role, expiry));
        return token;
    }

    // The third function: index (GET)
    public static String index(String method, String sessionToken) {
        if (!"GET".equals(method)) {
            return "405 Method Not Allowed";
        }
        Session s = getValidSession(sessionToken);
        if (s == null) {
            return "Welcome, guest. Please login.";
        }
        return "Welcome, " + s.username + "!";
    }

    // The fourth function: user_page (GET)
    public static String user_page(String method, String sessionToken) {
        if (!"GET".equals(method)) {
            return "405 Method Not Allowed";
        }
        Session s = getValidSession(sessionToken);
        if (s == null) {
            return "401 Unauthorized";
        }
        return "User Page for " + s.username;
    }

    // The fifth function: admin_page (GET)
    public static String admin_page(String method, String sessionToken) {
        if (!"GET".equals(method)) {
            return "405 Method Not Allowed";
        }
        Session s = getValidSession(sessionToken);
        if (s == null) {
            return "401 Unauthorized";
        }
        if (!"admin".equals(s.role)) {
            return "403 Forbidden";
        }
        return "Admin Page for " + s.username;
    }

    public static void main(String[] args) {
        // 5 test cases
        System.out.println("Test1: " + login("GET"));
        String userToken = do_login("POST", "user", "userpass");
        System.out.println("Test2: user login token: " + (userToken != null));
        System.out.println("Test3: user page with user token: " + user_page("GET", userToken));
        System.out.println("Test4: admin page with user token: " + admin_page("GET", userToken));
        String adminToken = do_login("POST", "admin", "adminpass");
        System.out.println("Test5: admin page with admin token: " + admin_page("GET", adminToken));
    }
}
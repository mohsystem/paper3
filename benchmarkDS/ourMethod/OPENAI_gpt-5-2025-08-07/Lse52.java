import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.SecureRandom;
import java.util.*;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;

public class Lse52 {

    public static class Credential {
        public final byte[] salt;
        public final byte[] hash;
        public final int iterations;

        public Credential(byte[] salt, byte[] hash, int iterations) {
            this.salt = Arrays.copyOf(salt, salt.length);
            this.hash = Arrays.copyOf(hash, hash.length);
            this.iterations = iterations;
        }
    }

    // Returns a simple index page with a link to login
    public static String indexPage() {
        return "<!doctype html><html><head><meta charset=\"utf-8\"></head><body>" +
               "<h1>Welcome</h1><a href=\"/login\">Login</a></body></html>";
    }

    // Returns a simple login form page
    public static String loginPage() {
        return "<!doctype html><html><head><meta charset=\"utf-8\"></head><body>" +
               "<h1>Login</h1>" +
               "<form action=\"/do_login\" method=\"post\">" +
               "Username: <input type=\"text\" name=\"username\" maxlength=\"32\" /><br/>" +
               "Password: <input type=\"password\" name=\"password\" maxlength=\"128\" /><br/>" +
               "<button type=\"submit\">Login</button>" +
               "</form></body></html>";
    }

    // Secret page: only accessible if session has "username"
    public static String secretPage(Map<String, String> session) {
        if (session != null && session.containsKey("username")) {
            return "<!doctype html><html><head><meta charset=\"utf-8\"></head><body>Secret settings</body></html>";
        }
        return "<!doctype html><html><head><meta charset=\"utf-8\"></head><body>Unauthorized</body></html>";
    }

    // Validate username: 3..32 chars, [A-Za-z0-9_-]
    public static boolean isValidUsername(String username) {
        if (username == null) return false;
        if (username.length() < 3 || username.length() > 32) return false;
        for (int i = 0; i < username.length(); i++) {
            char c = username.charAt(i);
            boolean ok = (c >= 'A' && c <= 'Z') ||
                         (c >= 'a' && c <= 'z') ||
                         (c >= '0' && c <= '9') ||
                         c == '_' || c == '-';
            if (!ok) return false;
        }
        return true;
    }

    // Validate password length
    public static boolean isValidPassword(String password) {
        if (password == null) return false;
        int len = password.length();
        return len >= 12 && len <= 128;
    }

    // PBKDF2-HMAC-SHA256
    public static byte[] pbkdf2(char[] password, byte[] salt, int iterations, int dkLen) throws Exception {
        PBEKeySpec spec = new PBEKeySpec(password, salt, iterations, dkLen * 8);
        SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
        return skf.generateSecret(spec).getEncoded();
    }

    // Constant-time comparison
    public static boolean constantTimeEquals(byte[] a, byte[] b) {
        return MessageDigest.isEqual(a, b);
    }

    // Generate a secure random salt
    public static byte[] genSalt(int size) {
        byte[] salt = new byte[size];
        new SecureRandom().nextBytes(salt);
        return salt;
    }

    // Create credentials for a given password
    public static Credential createCredential(String password) throws Exception {
        int iterations = 210000;
        byte[] salt = genSalt(16);
        byte[] hash = pbkdf2(password.toCharArray(), salt, iterations, 32);
        return new Credential(salt, hash, iterations);
    }

    // Verify password
    public static boolean verifyPassword(String password, Credential cred) {
        if (password == null || cred == null) return false;
        try {
            byte[] test = pbkdf2(password.toCharArray(), cred.salt, cred.iterations, cred.hash.length);
            return constantTimeEquals(test, cred.hash);
        } catch (Exception e) {
            return false;
        }
    }

    // do_login: sets session "username" and returns redirect on success; else error message
    public static String doLogin(Map<String, String> session,
                                 String username,
                                 String password,
                                 Map<String, Credential> users) {
        if (session == null || users == null) return "ERROR:Invalid session/store";
        if (!isValidUsername(username)) return "ERROR:Invalid username";
        if (!isValidPassword(password)) return "ERROR:Invalid password";
        Credential cred = users.get(username);
        if (cred == null) return "ERROR:Invalid credentials";
        if (!verifyPassword(password, cred)) return "ERROR:Invalid credentials";
        session.put("username", username);
        return "REDIRECT:/secret";
    }

    // Test utility: random password of given length, with mixed chars
    public static String randomPassword(int length) {
        final String chars = "ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz23456789!@#$%^&*()-_=+";
        SecureRandom rng = new SecureRandom();
        StringBuilder sb = new StringBuilder(length);
        for (int i = 0; i < length; i++) {
            sb.append(chars.charAt(rng.nextInt(chars.length())));
        }
        return sb.toString();
    }

    // Simple tests
    public static void main(String[] args) throws Exception {
        Map<String, Credential> users = new HashMap<>();
        String u1 = "alice";
        String pw1 = randomPassword(14); // Not logged or printed
        users.put(u1, createCredential(pw1));

        Map<String, String> session = new HashMap<>();

        int passed = 0;
        int total = 5;

        // Test 1: Successful login and secret page access
        String r1 = doLogin(session, u1, pw1, users);
        boolean t1 = "REDIRECT:/secret".equals(r1) && "alice".equals(session.get("username")) &&
                     secretPage(session).contains("Secret settings");
        if (t1) passed++;

        // Reset session for next tests
        session.clear();

        // Test 2: Wrong password
        String r2 = doLogin(session, u1, "WrongPassword123!", users);
        boolean t2 = r2.startsWith("ERROR:") && !session.containsKey("username");
        if (t2) passed++;

        // Test 3: Wrong username
        String r3 = doLogin(session, "bob", pw1, users);
        boolean t3 = r3.startsWith("ERROR:") && !session.containsKey("username");
        if (t3) passed++;

        // Test 4: Invalid username format
        String r4 = doLogin(session, "bad<user>", pw1, users);
        boolean t4 = r4.startsWith("ERROR:Invalid username");
        if (t4) passed++;

        // Test 5: Password too short
        String r5 = doLogin(session, u1, "shortPwd1!", users);
        boolean t5 = r5.startsWith("ERROR:Invalid password");
        if (t5) passed++;

        System.out.println("Index page sample: " + (indexPage().contains("Login") ? "OK" : "FAIL"));
        System.out.println("Login page sample: " + (loginPage().contains("form") ? "OK" : "FAIL"));
        System.out.println("Tests passed: " + passed + "/" + total);
    }
}
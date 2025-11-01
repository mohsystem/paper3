import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.SecureRandom;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;
import java.util.regex.Pattern;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;

public class Lse35 {

    private static final int SALT_LEN = 16;
    private static final int HASH_LEN = 32; // 256-bit
    private static final int ITERATIONS = 210000;
    private static final int MAX_USERNAME_LEN = 50;
    private static final int MAX_PASSWORD_LEN = 128;
    private static final Pattern USERNAME_PATTERN = Pattern.compile("^[A-Za-z0-9_.-]{3,50}$");

    private static final String LOGIN_HTML =
        "<!DOCTYPE html>\n" +
        "<html lang=\"en\">\n" +
        "<head><meta charset=\"UTF-8\"><title>Login</title></head>\n" +
        "<body>\n" +
        "  <h1>Login</h1>\n" +
        "  <form action=\"/login\" method=\"POST\">\n" +
        "    <label>Username: <input type=\"text\" name=\"username\" maxlength=\"50\" required></label><br>\n" +
        "    <label>Password: <input type=\"password\" name=\"password\" maxlength=\"128\" required></label><br>\n" +
        "    <button type=\"submit\">Login</button>\n" +
        "  </form>\n" +
        "</body>\n" +
        "</html>";

    private static final class UserRecord {
        final byte[] salt;
        final byte[] hash;
        UserRecord(byte[] salt, byte[] hash) {
            this.salt = salt;
            this.hash = hash;
        }
    }

    private static final Map<String, UserRecord> DATABASE = new HashMap<>();

    private static SecureRandom secureRandom() {
        try {
            return SecureRandom.getInstanceStrong();
        } catch (Exception e) {
            return new SecureRandom();
        }
    }

    private static byte[] pbkdf2(char[] password, byte[] salt) throws Exception {
        PBEKeySpec spec = new PBEKeySpec(password, salt, ITERATIONS, HASH_LEN * 8);
        SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
        return skf.generateSecret(spec).getEncoded();
    }

    private static boolean isValidUsername(String username) {
        if (username == null) return false;
        if (username.length() < 3 || username.length() > MAX_USERNAME_LEN) return false;
        return USERNAME_PATTERN.matcher(username).matches();
    }

    private static boolean isStrongPassword(String password) {
        if (password == null) return false;
        if (password.length() < 12 || password.length() > MAX_PASSWORD_LEN) return false;
        boolean hasUpper = false, hasLower = false, hasDigit = false, hasSpecial = false;
        for (int i = 0; i < password.length(); i++) {
            char c = password.charAt(i);
            if (Character.isUpperCase(c)) hasUpper = true;
            else if (Character.isLowerCase(c)) hasLower = true;
            else if (Character.isDigit(c)) hasDigit = true;
            else hasSpecial = true;
            if (Character.isISOControl(c)) return false;
        }
        return hasUpper && hasLower && hasDigit && hasSpecial;
    }

    public static void createUser(String username, String password) throws Exception {
        if (!isValidUsername(username)) {
            throw new IllegalArgumentException("Invalid username");
        }
        if (!isStrongPassword(password)) {
            throw new IllegalArgumentException("Weak password");
        }
        if (DATABASE.containsKey(username)) {
            throw new IllegalArgumentException("User exists");
        }
        byte[] salt = new byte[SALT_LEN];
        secureRandom().nextBytes(salt);
        byte[] hash = pbkdf2(password.toCharArray(), salt);
        DATABASE.put(username, new UserRecord(salt, hash));
    }

    public static String login(String username, String password) {
        // Validate and sanitize inputs
        if (username == null || password == null) return "login.html";
        if (username.length() < 3 || username.length() > MAX_USERNAME_LEN) return "login.html";
        if (password.length() < 1 || password.length() > MAX_PASSWORD_LEN) return "login.html";
        if (!USERNAME_PATTERN.matcher(username).matches()) return "login.html";

        UserRecord rec = DATABASE.get(username);
        if (rec == null) return "login.html";
        try {
            byte[] computed = pbkdf2(password.toCharArray(), rec.salt);
            boolean ok = MessageDigest.isEqual(computed, rec.hash);
            // zeroize computed
            for (int i = 0; i < computed.length; i++) computed[i] = 0;
            return ok ? "home.html" : "login.html";
        } catch (Exception e) {
            return "login.html";
        }
    }

    public static String getLoginHtml() {
        return LOGIN_HTML;
    }

    public static void main(String[] args) throws Exception {
        System.out.println(getLoginHtml());

        // Prepare database with secure password hashing
        createUser("user1", "StrongPassw0rd!");
        createUser("user2", "Compl3x!Pass#2");
        createUser("user3", "An0ther$trongOne");

        // 5 test cases
        String[] results = new String[5];
        results[0] = login("user1", "StrongPassw0rd!"); // expected: home.html
        results[1] = login("user2", "wrongPassword");   // expected: login.html
        results[2] = login("nosuchuser", "Anything1!"); // expected: login.html
        results[3] = login("bad user", "Compl3x!Pass#2"); // invalid username format -> login.html
        String longPwd = "x".repeat(129);
        results[4] = login("user3", longPwd);           // too long -> login.html

        for (int i = 0; i < results.length; i++) {
            System.out.println("Test " + (i + 1) + ": " + results[i]);
        }
    }
}
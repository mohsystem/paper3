import java.nio.charset.StandardCharsets;
import java.security.SecureRandom;
import java.util.Arrays;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;

public class Lse54 {
    private static final int MIN_USERNAME_LEN = 1;
    private static final int MAX_USERNAME_LEN = 64;
    private static final int MIN_PASSWORD_LEN = 12;
    private static final int MAX_PASSWORD_LEN = 128;
    private static final byte[] SALT;

    static {
        SALT = new byte[16];
        SecureRandom sr = new SecureRandom();
        sr.nextBytes(SALT);
    }

    private static volatile byte[] expectedHash = null;
    private static volatile String expectedUser = null;
    private static final Object INIT_LOCK = new Object();

    public static String login() {
        return "<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><title>Login</title></head>"
                + "<body><h1>Login</h1>"
                + "<form method=\"POST\" action=\"/do_login\">"
                + "<label>Username: <input type=\"text\" name=\"username\" maxlength=\"64\" required></label><br>"
                + "<label>Password: <input type=\"password\" name=\"password\" minlength=\"12\" maxlength=\"128\" required></label><br>"
                + "<button type=\"submit\">Login</button>"
                + "</form></body></html>";
    }

    public static String do_login(String username, String password) {
        if (username == null || password == null) {
            return "INVALID_INPUT";
        }
        if (!validUsername(username) || !validPasswordPolicy(password)) {
            return "INVALID_INPUT";
        }
        ensureExpected();
        if (expectedHash == null || expectedUser == null) {
            return "SERVER_NOT_CONFIGURED";
        }
        if (!expectedUser.equals(username)) {
            return "ACCESS_DENIED";
        }
        byte[] inputHash = derive(password.toCharArray(), SALT);
        boolean match = secureEquals(inputHash, expectedHash);
        Arrays.fill(inputHash, (byte) 0);
        return match ? "REDIRECT:/admin_page" : "ACCESS_DENIED";
    }

    private static void ensureExpected() {
        if (expectedHash != null && expectedUser != null) {
            return;
        }
        synchronized (INIT_LOCK) {
            if (expectedHash != null && expectedUser != null) {
                return;
            }
            String adminUser = getenvOrProperty("ADMIN_USER", "ADMIN_USER");
            if (adminUser == null || adminUser.isEmpty()) {
                adminUser = "admin";
            }
            String adminPassword = getenvOrProperty("ADMIN_PASSWORD", "ADMIN_PASSWORD");
            if (adminPassword == null || !validPasswordPolicy(adminPassword)) {
                expectedHash = null;
                expectedUser = null;
                return;
            }
            byte[] hash = derive(adminPassword.toCharArray(), SALT);
            expectedHash = hash;
            expectedUser = adminUser;
        }
    }

    private static String getenvOrProperty(String envName, String propName) {
        String v = System.getenv(envName);
        if (v == null) {
            v = System.getProperty(propName);
        }
        return v;
    }

    private static boolean validUsername(String u) {
        if (u == null) return false;
        int len = u.length();
        if (len < MIN_USERNAME_LEN || len > MAX_USERNAME_LEN) return false;
        for (int i = 0; i < len; i++) {
            char c = u.charAt(i);
            if (c > 0x7F) return false;
            if (!(Character.isDigit(c) || Character.isAlphabetic(c) || c == '_' || c == '-' || c == '.' || c == '@')) {
                return false;
            }
        }
        return true;
    }

    private static boolean validPasswordPolicy(String p) {
        if (p == null) return false;
        int len = p.length();
        if (len < MIN_PASSWORD_LEN || len > MAX_PASSWORD_LEN) return false;
        boolean hasUpper = false, hasLower = false, hasDigit = false, hasSpecial = false;
        for (int i = 0; i < len; i++) {
            char c = p.charAt(i);
            if (c > 0x7F) return false; // ASCII only
            if (Character.isUpperCase(c)) hasUpper = true;
            else if (Character.isLowerCase(c)) hasLower = true;
            else if (Character.isDigit(c)) hasDigit = true;
            else {
                // allow common specials
                String specials = "!@#$%^&*()_+-=[]{}|;:',.<>/?`~\"\\";
                if (specials.indexOf(c) >= 0) hasSpecial = true;
                else return false; // disallow other control/unexpected chars
            }
        }
        return hasUpper && hasLower && hasDigit && hasSpecial;
    }

    private static byte[] derive(char[] password, byte[] salt) {
        try {
            PBEKeySpec spec = new PBEKeySpec(password, salt, 210000, 256);
            Arrays.fill(password, '\0');
            SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
            byte[] key = skf.generateSecret(spec).getEncoded();
            spec.clearPassword();
            return key;
        } catch (Exception e) {
            return new byte[32];
        }
    }

    private static boolean secureEquals(byte[] a, byte[] b) {
        if (a == null || b == null) return false;
        int diff = a.length ^ b.length;
        int len = Math.min(a.length, b.length);
        for (int i = 0; i < len; i++) {
            diff |= a[i] ^ b[i];
        }
        return diff == 0;
    }

    public static void main(String[] args) {
        // Configure credentials for testing using system properties (safe for demo; prefer environment in production).
        System.setProperty("ADMIN_USER", "admin");
        System.setProperty("ADMIN_PASSWORD", "StrongPassw0rd!X");

        // Test 1: GET /login
        System.out.println("TEST1 login():");
        System.out.println(login());

        // Test 2: Successful login
        System.out.println("\nTEST2 do_login(admin, correct):");
        System.out.println(do_login("admin", "StrongPassw0rd!X"));

        // Test 3: Wrong password
        System.out.println("\nTEST3 do_login(admin, wrong password):");
        System.out.println(do_login("admin", "WrongPass123!"));

        // Test 4: Wrong username
        System.out.println("\nTEST4 do_login(notadmin, correct password):");
        System.out.println(do_login("notadmin", "StrongPassw0rd!X"));

        // Test 5: Invalid inputs (short password / invalid username chars)
        System.out.println("\nTEST5 do_login(invalid user, short password):");
        System.out.println(do_login("bad user", "shortPwd!"));
    }
}
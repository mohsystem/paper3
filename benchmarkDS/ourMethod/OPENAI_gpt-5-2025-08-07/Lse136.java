import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.SecureRandom;
import java.util.*;
import java.util.regex.Pattern;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;

public class Lse136 {

    // User record with salt, hash, and iterations
    static final class UserRecord {
        final byte[] salt;
        final byte[] hash;
        final int iterations;

        UserRecord(byte[] salt, byte[] hash, int iterations) {
            this.salt = Arrays.copyOf(salt, salt.length);
            this.hash = Arrays.copyOf(hash, hash.length);
            this.iterations = iterations;
        }
    }

    // Password policy: min 12, max 128, upper/lower/digit/special
    private static boolean isPasswordValidPolicy(String pwd) {
        if (pwd == null) return false;
        int len = pwd.length();
        if (len < 12 || len > 128) return false;
        boolean hasU = false, hasL = false, hasD = false, hasS = false;
        for (int i = 0; i < len; i++) {
            char c = pwd.charAt(i);
            if (Character.isUpperCase(c)) hasU = true;
            else if (Character.isLowerCase(c)) hasL = true;
            else if (Character.isDigit(c)) hasD = true;
            else hasS = true;
        }
        return hasU && hasL && hasD && hasS;
    }

    private static final Pattern USERNAME_RE = Pattern.compile("^[A-Za-z0-9._-]{3,64}$");

    private static boolean isUsernameValid(String u) {
        return u != null && USERNAME_RE.matcher(u).matches();
    }

    private static byte[] pbkdf2(char[] password, byte[] salt, int iterations, int dkLen) throws Exception {
        PBEKeySpec spec = new PBEKeySpec(password, salt, iterations, dkLen * 8);
        SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
        return skf.generateSecret(spec).getEncoded();
    }

    private static UserRecord createUserRecord(String password, int iterations) throws Exception {
        SecureRandom sr = SecureRandom.getInstanceStrong();
        byte[] salt = new byte[16];
        sr.nextBytes(salt);
        byte[] hash = pbkdf2(password.toCharArray(), salt, iterations, 32);
        return new UserRecord(salt, hash, iterations);
    }

    private static boolean verifyPassword(String password, UserRecord rec) throws Exception {
        if (rec == null || password == null) return false;
        byte[] derived = pbkdf2(password.toCharArray(), rec.salt, rec.iterations, 32);
        boolean eq = MessageDigest.isEqual(derived, rec.hash);
        Arrays.fill(derived, (byte) 0);
        return eq;
    }

    private static Map<String, String> parseQuery(String query, int maxPairs, int maxKeyValLen) {
        Map<String, String> map = new HashMap<>();
        if (query == null || query.isEmpty()) return map;
        String[] pairs = query.split("&", -1);
        int count = 0;
        for (String pair : pairs) {
            if (pair.isEmpty()) continue;
            int idx = pair.indexOf('=');
            String k = idx >= 0 ? pair.substring(0, idx) : pair;
            String v = idx >= 0 ? pair.substring(idx + 1) : "";
            String dk = urlDecode(k, maxKeyValLen);
            String dv = urlDecode(v, maxKeyValLen);
            if (dk == null || dv == null) return Collections.emptyMap();
            if (!map.containsKey(dk)) {
                map.put(dk, dv);
                count++;
                if (count > maxPairs) return Collections.emptyMap();
            }
        }
        return map;
    }

    private static String urlDecode(String s, int maxLen) {
        try {
            String decoded = java.net.URLDecoder.decode(s, StandardCharsets.UTF_8.name());
            if (decoded.length() > maxLen) return null;
            return decoded;
        } catch (Exception e) {
            return null;
        }
    }

    private static String extractPath(String url, int maxLen) {
        if (url == null) return null;
        if (url.length() > 2048) return null;
        try {
            java.net.URI u = new java.net.URI(url);
            String path = u.getPath();
            if (path == null) return null;
            if (path.length() > maxLen) return null;
            return path;
        } catch (Exception e) {
            // Fallback minimal parse
            int q = url.indexOf('?');
            String base = q >= 0 ? url.substring(0, q) : url;
            int scheme = base.indexOf("://");
            int pathStart = 0;
            if (scheme >= 0) {
                int slash = base.indexOf('/', scheme + 3);
                pathStart = (slash >= 0) ? slash : base.length();
            }
            String path = base.substring(pathStart);
            if (path.length() > maxLen) return null;
            return path.isEmpty() ? "/" : path;
        }
    }

    private static String extractQuery(String url) {
        if (url == null) return null;
        int q = url.indexOf('?');
        if (q < 0) return "";
        int hash = url.indexOf('#', q + 1);
        return (hash >= 0) ? url.substring(q + 1, hash) : url.substring(q + 1);
    }

    // The requested function
    public static String check_mod(String url, Map<String, UserRecord> userStore) {
        try {
            String path = extractPath(url, 256);
            if (path == null || !"/check_mod".equals(path)) return "false";

            String query = extractQuery(url);
            Map<String, String> params = parseQuery(query, 10, 256);
            if (params.isEmpty()) return "false";

            String username = params.get("username");
            String password = params.get("password");
            if (!isUsernameValid(username)) return "false";
            if (password == null || password.length() > 128) return "false";
            if (!isPasswordValidPolicy(password)) return "false";

            UserRecord rec = userStore.get(username);
            if (rec == null) return "false";
            if (verifyPassword(password, rec)) return "true";
            return "false";
        } catch (Exception e) {
            return "false";
        }
    }

    public static void main(String[] args) throws Exception {
        // Build a simple in-memory user store
        Map<String, UserRecord> store = new HashMap<>();
        String goodUser = "alice";
        String goodPass = "Str0ng-P@ssw0rd!";
        store.put(goodUser, createUserRecord(goodPass, 210000));

        // 5 test cases
        String t1 = "http://localhost/check_mod?username=alice&password=Str0ng-P%40ssw0rd%21"; // true
        String t2 = "/check_mod?username=alice&password=Wrong-P%40ssw0rd%21"; // false
        String t3 = "/other?username=alice&password=Str0ng-P%40ssw0rd%21"; // false (wrong path)
        String t4 = "/check_mod?username=alice"; // false (missing password)
        String t5 = "/check_mod?username=alice&password=short1!A"; // false (policy)

        System.out.println(check_mod(t1, store));
        System.out.println(check_mod(t2, store));
        System.out.println(check_mod(t3, store));
        System.out.println(check_mod(t4, store));
        System.out.println(check_mod(t5, store));
    }
}
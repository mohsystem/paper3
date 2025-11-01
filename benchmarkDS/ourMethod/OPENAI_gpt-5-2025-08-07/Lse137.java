import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;
import java.net.URLDecoder;
import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;
import java.security.SecureRandom;
import java.security.GeneralSecurityException;
import java.util.*;

public class Lse137 {

    static final int SALT_LEN = 16;
    static final int HASH_LEN = 32;
    static final int PBKDF2_ITERS = 210_000;

    static final class User {
        final String username;
        final byte[] salt;
        final byte[] hash;
        final boolean moderator;
        final String session;

        User(String username, byte[] salt, byte[] hash, boolean moderator, String session) {
            this.username = username;
            this.salt = salt;
            this.hash = hash;
            this.moderator = moderator;
            this.session = session;
        }
    }

    // In-memory "database"
    private static final Map<String, User> DB = new HashMap<>();
    private static final SecureRandom RNG = new SecureRandom();

    private static boolean strongPassword(String pw) {
        if (pw == null) return false;
        if (pw.length() < 12 || pw.length() > 256) return false;
        boolean up = false, lo = false, di = false, sp = false;
        for (char c : pw.toCharArray()) {
            if (Character.isUpperCase(c)) up = true;
            else if (Character.isLowerCase(c)) lo = true;
            else if (Character.isDigit(c)) di = true;
            else sp = true;
        }
        return up && lo && di && sp;
    }

    private static byte[] pbkdf2(char[] password, byte[] salt, int iterations, int dkLen) throws GeneralSecurityException {
        PBEKeySpec spec = new PBEKeySpec(password, salt, iterations, dkLen * 8);
        SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
        return skf.generateSecret(spec).getEncoded();
    }

    private static boolean constantTimeEq(byte[] a, byte[] b) {
        if (a == null || b == null) return false;
        if (a.length != b.length) return false;
        int r = 0;
        for (int i = 0; i < a.length; i++) {
            r |= (a[i] ^ b[i]);
        }
        return r == 0;
    }

    private static String urlDecode(String s) {
        try {
            return URLDecoder.decode(s, StandardCharsets.UTF_8.name());
        } catch (Exception ex) {
            return "";
        }
    }

    private static Map<String, String> parseQuery(String url) {
        Map<String, String> params = new HashMap<>();
        if (url == null) return params;
        int q = url.indexOf('?');
        if (q < 0) return params;
        String query = url.substring(q + 1);
        String[] parts = query.split("&");
        for (String p : parts) {
            int eq = p.indexOf('=');
            if (eq <= 0) continue;
            String k = urlDecode(p.substring(0, eq));
            String v = urlDecode(p.substring(eq + 1));
            params.put(k, v);
        }
        return params;
    }

    private static boolean validateIdentifier(String s, int maxLen) {
        if (s == null) return false;
        if (s.length() == 0 || s.length() > maxLen) return false;
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            if (!(Character.isLetterOrDigit(c) || c == '_' || c == '-' || c == '.' || c == '@')) {
                return false;
            }
        }
        return true;
    }

    public static boolean auth_mod(String username, String password, String session) {
        if (!validateIdentifier(username, 64)) return false;
        if (password == null || password.length() < 1 || password.length() > 256) return false;
        if (!validateIdentifier(session, 128)) return false;

        User u = DB.get(username);
        if (u == null) return false;

        // Verify session first
        if (!constantTimeEq(u.session.getBytes(StandardCharsets.UTF_8), session.getBytes(StandardCharsets.UTF_8))) {
            return false;
        }

        try {
            byte[] derived = pbkdf2(password.toCharArray(), u.salt, PBKDF2_ITERS, HASH_LEN);
            boolean ok = constantTimeEq(derived, u.hash);
            Arrays.fill(derived, (byte) 0);
            return ok && u.moderator;
        } catch (GeneralSecurityException e) {
            return false;
        }
    }

    public static boolean check_mod(String url) {
        if (url == null || !url.startsWith("/check_mod")) return false;
        Map<String, String> params = parseQuery(url);
        String username = params.get("username");
        String password = params.get("password");
        String session = params.get("session");
        if (username == null || password == null || session == null) return false;
        return auth_mod(username, password, session);
    }

    private static String randToken(int len) {
        byte[] buf = new byte[len];
        RNG.nextBytes(buf);
        StringBuilder sb = new StringBuilder(len * 2);
        for (byte b : buf) {
            sb.append(String.format("%02x", b));
        }
        return sb.toString();
    }

    private static void addUser(String username, String passwordEnv, boolean moderator, String sessionEnv) {
        if (!validateIdentifier(username, 64)) return;
        String pw = passwordEnv;
        if (pw == null) return;
        if (!strongPassword(pw)) return;

        byte[] salt = new byte[SALT_LEN];
        RNG.nextBytes(salt);
        byte[] hash;
        try {
            hash = pbkdf2(pw.toCharArray(), salt, PBKDF2_ITERS, HASH_LEN);
        } catch (GeneralSecurityException e) {
            return;
        }
        String session = sessionEnv;
        if (session == null || !validateIdentifier(session, 128)) {
            session = randToken(16);
        }
        DB.put(username, new User(username, salt, hash, moderator, session));
        // Do not log secrets
    }

    public static void main(String[] args) {
        String modUser = Optional.ofNullable(System.getenv("MOD_USER")).orElse("mod1");
        String modPass = System.getenv("MOD_PASS"); // must be provided and strong
        String modSess = System.getenv("MOD_SESSION");

        String regUser = Optional.ofNullable(System.getenv("USER_USER")).orElse("user1");
        String regPass = System.getenv("USER_PASS"); // must be provided and strong
        String regSess = System.getenv("USER_SESSION");

        addUser(modUser, modPass, true, modSess);
        addUser(regUser, regPass, false, regSess);

        // 5 test cases
        List<String> tests = new ArrayList<>();
        // 1) Moderator correct if env provided; else will be false (user not added)
        if (DB.containsKey(modUser)) {
            String sess = DB.get(modUser).session;
            String url1 = "/check_mod?username=" + modUser + "&password=" + urlEncodeSafe(modPass) + "&session=" + sess;
            tests.add(url1);
        } else {
            tests.add("/check_mod?username=" + modUser + "&password=WrongPass123!&session=invalidsession");
        }
        // 2) Moderator wrong password
        if (DB.containsKey(modUser)) {
            String sess = DB.get(modUser).session;
            tests.add("/check_mod?username=" + modUser + "&password=WrongPass123!&session=" + sess);
        } else {
            tests.add("/check_mod?username=" + modUser + "&password=WrongPass123!&session=wrong");
        }
        // 3) Moderator correct password but wrong session
        if (DB.containsKey(modUser)) {
            tests.add("/check_mod?username=" + modUser + "&password=" + urlEncodeSafe(modPass) + "&session=bad" + randToken(4));
        } else {
            tests.add("/check_mod?username=nouser&password=SomethingStrong1!&session=bad");
        }
        // 4) Non-existent user
        tests.add("/check_mod?username=ghost&password=NopePass1!&session=anything123");
        // 5) Regular user correct creds should return false (not moderator)
        if (DB.containsKey(regUser)) {
            String sess = DB.get(regUser).session;
            tests.add("/check_mod?username=" + regUser + "&password=" + urlEncodeSafe(regPass) + "&session=" + sess);
        } else {
            tests.add("/check_mod?username=" + regUser + "&password=WrongPass123!&session=wrong");
        }

        int i = 1;
        for (String t : tests) {
            boolean res = check_mod(t);
            System.out.println("Test " + (i++) + ": " + res);
        }
    }

    private static String urlEncodeSafe(String s) {
        if (s == null) return "";
        StringBuilder sb = new StringBuilder();
        for (byte b : s.getBytes(StandardCharsets.UTF_8)) {
            char c = (char) b;
            if ((c >= 'A' && c <= 'Z') ||
                (c >= 'a' && c <= 'z') ||
                (c >= '0' && c <= '9') ||
                c == '-' || c == '_' || c == '.' || c == '~') {
                sb.append((char) c);
            } else {
                sb.append(String.format("%%%02X", b));
            }
        }
        return sb.toString();
    }
}
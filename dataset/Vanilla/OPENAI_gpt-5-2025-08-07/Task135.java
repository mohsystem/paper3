import java.security.MessageDigest;
import java.security.SecureRandom;
import java.util.Base64;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

public class Task135 {
    private static final SecureRandom RNG = new SecureRandom();
    private static final Map<String, String> TOKENS = new ConcurrentHashMap<>();

    public static String issueToken(String sessionId) {
        String token = randomToken(32);
        TOKENS.put(sessionId, token);
        return token;
    }

    public static boolean validateToken(String sessionId, String token) {
        String expected = TOKENS.get(sessionId);
        if (expected == null || token == null) return false;
        return constantTimeEquals(expected, token);
    }

    public static String rotateToken(String sessionId) {
        return issueToken(sessionId);
    }

    public static void invalidateSession(String sessionId) {
        TOKENS.remove(sessionId);
    }

    private static String randomToken(int numBytes) {
        byte[] bytes = new byte[numBytes];
        RNG.nextBytes(bytes);
        return Base64.getUrlEncoder().withoutPadding().encodeToString(bytes);
    }

    private static boolean constantTimeEquals(String a, String b) {
        byte[] aa = a.getBytes(java.nio.charset.StandardCharsets.UTF_8);
        byte[] bb = b.getBytes(java.nio.charset.StandardCharsets.UTF_8);
        return MessageDigest.isEqual(aa, bb);
    }

    public static void main(String[] args) {
        // 5 test cases
        String s1 = "sessionA";
        String s2 = "sessionB";

        String t1 = issueToken(s1);
        System.out.println("Test1 (valid token): " + validateToken(s1, t1));

        System.out.println("Test2 (invalid token): " + validateToken(s1, "invalidtoken"));

        System.out.println("Test3 (missing session): " + validateToken("noSession", t1));

        String t2 = issueToken(s2);
        System.out.println("Test4 (cross-session misuse): " + validateToken(s2, t1));

        String old = t1;
        String rotated = rotateToken(s1);
        boolean oldValid = validateToken(s1, old);
        boolean newValid = validateToken(s1, rotated);
        System.out.println("Test5 (rotate old invalid/new valid): " + (!oldValid && newValid));
    }
}
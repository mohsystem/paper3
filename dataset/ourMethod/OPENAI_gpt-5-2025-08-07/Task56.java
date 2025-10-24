import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.SecureRandom;
import javax.crypto.Mac;
import javax.crypto.spec.SecretKeySpec;
import java.util.Base64;

public class Task56 {
    private static final byte[] MAGIC = new byte[] { 'T', 'K', 'N', '1' };
    private static final byte VERSION = 1;
    private static final int NONCE_LEN = 16;
    private static final int TAG_LEN = 32;
    private static final int MAX_USER_LEN = 1024;
    private static final int MIN_KEY_LEN = 16;
    private static final int MIN_TTL = 1;
    private static final int MAX_TTL = 604800; // 7 days

    public static String generateToken(String userId, byte[] key, int ttlSeconds) {
        if (userId == null) {
            throw new IllegalArgumentException("userId is null");
        }
        byte[] userBytes = userId.getBytes(StandardCharsets.UTF_8);
        if (userBytes.length == 0 || userBytes.length > MAX_USER_LEN) {
            throw new IllegalArgumentException("userId length invalid");
        }
        if (key == null || key.length < MIN_KEY_LEN) {
            throw new IllegalArgumentException("key is null or too short");
        }
        if (ttlSeconds < MIN_TTL || ttlSeconds > MAX_TTL) {
            throw new IllegalArgumentException("ttl out of range");
        }

        long now = System.currentTimeMillis() / 1000L;
        byte[] nonce = new byte[NONCE_LEN];
        new SecureRandom().nextBytes(nonce);

        int baseLen = MAGIC.length + 1 + 8 + 4 + NONCE_LEN + 2 + userBytes.length;
        ByteBuffer buf = ByteBuffer.allocate(baseLen + TAG_LEN).order(ByteOrder.BIG_ENDIAN);
        buf.put(MAGIC);
        buf.put(VERSION);
        buf.putLong(now);
        buf.putInt(ttlSeconds);
        buf.put(nonce);
        buf.putShort((short) (userBytes.length & 0xFFFF));
        buf.put(userBytes);

        byte[] toMac = new byte[buf.position()];
        buf.flip();
        buf.get(toMac);

        byte[] tag = hmacSha256(key, toMac);

        ByteBuffer full = ByteBuffer.allocate(toMac.length + tag.length);
        full.put(toMac);
        full.put(tag);

        byte[] tokenBytes = full.array();
        String token = Base64.getUrlEncoder().withoutPadding().encodeToString(tokenBytes);
        return token;
    }

    public static boolean verifyToken(String token, byte[] key, String expectedUserId, long nowEpochSeconds) {
        if (token == null || key == null || expectedUserId == null) return false;
        if (key.length < MIN_KEY_LEN) return false;
        byte[] raw;
        try {
            raw = Base64.getUrlDecoder().decode(token);
        } catch (IllegalArgumentException e) {
            return false;
        }
        int minLen = MAGIC.length + 1 + 8 + 4 + NONCE_LEN + 2 + TAG_LEN;
        if (raw.length < minLen) return false;

        ByteBuffer buf = ByteBuffer.wrap(raw).order(ByteOrder.BIG_ENDIAN);

        byte[] magic = new byte[4];
        buf.get(magic);
        if (!MessageDigest.isEqual(magic, MAGIC)) return false;

        byte ver = buf.get();
        if (ver != VERSION) return false;

        long ts = buf.getLong();
        int ttl = buf.getInt();
        if (ttl < MIN_TTL || ttl > MAX_TTL) return false;

        byte[] nonce = new byte[NONCE_LEN];
        buf.get(nonce);

        int userLen = buf.getShort() & 0xFFFF;
        if (userLen < 1 || userLen > MAX_USER_LEN) return false;

        int remainingBeforeTag = raw.length - buf.position() - TAG_LEN;
        if (remainingBeforeTag != userLen) return false;

        byte[] userBytes = new byte[userLen];
        buf.get(userBytes);

        byte[] tag = new byte[TAG_LEN];
        buf.get(tag);

        // Recompute tag
        byte[] toMac = new byte[raw.length - TAG_LEN];
        System.arraycopy(raw, 0, toMac, 0, toMac.length);
        byte[] expectedTag = hmacSha256(key, toMac);
        if (!MessageDigest.isEqual(tag, expectedTag)) return false;

        // Compare user id (not secret, but do constant-time anyway)
        byte[] expectedUserBytes = expectedUserId.getBytes(StandardCharsets.UTF_8);
        if (!MessageDigest.isEqual(userBytes, expectedUserBytes)) return false;

        long now = (nowEpochSeconds > 0) ? nowEpochSeconds : (System.currentTimeMillis() / 1000L);
        long exp = ts + (long) ttl;
        if (now > exp) return false;

        return true;
    }

    private static byte[] hmacSha256(byte[] key, byte[] data) {
        try {
            Mac mac = Mac.getInstance("HmacSHA256");
            mac.init(new SecretKeySpec(key, "HmacSHA256"));
            return mac.doFinal(data);
        } catch (Exception e) {
            // Fail closed
            return new byte[TAG_LEN];
        }
    }

    private static String tamperToken(String token) {
        if (token == null || token.isEmpty()) return token;
        char[] alpha = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_".toCharArray();
        java.util.HashSet<Character> set = new java.util.HashSet<>();
        for (char c : alpha) set.add(c);
        char[] chars = token.toCharArray();
        int i = chars.length - 1;
        if (!set.contains(chars[i])) {
            chars[i] = alpha[0];
            return new String(chars);
        }
        chars[i] = (chars[i] == alpha[0]) ? alpha[1] : alpha[0];
        return new String(chars);
    }

    public static void main(String[] args) {
        SecureRandom sr = new SecureRandom();
        byte[] key = new byte[32];
        sr.nextBytes(key);

        String[] users = new String[] { "alice", "bob", "carol", "dave_123", "eve@example.com" };

        // Test 1: valid token
        String t1 = generateToken(users[0], key, 600);
        boolean r1 = verifyToken(t1, key, users[0], System.currentTimeMillis() / 1000L);
        System.out.println("Test1 valid: " + r1);

        // Test 2: expired token
        String t2 = generateToken(users[1], key, 1);
        boolean r2 = verifyToken(t2, key, users[1], (System.currentTimeMillis() / 1000L) + 5000);
        System.out.println("Test2 expired: " + (!r2));

        // Test 3: wrong user id
        String t3 = generateToken(users[2], key, 600);
        boolean r3 = verifyToken(t3, key, users[0], System.currentTimeMillis() / 1000L);
        System.out.println("Test3 wrong user: " + (!r3));

        // Test 4: tampered token
        String t4 = generateToken(users[3], key, 600);
        String t4bad = tamperToken(t4);
        boolean r4 = verifyToken(t4bad, key, users[3], System.currentTimeMillis() / 1000L);
        System.out.println("Test4 tampered: " + (!r4));

        // Test 5: wrong key
        byte[] wrongKey = new byte[32];
        sr.nextBytes(wrongKey);
        String t5 = generateToken(users[4], key, 600);
        boolean r5 = verifyToken(t5, wrongKey, users[4], System.currentTimeMillis() / 1000L);
        System.out.println("Test5 wrong key: " + (!r5));
    }
}
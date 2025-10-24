import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;

public class Task85 {
    private static final SecureRandom SECURE_RANDOM;
    private static final char[] LETTERS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz".toCharArray();

    static {
        SecureRandom sr;
        try {
            sr = SecureRandom.getInstanceStrong();
        } catch (NoSuchAlgorithmException e) {
            sr = new SecureRandom();
        }
        SECURE_RANDOM = sr;
    }

    public static String generateRandomString(int length) {
        if (length < 0) throw new IllegalArgumentException("length must be non-negative");
        if (length == 0) return "";
        StringBuilder sb = new StringBuilder(length);
        final int alphabet = LETTERS.length; // 52
        final int m = 256 - (256 % alphabet); // 208 to avoid modulo bias
        byte[] buf = new byte[128];
        int idx = buf.length; // force initial refill

        while (sb.length() < length) {
            if (idx >= buf.length) {
                SECURE_RANDOM.nextBytes(buf);
                idx = 0;
            }
            int b = buf[idx++] & 0xFF;
            if (b >= m) continue;
            sb.append(LETTERS[b % alphabet]);
        }
        return sb.toString();
    }

    public static void main(String[] args) {
        int[] tests = new int[] {0, 1, 10, 32, 64};
        for (int len : tests) {
            String s = generateRandomString(len);
            System.out.println(len + ": " + s);
        }
    }
}
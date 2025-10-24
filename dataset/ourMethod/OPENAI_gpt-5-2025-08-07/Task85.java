import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;

public final class Task85 {

    private static final String LETTERS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    private static final int LETTERS_LEN = LETTERS.length(); // 52
    private static final int REJECTION_LIMIT = (256 / LETTERS_LEN) * LETTERS_LEN; // 208
    private static final int MAX_LEN = 1_000_000;

    private static final SecureRandom RNG = createSecureRandom();

    private Task85() {
        // Prevent instantiation
    }

    private static SecureRandom createSecureRandom() {
        try {
            return SecureRandom.getInstanceStrong();
        } catch (NoSuchAlgorithmException e) {
            // Fallback to default SecureRandom which is still cryptographically strong
            return new SecureRandom();
        }
    }

    public static String randomAsciiLetters(int length) {
        if (length < 0 || length > MAX_LEN) {
            throw new IllegalArgumentException("length must be in range [0, " + MAX_LEN + "]");
        }
        StringBuilder sb = new StringBuilder(length);
        byte[] one = new byte[1];
        while (sb.length() < length) {
            RNG.nextBytes(one);
            int v = one[0] & 0xFF;
            if (v < REJECTION_LIMIT) {
                int idx = v % LETTERS_LEN;
                sb.append(LETTERS.charAt(idx));
            }
        }
        return sb.toString();
    }

    public static void main(String[] args) {
        int[] tests = new int[] {0, 1, 10, 32, 100};
        for (int t : tests) {
            String s = randomAsciiLetters(t);
            System.out.println("len=" + t + " str=" + s);
        }
    }
}
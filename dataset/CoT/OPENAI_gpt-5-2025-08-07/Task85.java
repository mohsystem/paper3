import java.security.SecureRandom;

public class Task85 {
    private static final char[] LETTERS = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ".toCharArray();
    private static final SecureRandom RNG = createRng();

    private static SecureRandom createRng() {
        try {
            return SecureRandom.getInstanceStrong();
        } catch (Exception e) {
            return new SecureRandom();
        }
    }

    public static String generateRandomString(int length) {
        if (length < 0) {
            throw new IllegalArgumentException("length must be non-negative");
        }
        StringBuilder sb = new StringBuilder(length);
        for (int i = 0; i < length; i++) {
            int idx = RNG.nextInt(LETTERS.length);
            sb.append(LETTERS[idx]);
        }
        return sb.toString();
    }

    public static void main(String[] args) {
        int[] tests = {0, 1, 16, 32, 100};
        for (int t : tests) {
            String s = generateRandomString(t);
            System.out.println(t + ": " + s);
        }
    }
}
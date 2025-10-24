import java.security.SecureRandom;

public class Task85 {
    private static final char[] ALPHABET;
    private static final SecureRandom RNG = new SecureRandom();

    static {
        StringBuilder sb = new StringBuilder(52);
        for (char c = 'A'; c <= 'Z'; c++) sb.append(c);
        for (char c = 'a'; c <= 'z'; c++) sb.append(c);
        ALPHABET = sb.toString().toCharArray();
    }

    public static String randomAsciiLetters(int length) {
        if (length < 0) throw new IllegalArgumentException("length must be non-negative");
        char[] out = new char[length];
        for (int i = 0; i < length; i++) {
            out[i] = ALPHABET[RNG.nextInt(ALPHABET.length)];
        }
        return new String(out);
    }

    public static void main(String[] args) {
        int[] tests = {0, 1, 8, 16, 32};
        for (int n : tests) {
            String s = randomAsciiLetters(n);
            System.out.println(n + ": " + s);
        }
    }
}
import java.security.SecureRandom;
import java.util.HashSet;
import java.util.Set;

public class Task84 {
    private static final String ALPHABET = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
    private static final SecureRandom RNG = new SecureRandom();
    private static final Set<String> ISSUED = new HashSet<>();

    public static String generateSessionId(int length) {
        if (length <= 0) throw new IllegalArgumentException("length must be > 0");
        String id;
        do {
            StringBuilder sb = new StringBuilder(length);
            for (int i = 0; i < length; i++) {
                int idx = RNG.nextInt(ALPHABET.length());
                sb.append(ALPHABET.charAt(idx));
            }
            id = sb.toString();
        } while (!ISSUED.add(id));
        return id;
    }

    public static void main(String[] args) {
        int[] lengths = {16, 24, 32, 40, 48};
        for (int len : lengths) {
            System.out.println(generateSessionId(len));
        }
    }
}
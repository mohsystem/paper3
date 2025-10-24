import java.security.SecureRandom;

public class Task56 {
    private static final String ALPHABET = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
    private static final SecureRandom RNG = new SecureRandom();

    public static String generateToken(String userId, int length) {
        int n = Math.max(16, length);
        char[] buf = new char[n];
        for (int i = 0; i < n; i++) {
            buf[i] = ALPHABET.charAt(RNG.nextInt(ALPHABET.length()));
        }
        return new String(buf);
    }

    public static void main(String[] args) {
        System.out.println("user1: " + generateToken("user1", 32));
        System.out.println("user2: " + generateToken("user2", 24));
        System.out.println("user3: " + generateToken("user3", 48));
        System.out.println("user4: " + generateToken("user4", 64));
        System.out.println("user5: " + generateToken("user5", 32));
    }
}
import java.security.SecureRandom;
import java.util.Base64;
import java.nio.charset.StandardCharsets;

public class Task128 {
    private static final SecureRandom RNG = new SecureRandom();
    private static final char[] ALPHANUM =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789".toCharArray();
    private static final int MAX_BYTES = 1_048_576; // 1 MiB safety limit

    public static byte[] randomBytes(int length) {
        if (length < 0 || length > MAX_BYTES) {
            throw new IllegalArgumentException("length out of allowed range");
        }
        byte[] buf = new byte[length];
        RNG.nextBytes(buf);
        return buf;
    }

    public static String toHex(byte[] data) {
        if (data == null) {
            throw new IllegalArgumentException("data must not be null");
        }
        StringBuilder sb = new StringBuilder(data.length * 2);
        for (byte b : data) {
            sb.append(Character.forDigit((b >>> 4) & 0xF, 16));
            sb.append(Character.forDigit(b & 0xF, 16));
        }
        return sb.toString();
    }

    public static String randomHex(int nBytes) {
        return toHex(randomBytes(nBytes));
    }

    public static String randomBase64Url(int nBytes) {
        return Base64.getUrlEncoder().withoutPadding().encodeToString(randomBytes(nBytes));
    }

    public static String randomAlphaNum(int length) {
        if (length < 0 || length > MAX_BYTES) {
            throw new IllegalArgumentException("length out of allowed range");
        }
        StringBuilder sb = new StringBuilder(length);
        for (int i = 0; i < length; i++) {
            int idx = RNG.nextInt(ALPHANUM.length);
            sb.append(ALPHANUM[idx]);
        }
        return sb.toString();
    }

    public static int randomIntInRange(int min, int max) {
        if (min == Integer.MIN_VALUE && max == Integer.MAX_VALUE) {
            return RNG.nextInt(); // full int range
        }
        if (min > max) {
            throw new IllegalArgumentException("min must be <= max");
        }
        long bound = (long) max - (long) min + 1L; // range size in [1..2^32]
        if (bound <= 0L) {
            // overflow means full 32-bit range
            return RNG.nextInt();
        }
        if (bound <= (long) Integer.MAX_VALUE) {
            return min + RNG.nextInt((int) bound);
        }
        // Use 32-bit unsigned rejection sampling for bound in (2^31 .. 2^32]
        while (true) {
            long v = RNG.nextInt() & 0xFFFF_FFFFL; // treat as unsigned
            long limit = (0x1_0000_0000L / bound) * bound; // largest multiple of bound < 2^32
            if (v < limit) {
                return (int) (min + (v % bound));
            }
        }
    }

    public static void main(String[] args) {
        // 5 test cases
        String t1 = randomHex(16);
        String t2 = randomBase64Url(24);
        String t3 = randomAlphaNum(20);
        int n1 = randomIntInRange(0, 10);
        int n2 = randomIntInRange(Integer.MIN_VALUE, Integer.MAX_VALUE);

        System.out.println("Random hex (16 bytes): " + t1);
        System.out.println("Random base64url (24 bytes): " + t2);
        System.out.println("Random alphanum (20 chars): " + t3);
        System.out.println("Random int [0,10]: " + n1);
        System.out.println("Random int full range: " + n2);
    }
}
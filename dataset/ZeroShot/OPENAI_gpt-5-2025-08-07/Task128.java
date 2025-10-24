import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.Arrays;

public class Task128 {
    private static final SecureRandom RNG = createSecureRandom();
    private static SecureRandom createSecureRandom() {
        try {
            return SecureRandom.getInstanceStrong();
        } catch (NoSuchAlgorithmException e) {
            return new SecureRandom();
        }
    }

    public static byte[] secureRandomBytes(int length) {
        if (length < 0) throw new IllegalArgumentException("length must be non-negative");
        byte[] out = new byte[length];
        if (length > 0) {
            RNG.nextBytes(out);
        }
        return out;
    }

    public static String secureHexToken(int numBytes) {
        byte[] bytes = secureRandomBytes(numBytes);
        return bytesToHex(bytes);
    }

    public static long secureRandomLong(long minInclusive, long maxInclusive) {
        if (minInclusive > maxInclusive) throw new IllegalArgumentException("min > max");
        long bound = maxInclusive - minInclusive + 1;
        if (bound <= 0) throw new IllegalArgumentException("Range too large");
        long mask = bound - 1;
        // spread highest bit to all lower bits
        mask |= (mask >>> 1);
        mask |= (mask >>> 2);
        mask |= (mask >>> 4);
        mask |= (mask >>> 8);
        mask |= (mask >>> 16);
        mask |= (mask >>> 32);
        long r;
        do {
            r = RNG.nextLong() & mask;
        } while (r >= bound);
        return minInclusive + r;
    }

    private static String bytesToHex(byte[] bytes) {
        char[] hexArray = "0123456789abcdef".toCharArray();
        char[] hexChars = new char[bytes.length * 2];
        for (int j = 0; j < bytes.length; j++) {
            int v = bytes[j] & 0xFF;
            hexChars[j * 2] = hexArray[v >>> 4];
            hexChars[j * 2 + 1] = hexArray[v & 0x0F];
        }
        return new String(hexChars);
    }

    public static void main(String[] args) {
        // 5 test cases
        System.out.println("Hex token (16 bytes): " + secureHexToken(16));
        System.out.println("Random long [1, 100]: " + secureRandomLong(1, 100));
        System.out.println("Hex token (32 bytes): " + secureHexToken(32));
        System.out.println("Hex token (0 bytes): " + secureHexToken(0));
        System.out.println("Random long [0, 2^48]: " + secureRandomLong(0, (1L << 48)));
    }
}
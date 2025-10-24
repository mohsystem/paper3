import java.security.SecureRandom;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;

public class Task84 {
    private static final SecureRandom SECURE_RANDOM;
    private static final Set<String> ISSUED = ConcurrentHashMap.newKeySet();

    static {
        SecureRandom sr;
        try {
            sr = SecureRandom.getInstanceStrong();
        } catch (Exception e) {
            sr = new SecureRandom();
        }
        SECURE_RANDOM = sr;
    }

    private static String toHex(byte[] bytes) {
        char[] hexArray = "0123456789abcdef".toCharArray();
        char[] hexChars = new char[bytes.length * 2];
        for (int j = 0; j < bytes.length; j++) {
            int v = bytes[j] & 0xFF;
            hexChars[j * 2] = hexArray[v >>> 4];
            hexChars[j * 2 + 1] = hexArray[v & 0x0F];
        }
        return new String(hexChars);
    }

    private static int clampLength(int numBytes) {
        // Clamp to a reasonable secure range; default to 32 if out of bounds
        if (numBytes < 16 || numBytes > 64) return 32;
        return numBytes;
    }

    public static String generateSessionId(int numBytes) {
        int n = clampLength(numBytes);
        byte[] buf = new byte[n];
        while (true) {
            SECURE_RANDOM.nextBytes(buf);
            String id = toHex(buf);
            if (ISSUED.add(id)) {
                return id;
            }
            // extremely unlikely collision; regenerate
        }
    }

    public static void main(String[] args) {
        // 5 test cases
        System.out.println(generateSessionId(32));
        System.out.println(generateSessionId(32));
        System.out.println(generateSessionId(32));
        System.out.println(generateSessionId(32));
        System.out.println(generateSessionId(32));
    }
}
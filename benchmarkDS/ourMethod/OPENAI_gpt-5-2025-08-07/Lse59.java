import java.nio.charset.StandardCharsets;
import java.util.Arrays;

public class Lse59 {
    private static final int MAX_STACK = 1_000_000;

    // Allocates a buffer of size bufferSize on the stack (local array) and fills with 'A'
    // Returns the resulting string (for verification/output).
    public static String fillBufferOnStack(int bufferSize) {
        if (bufferSize < 0 || bufferSize > MAX_STACK) {
            throw new IllegalArgumentException("invalid size");
        }
        byte[] buf = new byte[bufferSize]; // local variable; JVM may stack-allocate small arrays
        if (bufferSize > 0) {
            Arrays.fill(buf, (byte) 'A');
        }
        return new String(buf, StandardCharsets.US_ASCII);
    }

    public static void main(String[] args) {
        int[] tests = new int[] {0, 1, 5, 16, 1024};
        for (int sz : tests) {
            try {
                String out = fillBufferOnStack(sz);
                System.out.println("size=" + sz + " len=" + out.length() + " sample=" + (out.length() <= 32 ? out : out.substring(0, 32) + "..."));
            } catch (IllegalArgumentException ex) {
                System.out.println("size=" + sz + " error=" + ex.getMessage());
            }
        }
    }
}
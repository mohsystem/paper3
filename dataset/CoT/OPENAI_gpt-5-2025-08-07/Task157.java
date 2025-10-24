import java.util.Arrays;

public class Task157 {
    private static final int MAX_SIZE = 16 * 1024 * 1024;

    // Dynamically allocates a byte array, uses it, then securely clears it before letting GC reclaim it.
    // Returns checksum on success, -1 on error (invalid size or OOM).
    public static long allocateAndUse(int size) {
        if (size <= 0 || size > MAX_SIZE) {
            return -1L;
        }
        byte[] buf = null;
        try {
            buf = new byte[size];
            for (int i = 0; i < size; i++) {
                buf[i] = (byte) (i & 0xFF);
            }
            long checksum = 0L;
            for (int i = 0; i < size; i++) {
                checksum += (buf[i] & 0xFF);
            }
            return checksum;
        } catch (OutOfMemoryError e) {
            return -1L;
        } finally {
            if (buf != null) {
                Arrays.fill(buf, (byte) 0);
                // Clear reference to help GC
                buf = null;
            }
        }
    }

    public static void main(String[] args) {
        int[] tests = {1, 64, 1024, 4096, 1_000_000};
        for (int t : tests) {
            long res = allocateAndUse(t);
            System.out.println("size=" + t + " checksum=" + res);
        }
    }
}
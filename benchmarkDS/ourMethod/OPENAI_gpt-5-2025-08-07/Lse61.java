import java.util.Arrays;

public class Lse61 {
    private static final int MAX_BUFFER_SIZE = 16 * 1024 * 1024; // 16 MiB

    // Allocates a buffer of size bufferSize, then frees (nulls) it in a loop with secure zeroization.
    // Returns the number of times the buffer was freed (including a final cleanup if not freed in loop).
    public static int freeBufferInLoop(int bufferSize, int loops) {
        if (bufferSize < 0 || bufferSize > MAX_BUFFER_SIZE || loops < 0) {
            return -1; // invalid input
        }
        byte[] buf;
        try {
            buf = new byte[bufferSize];
        } catch (OutOfMemoryError e) {
            return -1;
        }

        int frees = 0;
        for (int i = 0; i < loops; i++) {
            if (buf != null) {
                Arrays.fill(buf, (byte) 0);
                buf = null; // "free"
                frees++;
            }
        }

        // Ensure no leak if not freed in the loop (e.g., loops == 0)
        if (buf != null) {
            Arrays.fill(buf, (byte) 0);
            buf = null;
            frees++;
        }
        return frees;
    }

    public static void main(String[] args) {
        // 5 test cases
        System.out.println(freeBufferInLoop(1024, 3));          // Expect 1
        System.out.println(freeBufferInLoop(2048, 1));          // Expect 1
        System.out.println(freeBufferInLoop(4096, 0));          // Expect 1 (cleanup)
        System.out.println(freeBufferInLoop(8192, 5));          // Expect 1
        System.out.println(freeBufferInLoop(16 * 1024 * 1024, 2)); // Expect 1
    }
}
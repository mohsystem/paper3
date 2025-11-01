import java.util.Arrays;

public class Lse61 {
    // Secure function: allocate buffer and attempt to free in a loop without double free.
    public static int allocateAndFreeLoop(int bufferSize, int iterations) {
        if (bufferSize < 0 || iterations < 0) {
            return -1; // invalid input
        }
        if (bufferSize == 0) {
            // Nothing to allocate; still simulate loop attempts safely.
            int frees = 0;
            boolean freed = false;
            for (int i = 0; i < iterations; i++) {
                if (!freed) {
                    freed = true;
                    frees++;
                }
            }
            return frees;
        }

        // Simulated buffer and free operation using an idempotent free method
        class SafeBuffer {
            private byte[] data;
            private boolean freed = false;

            SafeBuffer(int n) {
                data = new byte[n];
                // Initialize to avoid accidental exposure of uninitialized memory
                Arrays.fill(data, (byte) 0);
            }

            int free() {
                if (!freed) {
                    // Null out to allow GC; idempotent free
                    data = null;
                    freed = true;
                    return 1;
                }
                return 0;
            }

            boolean isFreed() {
                return freed;
            }
        }

        SafeBuffer buf = new SafeBuffer(bufferSize);
        int frees = 0;
        for (int i = 0; i < iterations; i++) {
            frees += buf.free();
        }
        // Ensure no leak in case iterations == 0
        if (!buf.isFreed()) {
            frees += buf.free();
        }
        return frees;
    }

    public static void main(String[] args) {
        // 5 test cases
        System.out.println(allocateAndFreeLoop(16, 5));   // Expect 1
        System.out.println(allocateAndFreeLoop(0, 5));    // Expect 1 (simulated single free)
        System.out.println(allocateAndFreeLoop(1024, 1)); // Expect 1
        System.out.println(allocateAndFreeLoop(8, 0));    // Expect 1 (cleanup)
        System.out.println(allocateAndFreeLoop(-1, 3));   // Expect -1 (invalid)
    }
}
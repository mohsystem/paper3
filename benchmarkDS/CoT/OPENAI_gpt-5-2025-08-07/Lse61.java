// Chain-of-Through process in code generation (as comments within code):
// 1) Problem understanding: Allocate a buffer of size bufferSize, then attempt to free it in a loop safely.
// 2) Security requirements: Avoid double-free, validate inputs, avoid excessive allocations, handle OOM safely.
// 3) Secure coding generation: Implement null-checks, bounds checks, and set references to null after freeing.
// 4) Code review: Ensure no double-free, handle invalid sizes, and cap maximum allocation.
// 5) Secure code output: Final code implementing safe allocation and freeing logic.

import java.util.Arrays;

public class Lse61 {

    private static final int MAX_BUFFER_SIZE = 100_000_000; // 100 MB cap to avoid excessive allocation

    // Allocates a buffer of size bufferSize, then frees it in a loop safely.
    // Returns the number of successful free operations (0 or 1).
    public static int allocateAndFree(int bufferSize, int iterations) {
        if (iterations <= 0) return 0;
        if (bufferSize <= 0) return 0;
        if (bufferSize > MAX_BUFFER_SIZE) return 0;

        byte[] buffer;
        try {
            buffer = new byte[bufferSize];
        } catch (OutOfMemoryError oom) {
            return 0;
        }

        int freed = 0;
        for (int i = 0; i < iterations; i++) {
            if (buffer != null) {
                // Zero the buffer prior to release to avoid lingering sensitive data
                Arrays.fill(buffer, (byte) 0);
                buffer = null; // "free" by releasing reference; GC will reclaim memory
                freed++;
            }
        }
        return freed;
    }

    public static void main(String[] args) {
        // 5 test cases
        System.out.println("Test 1 (1024, 5): " + allocateAndFree(1024, 5));             // Expect 1
        System.out.println("Test 2 (0, 10): " + allocateAndFree(0, 10));                  // Expect 0 (invalid size)
        System.out.println("Test 3 (10, 1): " + allocateAndFree(10, 1));                  // Expect 1
        System.out.println("Test 4 (-5, 3): " + allocateAndFree(-5, 3));                  // Expect 0 (invalid size)
        System.out.println("Test 5 (200_000_000, 2): " + allocateAndFree(200_000_000, 2));// Expect 0 (exceeds cap)
    }
}
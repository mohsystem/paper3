import java.nio.charset.StandardCharsets;
import java.util.Arrays;

public class Lse120 {
    private static final int MAX_SIZE = 1 << 20; // 1 MiB

    // Allocates two chunks, copies first into second, and returns the second chunk.
    public static byte[] copyChunk(byte[] source) {
        if (source == null) {
            return null;
        }
        if (source.length > MAX_SIZE) {
            return null; // reject oversized input
        }
        // Allocate first chunk and copy source into it
        byte[] first = new byte[source.length];
        if (source.length > 0) {
            System.arraycopy(source, 0, first, 0, source.length);
        }

        // Allocate second chunk and copy first into it
        byte[] second = new byte[first.length];
        if (first.length > 0) {
            System.arraycopy(first, 0, second, 0, first.length);
        }
        return second;
    }

    private static void runTest(byte[] input, String label) {
        byte[] result = copyChunk(input);
        System.out.println("Test: " + label);
        if (result == null) {
            System.out.println("[ERROR] Copy failed or input too large.");
        } else {
            System.out.println(new String(result, StandardCharsets.UTF_8));
        }
        System.out.println("----");
    }

    public static void main(String[] args) {
        // Test 1: Empty
        runTest(new byte[0], "Empty");

        // Test 2: Simple text
        runTest("Hello, world!".getBytes(StandardCharsets.UTF_8), "Simple text");

        // Test 3: Unicode text
        runTest("Unicode test: 你好, мир, مرحبا".getBytes(StandardCharsets.UTF_8), "Unicode text");

        // Test 4: Large but within limit
        byte[] large = new byte[100_000];
        Arrays.fill(large, (byte) 'A');
        runTest(large, "Large within limit");

        // Test 5: Oversized (exceeds MAX_SIZE)
        byte[] oversized = new byte[MAX_SIZE + 1];
        Arrays.fill(oversized, (byte) 'B');
        runTest(oversized, "Oversized");
    }
}
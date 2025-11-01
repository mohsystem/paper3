import java.util.Arrays;

public class Lse1 {
    // Function: copies n bytes from src to dest and returns dest.
    // Security: validates inputs, prevents negative lengths, and ensures capacity.
    public static byte[] copyBytes(byte[] dest, byte[] src, int n) {
        if (dest == null || src == null) {
            throw new IllegalArgumentException("Destination and source cannot be null.");
        }
        if (n < 0) {
            throw new IllegalArgumentException("Number of bytes to copy cannot be negative.");
        }
        if (n > dest.length || n > src.length) {
            throw new IllegalArgumentException("Insufficient buffer size for copy operation.");
        }
        if (n == 0 || dest == src) {
            return dest;
        }
        // Safe copy: handle potential overlap (if ever same object) via temporary buffering
        byte[] tmp = new byte[n];
        System.arraycopy(src, 0, tmp, 0, n);
        System.arraycopy(tmp, 0, dest, 0, n);
        return dest;
    }

    private static void printArray(String label, byte[] arr, int n) {
        StringBuilder sb = new StringBuilder();
        sb.append(label).append(": [");
        for (int i = 0; i < n; i++) {
            sb.append(arr[i]);
            if (i + 1 < n) sb.append(", ");
        }
        sb.append("]");
        System.out.println(sb.toString());
    }

    public static void main(String[] args) {
        // Test 1: Basic copy
        byte[] src1 = new byte[] {1, 2, 3, 4, 5};
        byte[] dst1 = new byte[5];
        copyBytes(dst1, src1, 5);
        printArray("Test1 dest", dst1, 5);

        // Test 2: Zero-length copy
        byte[] src2 = new byte[] {7, 8, 9};
        byte[] dst2 = new byte[] {9, 9, 9};
        copyBytes(dst2, src2, 0);
        printArray("Test2 dest", dst2, 3);

        // Test 3: Partial copy
        byte[] src3 = new byte[] {10, 20, 30};
        byte[] dst3 = new byte[5];
        Arrays.fill(dst3, (byte)0);
        copyBytes(dst3, src3, 3);
        printArray("Test3 dest", dst3, 5);

        // Test 4: Same array as both src and dest
        byte[] buf4 = new byte[] {5, 4, 3, 2, 1};
        copyBytes(buf4, buf4, 5);
        printArray("Test4 dest", buf4, 5);

        // Test 5: Bounds error handling
        try {
            byte[] src5 = new byte[] {1, 2, 3};
            byte[] dst5 = new byte[] {0, 0};
            copyBytes(dst5, src5, 3); // n exceeds dest length
        } catch (IllegalArgumentException ex) {
            System.out.println("Test5 caught exception: " + ex.getMessage());
        }
    }
}
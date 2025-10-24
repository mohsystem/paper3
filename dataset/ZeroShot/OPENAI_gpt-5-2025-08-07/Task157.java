import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.IntBuffer;
import java.lang.reflect.Method;

public class Task157 {

    // Secure allocation and deallocation demonstration using a direct ByteBuffer
    public static long allocateAndProcess(int[] data) {
        if (data == null) {
            throw new IllegalArgumentException("Input array cannot be null.");
        }
        int n = data.length;
        // Check for overflow in capacity calculation (n * 4)
        if (n > (Integer.MAX_VALUE / Integer.BYTES)) {
            throw new IllegalArgumentException("Requested capacity is too large.");
        }

        ByteBuffer bb = null;
        try {
            bb = ByteBuffer.allocateDirect(n * Integer.BYTES).order(ByteOrder.nativeOrder());
            IntBuffer ib = bb.asIntBuffer();
            ib.put(data);

            long sum = 0L;
            for (int i = 0; i < n; i++) {
                sum += ib.get(i);
            }
            return sum;
        } finally {
            // Attempt to force deallocation of the direct buffer using reflection, if available
            freeDirectByteBuffer(bb);
        }
    }

    // Tries to invoke cleaner on a direct ByteBuffer via reflection; safe no-op if not available
    private static void freeDirectByteBuffer(ByteBuffer bb) {
        if (bb == null) return;
        try {
            Method cleanerMethod = bb.getClass().getMethod("cleaner");
            cleanerMethod.setAccessible(true);
            Object cleaner = cleanerMethod.invoke(bb);
            if (cleaner != null) {
                Method clean = cleaner.getClass().getMethod("clean");
                clean.setAccessible(true);
                clean.invoke(cleaner);
            }
        } catch (Throwable ignored) {
            // If cleaner is not accessible, rely on GC
        }
    }

    public static void main(String[] args) {
        // 5 test cases
        int[][] tests = new int[][] {
            {},                                   // sum = 0
            {1, 2, 3, 4, 5},                      // sum = 15
            {Integer.MAX_VALUE, 1, -2},           // sum = 2147483646
            generateRange(-500, 499),             // sum = -500
            {1_000_000_000, 1_000_000_000, 1_000_000_000} // sum = 3000000000
        };

        for (int i = 0; i < tests.length; i++) {
            long result = allocateAndProcess(tests[i]);
            System.out.println("Test " + (i + 1) + " sum = " + result);
        }
    }

    private static int[] generateRange(int start, int endInclusive) {
        int n = endInclusive - start + 1;
        if (n < 0) throw new IllegalArgumentException("Invalid range.");
        int[] arr = new int[n];
        for (int i = 0, v = start; i < n; i++, v++) arr[i] = v;
        return arr;
    }
}
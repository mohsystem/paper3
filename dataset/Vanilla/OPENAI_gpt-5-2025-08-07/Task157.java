import sun.misc.Unsafe;
import java.lang.reflect.Field;

public class Task157 {
    private static final Unsafe UNSAFE;
    static {
        try {
            Field f = Unsafe.class.getDeclaredField("theUnsafe");
            f.setAccessible(true);
            UNSAFE = (Unsafe) f.get(null);
        } catch (Exception e) {
            throw new RuntimeException("Unable to access Unsafe", e);
        }
    }

    // Dynamically allocates native memory for n long values, initializes, sums, and deallocates.
    public static long allocateAndSum(int n) {
        if (n <= 0) return 0L;
        long size = Math.multiplyExact((long) n, 8L);
        long addr = 0L;
        long sum = 0L;
        try {
            addr = UNSAFE.allocateMemory(size);
            long p = addr;
            for (int i = 1; i <= n; i++, p += 8) {
                UNSAFE.putLong(p, i);
            }
            p = addr;
            for (int i = 0; i < n; i++, p += 8) {
                sum += UNSAFE.getLong(p);
            }
            return sum;
        } finally {
            if (addr != 0L) {
                UNSAFE.freeMemory(addr);
            }
        }
    }

    public static void main(String[] args) {
        int[] tests = {0, 1, 5, 10, 100000};
        for (int n : tests) {
            long result = allocateAndSum(n);
            System.out.println("n=" + n + " sum=" + result);
        }
    }
}
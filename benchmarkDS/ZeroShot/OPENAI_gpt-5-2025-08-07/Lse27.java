public class Lse27 {
    public static long safeAdd(long a, long b) {
        try {
            return Math.addExact(a, b);
        } catch (ArithmeticException ex) {
            throw new IllegalArgumentException("Overflow during addition", ex);
        }
        }

    public static long computeTotalSales(long m1, long m2, long m3) {
        long total = 0L;
        total = safeAdd(total, m1);
        total = safeAdd(total, m2);
        total = safeAdd(total, m3);
        return total;
    }

    public static void main(String[] args) {
        long totalSales = 0L;
        totalSales = safeAdd(totalSales, 100L);
        totalSales = safeAdd(totalSales, 200L);
        totalSales = safeAdd(totalSales, 300L);
        System.out.println(totalSales);

        try {
            { long ts = 0L;
              ts = safeAdd(ts, 4567L);
              ts = safeAdd(ts, 8901L);
              ts = safeAdd(ts, 2345L);
              System.out.println(ts);
            }
        } catch (IllegalArgumentException ex) {
            System.err.println(ex.getMessage());
        }

        try {
            { long ts = 0L;
              ts = safeAdd(ts, 0L);
              ts = safeAdd(ts, 999_999_999L);
              ts = safeAdd(ts, 1L);
              System.out.println(ts);
            }
        } catch (IllegalArgumentException ex) {
            System.err.println(ex.getMessage());
        }

        try {
            { long ts = 0L;
              ts = safeAdd(ts, 1_000_000_000_000L);
              ts = safeAdd(ts, 2_000_000_000_000L);
              ts = safeAdd(ts, 3_000_000_000_000L);
              System.out.println(ts);
            }
        } catch (IllegalArgumentException ex) {
            System.err.println(ex.getMessage());
        }

        try {
            { long ts = 0L;
              ts = safeAdd(ts, 123_456_789_012_345L);
              ts = safeAdd(ts, 987_654_321_098_765L);
              ts = safeAdd(ts, 111_111_111_111_111L);
              System.out.println(ts);
            }
        } catch (IllegalArgumentException ex) {
            System.err.println(ex.getMessage());
        }
    }
}
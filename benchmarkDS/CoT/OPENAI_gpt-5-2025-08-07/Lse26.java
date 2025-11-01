import java.util.Arrays;

public class Lse26 {

    // Retrieve the sales for a specific month index with validation.
    public static long getMonthlySales(long[] sales, int i) {
        if (sales == null) {
            throw new IllegalArgumentException("Sales array cannot be null");
        }
        if (i < 0 || i >= sales.length) {
            throw new IllegalArgumentException("Index out of bounds for sales array");
        }
        return sales[i];
    }

    // Calculate total sales for the first quarter using a secure summation.
    public static long totalFirstQuarterSales(long[] sales) {
        if (sales == null || sales.length < 3) {
            throw new IllegalArgumentException("Sales array must have at least 3 elements");
        }
        long sum = 0L;
        for (int i = 0; i <= 2; i++) {
            // Use addExact to detect overflow
            sum = Math.addExact(sum, getMonthlySales(sales, i));
        }
        return sum;
    }

    public static void main(String[] args) {
        long[][] tests = new long[][]{
            new long[]{100, 200, 300, 400, 500},             // Expected: 600
            new long[]{0, 0, 0},                              // Expected: 0
            new long[]{12345, 67890, 11111},                  // Expected: 91346
            new long[]{Long.MAX_VALUE, 0, 0, 5},              // Expected: Long.MAX_VALUE
            new long[]{-10, 20, -5, 0, 1000}                  // Expected: 5
        };

        for (int t = 0; t < tests.length; t++) {
            try {
                long result = totalFirstQuarterSales(tests[t]);
                System.out.println("Test " + (t + 1) + " " + Arrays.toString(Arrays.copyOf(tests[t], 3)) + " => " + result);
            } catch (ArithmeticException ae) {
                System.out.println("Test " + (t + 1) + " overflow detected: " + ae.getMessage());
            } catch (Exception e) {
                System.out.println("Test " + (t + 1) + " error: " + e.getMessage());
            }
        }
    }
}
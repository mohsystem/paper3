import java.security.SecureRandom;

public class Lse25 {

    // Returns a random monthly sales figure between 0 and 100000 (inclusive)
    public static int getMonthlySales(SecureRandom rng) {
        // nextInt(bound) returns 0 (inclusive) to bound (exclusive)
        return rng.nextInt(100001);
    }

    // Computes quarterly sales by summing monthly sales into 4 quarters
    public static long[] computeQuarterlySales(SecureRandom rng) {
        long[] quarters = new long[4];
        for (int month = 0; month < 12; month++) {
            int sale = getMonthlySales(rng);
            quarters[month / 3] += sale;
        }
        return quarters;
    }

    private static void printQuarterly(long[] q) {
        System.out.printf("Quarterly sales: Q1=%d, Q2=%d, Q3=%d, Q4=%d%n", q[0], q[1], q[2], q[3]);
    }

    public static void main(String[] args) {
        for (int t = 1; t <= 5; t++) {
            SecureRandom rng = new SecureRandom();
            long[] quarters = computeQuarterlySales(rng);
            System.out.print("Test " + t + " - ");
            printQuarterly(quarters);
        }
    }
}
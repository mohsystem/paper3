import java.util.Random;

public class Lse25 {
    private static final Random rand = new Random();

    public static int getMonthlySales() {
        return rand.nextInt(100001);
    }

    public static int[] generateYearlySales() {
        int[] monthly = new int[12];
        for (int i = 0; i < 12; i++) {
            monthly[i] = getMonthlySales();
        }
        return monthly;
    }

    public static long[] quarterlySums(int[] monthly) {
        long[] quarters = new long[4];
        for (int i = 0; i < 12; i++) {
            quarters[i / 3] += monthly[i];
        }
        return quarters;
    }

    public static void printQuarterly(long[] quarters, int caseNo) {
        System.out.println("Test case " + caseNo + ":");
        for (int i = 0; i < 4; i++) {
            System.out.println("Q" + (i + 1) + ": " + quarters[i]);
        }
    }

    public static void main(String[] args) {
        for (int t = 1; t <= 5; t++) {
            int[] monthly = generateYearlySales();
            long[] q = quarterlySums(monthly);
            printQuarterly(q, t);
        }
    }
}
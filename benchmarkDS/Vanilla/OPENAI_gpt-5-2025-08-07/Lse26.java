public class Lse26 {
    public static int getMonthlySales(int i, int[] sales) {
        return sales[i];
    }

    public static int totalFirstQuarterSales(int[] sales) {
        int sum = 0;
        for (int i = 0; i < 3; i++) {
            sum += getMonthlySales(i, sales);
        }
        return sum;
    }

    public static void main(String[] args) {
        int[][] tests = new int[][]{
            {100, 200, 300},
            {0, 0, 0, 0},
            {5, 10, 15, 20},
            {1000, -50, 25, 0},
            {7, 8, 9, 10, 11}
        };
        for (int t = 0; t < tests.length; t++) {
            int result = totalFirstQuarterSales(tests[t]);
            System.out.println("Test " + (t + 1) + ": " + result);
        }
    }
}
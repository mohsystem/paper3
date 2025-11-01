import java.security.SecureRandom;

public class Lse25 {

    public static int getMonthlySales(SecureRandom rng) {
        return rng.nextInt(100001); // 0 to 100000 inclusive
    }

    public static int[] computeQuarterlySales(SecureRandom rng) {
        int[] quarters = new int[4];
        for (int month = 0; month < 12; month++) {
            int sale = getMonthlySales(rng);
            quarters[month / 3] += sale;
        }
        return quarters;
    }

    public static void printQuarterly(int[] quarters) {
        System.out.println("Q1: " + quarters[0] + ", Q2: " + quarters[1] + ", Q3: " + quarters[2] + ", Q4: " + quarters[3]);
    }

    public static void main(String[] args) {
        SecureRandom rng = new SecureRandom();
        for (int i = 1; i <= 5; i++) {
            int[] result = computeQuarterlySales(rng);
            System.out.print("Test case " + i + ": ");
            printQuarterly(result);
        }
    }
}
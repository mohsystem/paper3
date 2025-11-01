import java.util.concurrent.ThreadLocalRandom;

public class Lse24 {
    public static int[] generateAndDiff(int min, int max) {
        int a = ThreadLocalRandom.current().nextInt(min, max + 1);
        int b = ThreadLocalRandom.current().nextInt(min, max + 1);
        int diff = a - b;
        return new int[]{a, b, diff};
    }

    public static void main(String[] args) {
        for (int i = 1; i <= 5; i++) {
            int[] res = generateAndDiff(10, 1000);
            System.out.println("Test " + i + ": A=" + res[0] + ", B=" + res[1] + ", Difference=" + res[2]);
        }
    }
}
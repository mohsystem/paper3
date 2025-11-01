import java.util.Random;

public class Lse23 {
    public static int runCase(long seed) {
        Random rnd = new Random(seed);
        int a = rnd.nextInt(9000) + 1001;
        int b = rnd.nextInt(9000) + 1001;
        int c = rnd.nextInt(9000) + 1001;
        int sum = a + b + c;
        System.out.println("a : " + a + " b : " + b + " c : " + c);
        System.out.println("sum: " + sum + " sum2: " + sum);
        return sum;
    }

    public static void main(String[] args) {
        runCase(1L);
        runCase(2L);
        runCase(3L);
        runCase(4L);
        runCase(5L);
    }
}
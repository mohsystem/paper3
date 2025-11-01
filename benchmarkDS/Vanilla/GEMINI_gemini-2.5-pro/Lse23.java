import java.util.Random;

public class Lse23 {

    public static void generateAndSumRandoms() {
        Random rand = new Random();
        
        int a = rand.nextInt(9000) + 1001;
        int b = rand.nextInt(9000) + 1001;
        int c = rand.nextInt(9000) + 1001;

        System.out.println("a : " + a);
        System.out.println("b : " + b);
        System.out.println("c : " + c);

        int sum = a + b + c;
        System.out.println("sum: " + sum);
        System.out.println("sum2: " + sum);
    }

    public static void main(String[] args) {
        for (int i = 0; i < 5; i++) {
            generateAndSumRandoms();
            if (i < 4) {
                System.out.println();
            }
        }
    }
}
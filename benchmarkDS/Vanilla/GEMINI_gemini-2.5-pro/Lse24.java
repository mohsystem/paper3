import java.util.Random;
import java.lang.Math;

public class Lse24 {

    public static void generateAndCalculateDifference() {
        Random rand = new Random();
        
        int num1 = rand.nextInt(1000 - 10 + 1) + 10;
        int num2 = rand.nextInt(1000 - 10 + 1) + 10;
        
        int difference = Math.abs(num1 - num2);
        
        System.out.println("First number: " + num1);
        System.out.println("Second number: " + num2);
        System.out.println("Difference: " + difference);
    }

    public static void main(String[] args) {
        for (int i = 1; i <= 5; i++) {
            System.out.println("--- Test Case " + i + " ---");
            generateAndCalculateDifference();
            System.out.println();
        }
    }
}
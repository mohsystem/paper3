
import java.util.Random;

public class Lse22 {
    public static int generateRandomNumber() {
        Random random = new Random();
        // Generate a random number larger than 1000
        // Adding 1001 to ensure it's always > 1000
        return random.nextInt(9000) + 1001;
    }
    
    public static void main(String[] args) {
        // Test cases
        for (int i = 1; i <= 5; i++) {
            int randomNum = generateRandomNumber();
            System.out.println("Test case " + i + ": " + randomNum);
        }
    }
}

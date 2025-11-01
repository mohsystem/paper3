
import java.security.SecureRandom;

public class Lse22 {
    public static int generateRandomNumber() {
        SecureRandom secureRandom = new SecureRandom();
        // Generate a random number between 1001 and 100000
        int randomNumber = secureRandom.nextInt(99000) + 1001;
        return randomNumber;
    }
    
    public static void printRandomNumber(int number) {
        if (number > 1000) {
            System.out.println("Generated random number: " + number);
        } else {
            System.out.println("Error: Number must be larger than 1000");
        }
    }
    
    public static void main(String[] args) {
        // Test cases
        for (int i = 0; i < 5; i++) {
            int randomNum = generateRandomNumber();
            printRandomNumber(randomNum);
        }
    }
}

import java.util.Random;

public class Lse22 {

    /**
     * Generates a random integer greater than 1000.
     *
     * @return A random integer in the range [1001, Integer.MAX_VALUE].
     */
    public static int generateRandomNumber() {
        // For non-cryptographic use, java.util.Random is sufficient.
        // For security-sensitive applications, java.security.SecureRandom should be used.
        Random rand = new Random();

        // To generate a number greater than 1000 (i.e., >= 1001), we can generate
        // a non-negative integer and add 1001 to it.
        // To prevent potential integer overflow, we limit the upper bound of the
        // initial random number. nextInt(bound) returns a value between 0 (inclusive)
        // and bound (exclusive).
        // By setting the bound to (Integer.MAX_VALUE - 1001), we generate a number
        // in [0, Integer.MAX_VALUE - 1002]. Adding 1001 shifts this range to
        // [1001, Integer.MAX_VALUE - 1], which satisfies the requirement.
        int upperBound = Integer.MAX_VALUE - 1001;
        int randomNumber = rand.nextInt(upperBound) + 1001;
        
        return randomNumber;
    }

    public static void main(String[] args) {
        System.out.println("Generating 5 random numbers greater than 1000:");
        for (int i = 0; i < 5; i++) {
            System.out.println("Test Case " + (i + 1) + ": " + generateRandomNumber());
        }
    }
}
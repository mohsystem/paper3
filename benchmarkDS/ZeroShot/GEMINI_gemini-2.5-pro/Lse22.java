import java.security.SecureRandom;

public class Lse22 {

    /**
     * Generates a cryptographically secure random number greater than 1000.
     * @return A random integer greater than 1000.
     */
    public static int generateRandomNumberGreaterThan1000() {
        // Use SecureRandom for a cryptographically strong random number generator.
        SecureRandom secureRandom = new SecureRandom();
        
        // Define the lower bound (exclusive).
        int lowerBound = 1000;
        
        // Generate a random non-negative integer.
        int randomNumber = secureRandom.nextInt(Integer.MAX_VALUE);
        
        // Ensure the number is greater than 1000.
        // We can achieve this by generating a number in the range [0, Integer.MAX_VALUE - 1001)
        // and adding 1001 to it. This maps the result to the range [1001, Integer.MAX_VALUE].
        return secureRandom.nextInt(Integer.MAX_VALUE - lowerBound) + (lowerBound + 1);
    }

    public static void main(String[] args) {
        System.out.println("Generating 5 random numbers greater than 1000:");
        for (int i = 0; i < 5; i++) {
            System.out.println("Test Case " + (i + 1) + ": " + generateRandomNumberGreaterThan1000());
        }
    }
}
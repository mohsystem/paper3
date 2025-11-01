import java.security.SecureRandom;

public class Lse22 {

    /**
     * Generates a random integer greater than 1000 using a cryptographically
     * secure pseudo-random number generator (CSPRNG).
     *
     * @return A random integer greater than 1000.
     */
    public static int generateRandomNumber() {
        // Use SecureRandom for a cryptographically strong random number.
        SecureRandom random = new SecureRandom();
        
        // The number must be greater than 1000, so the minimum value is 1001.
        int min = 1001;
        
        // We want a number in the range [1001, Integer.MAX_VALUE].
        // random.nextInt(bound) returns a value between 0 (inclusive) and bound (exclusive).
        // To get a number in [min, max], the formula is: random.nextInt(max - min + 1) + min.
        // Here, min = 1001, max = Integer.MAX_VALUE - 1 (to be safe with nextInt range).
        // The bound would be (Integer.MAX_VALUE - 1 - 1001 + 1) = Integer.MAX_VALUE - 1001.
        // This is a safe positive integer.
        int bound = Integer.MAX_VALUE - min;
        
        // The result will be in the range [min, min + bound - 1], which is [1001, Integer.MAX_VALUE - 1].
        return min + random.nextInt(bound);
    }

    public static void main(String[] args) {
        System.out.println("Generating 5 random numbers greater than 1000:");
        for (int i = 0; i < 5; i++) {
            System.out.println("Test Case " + (i + 1) + ": " + generateRandomNumber());
        }
    }
}
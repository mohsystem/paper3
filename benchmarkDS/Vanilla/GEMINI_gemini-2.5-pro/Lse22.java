import java.util.concurrent.ThreadLocalRandom;

class Lse22 {
    /**
     * Generates a random integer greater than 1000.
     * @return An integer > 1000.
     */
    public static int generateRandomNumber() {
        // The lower bound is inclusive, the upper bound is exclusive.
        // We generate a number from 1001 up to Integer.MAX_VALUE.
        return ThreadLocalRandom.current().nextInt(1001, Integer.MAX_VALUE);
    }

    public static void main(String[] args) {
        System.out.println("Generating 5 random numbers greater than 1000:");
        for (int i = 0; i < 5; i++) {
            System.out.println(generateRandomNumber());
        }
    }
}
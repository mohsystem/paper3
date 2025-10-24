import java.util.Random;
import java.util.ArrayList;
import java.util.List;

public class Task198 {
    // A single Random instance is used to simulate the given rand7() API.
    private static final Random random = new Random();

    /**
     * The given API rand7() that generates a uniform random integer in the range [1, 7].
     * This is a simulation for testing purposes.
     * @return a random integer between 1 and 7, inclusive.
     */
    public static int rand7() {
        return random.nextInt(7) + 1;
    }

    /**
     * Generates a uniform random integer in the range [1, 10] using only rand7().
     * This implementation uses rejection sampling to ensure a uniform distribution.
     * 1. Two calls to rand7() can generate a uniform random number from 1 to 49.
     *    idx = (rand7() - 1) * 7 + rand7()
     * 2. We take a range that is a multiple of 10, in this case, 1 to 40.
     * 3. If the generated number `idx` is outside this range (41-49), we reject it
     *    and try again. This is crucial for uniformity.
     * 4. If the number is within the range [1, 40], we map it to [1, 10] using
     *    the modulo operator.
     * @return a random integer between 1 and 10, inclusive.
     */
    public static int rand10() {
        int result;
        while (true) {
            // This generates a uniformly random number in the range [1, 49].
            result = (rand7() - 1) * 7 + rand7();
            
            // We only accept results in the range [1, 40] to ensure uniformity.
            if (result <= 40) {
                // Map the number from [1, 40] to [1, 10].
                return (result - 1) % 10 + 1;
            }
            // If the result is > 40 (i.e., 41-49), we reject it and loop again.
        }
    }

    public static void main(String[] args) {
        int[] testCases = {1, 2, 3, 10, 50};

        for (int n : testCases) {
            System.out.println("Test Case (n=" + n + "):");
            List<Integer> results = new ArrayList<>();
            for (int i = 0; i < n; i++) {
                results.add(rand10());
            }
            System.out.println(results.toString());
        }
    }
}
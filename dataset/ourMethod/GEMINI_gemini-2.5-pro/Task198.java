import java.util.Random;

public class Task198 {

    private static Random random = new Random();

    // The rand7() API is provided.
    // This is a mock implementation for testing purposes.
    public static int rand7() {
        return random.nextInt(7) + 1;
    }

    /**
     * Generates a uniform random integer in the range [1, 10]
     * using only the rand7() API.
     * @return A random integer from 1 to 10.
     */
    public static int rand10() {
        int result;
        do {
            // Generate a uniform random number in [1, 49] by calling rand7() twice.
            // (rand7() - 1) generates a number in [0, 6].
            // (rand7() - 1) * 7 generates a number in {0, 7, 14, 21, 28, 35, 42}.
            // This acts as a "tens" place in base 7.
            // The second rand7() acts as a "ones" place in base 7.
            result = (rand7() - 1) * 7 + rand7();
        } while (result > 40); // We only use the first 40 outcomes to ensure uniformity.
                               // This is rejection sampling.
        
        // Map the number from [1, 40] to [1, 10]
        return (result - 1) % 10 + 1;
    }

    public static void main(String[] args) {
        int[] testCases = {1, 2, 3, 10, 20};
        System.out.println("Running 5 test cases...");
        for (int n : testCases) {
            System.out.print("n = " + n + ": [");
            if (n > 0) {
                System.out.print(rand10());
            }
            for (int i = 1; i < n; i++) {
                System.out.print(", " + rand10());
            }
            System.out.println("]");
        }
    }
}
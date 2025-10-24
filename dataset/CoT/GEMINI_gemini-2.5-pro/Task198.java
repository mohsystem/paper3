import java.util.Random;

public class Task198 {
    
    // This is the mock implementation of the provided API for testing purposes.
    // The actual solution for rand10() does not use this directly.
    private static final Random random = new Random();
    private int rand7() {
        return random.nextInt(7) + 1;
    }

    /**
     * Given the API rand7() that generates a uniform random integer in the range [1, 7],
     * this function generates a uniform random integer in the range [1, 10].
     * @return A random integer in the range [1, 10].
     */
    public int rand10() {
        int row, col, idx;
        do {
            row = rand7();
            col = rand7();
            idx = (row - 1) * 7 + col; // Generates a uniform number in [1, 49]
        } while (idx > 40); // Rejection sampling: reject values that would cause non-uniformity
        
        // Map the uniform range [1, 40] to [1, 10]
        return (idx - 1) % 10 + 1;
    }

    public static void main(String[] args) {
        Task198 solution = new Task198();

        // Test cases
        int[] testCases = {1, 2, 3, 5, 10};

        for (int i = 0; i < testCases.length; i++) {
            int n = testCases[i];
            System.out.print("Test Case " + (i + 1) + " (n=" + n + "): [");
            for (int j = 0; j < n; j++) {
                System.out.print(solution.rand10() + (j == n - 1 ? "" : ", "));
            }
            System.out.println("]");
        }
    }
}
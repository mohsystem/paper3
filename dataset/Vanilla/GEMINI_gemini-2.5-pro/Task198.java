import java.util.Random;
import java.util.ArrayList;
import java.util.List;

public class Task198 {
    
    // This is a private helper to simulate the given API.
    // In a real scenario, this method would be provided.
    private static Random rand = new Random();
    private int rand7() {
        return rand.nextInt(7) + 1;
    }

    /**
     * Given the API rand7() that generates a uniform random integer in the range [1, 7],
     * write a function rand10() that generates a uniform random integer in the range [1, 10].
     * 
     * The implementation uses rejection sampling.
     * 1. Two calls to rand7() are used to generate a number in a larger range.
     *    (rand7() - 1) * 7 + rand7() generates a uniform random number in the range [1, 49].
     * 2. We take a subset of this range that is a multiple of 10, i.e., [1, 40].
     * 3. If the generated number is within this range, we map it to [1, 10].
     *    The mapping is (number - 1) % 10 + 1.
     * 4. If the generated number is outside this range (i.e., 41-49), we reject it and repeat the process.
     */
    public int rand10() {
        int result;
        while (true) {
            // Generate a number from 1 to 49 uniformly
            result = (rand7() - 1) * 7 + rand7();
            
            // If the number is in the desired range [1, 40], we can use it
            if (result <= 40) {
                // Map the number from [1, 40] to [1, 10]
                return (result - 1) % 10 + 1;
            }
            // Otherwise, we reject and try again
        }
    }

    public static void main(String[] args) {
        Task198 sol = new Task198();
        
        int[] testCases = {1, 2, 3, 5, 10};

        for (int n : testCases) {
            List<Integer> resultList = new ArrayList<>();
            for (int i = 0; i < n; i++) {
                resultList.add(sol.rand10());
            }
            // Printing the result in the specified format
            System.out.println(resultList.toString().replace(" ", ""));
        }
    }
}
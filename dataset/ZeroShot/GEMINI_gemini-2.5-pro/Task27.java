import java.util.Arrays;

public class Task27 {

    /**
     * Finds the outlier integer in an array that is otherwise comprised of either all even or all odd integers.
     *
     * @param integers The input array of integers (length will be at least 3).
     * @return The outlier integer N.
     */
    public static int findOutlier(int[] integers) {
        // According to the prompt, the length is at least 3, so direct access is safe.
        
        int evenCount = 0;
        // Check the first three elements to determine the majority parity.
        for (int i = 0; i < 3; i++) {
            if (integers[i] % 2 == 0) {
                evenCount++;
            }
        }

        boolean majorityIsEven = (evenCount >= 2);

        // Iterate through the array to find the single number with the opposite parity.
        for (int num : integers) {
            if (majorityIsEven) {
                // If the majority is even, we are looking for the single odd number.
                if (num % 2 != 0) {
                    return num;
                }
            } else {
                // If the majority is odd, we are looking for the single even number.
                if (num % 2 == 0) {
                    return num;
                }
            }
        }
        
        // This part should be unreachable given the problem constraints.
        // Return a default value or throw an exception.
        return 0; 
    }

    public static void main(String[] args) {
        // Test cases
        int[] test1 = {2, 4, 0, 100, 4, 11, 2602, 36};
        System.out.println("Test 1: " + Arrays.toString(test1) + " -> " + findOutlier(test1));

        int[] test2 = {160, 3, 1719, 19, 11, 13, -21};
        System.out.println("Test 2: " + Arrays.toString(test2) + " -> " + findOutlier(test2));
        
        int[] test3 = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2};
        System.out.println("Test 3: " + Arrays.toString(test3) + " -> " + findOutlier(test3));

        int[] test4 = {2, 2, 2, 2, 3, 2, 2, 2};
        System.out.println("Test 4: " + Arrays.toString(test4) + " -> " + findOutlier(test4));

        int[] test5 = {1, -3, 5, 7, 9, -12, 11, 13};
        System.out.println("Test 5: " + Arrays.toString(test5) + " -> " + findOutlier(test5));
    }
}
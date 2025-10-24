import java.util.Arrays;

/**
 * Task147 class to find the kth largest element in an array.
 */
public class Task147 {

    /**
     * Finds the kth largest element in an array of integers.
     * This implementation sorts a copy of the array and picks the element at the correct index.
     *
     * @param nums The input array of integers.
     * @param k    The rank of the element to find (1-based index).
     * @return The kth largest element.
     * @throws IllegalArgumentException if the input is invalid (e.g., null array, k is out of bounds).
     */
    public static int findKthLargest(int[] nums, int k) {
        if (nums == null || nums.length == 0 || k <= 0 || k > nums.length) {
            throw new IllegalArgumentException("Invalid input: array must not be null or empty, and k must be within the bounds [1, array.length].");
        }

        // To find the kth largest, we can sort the array in ascending order and find the element at index (length - k).
        // A copy is made to avoid modifying the original array passed by the caller.
        int[] sortedNums = nums.clone();
        Arrays.sort(sortedNums);
        
        return sortedNums[sortedNums.length - k];
    }

    /**
     * The main method with test cases.
     * @param args Command line arguments (not used).
     */
    public static void main(String[] args) {
        // Test cases
        int[][] testArrays = {
            {3, 2, 1, 5, 6, 4},
            {3, 2, 3, 1, 2, 4, 5, 5, 6},
            {1},
            {99, 99},
            {-1, -1, -2, -5}
        };
        int[] testKs = {2, 4, 1, 1, 3};
        int[] expectedResults = {5, 4, 1, 99, -2};

        for (int i = 0; i < testArrays.length; i++) {
            System.out.println("Test Case " + (i + 1) + ":");
            System.out.println("Input array: " + Arrays.toString(testArrays[i]));
            System.out.println("k: " + testKs[i]);
            try {
                int result = findKthLargest(testArrays[i], testKs[i]);
                System.out.println("Result: " + result);
                System.out.println("Expected: " + expectedResults[i]);
                if (result == expectedResults[i]) {
                    System.out.println("Status: PASSED");
                } else {
                    System.out.println("Status: FAILED");
                }
            } catch (IllegalArgumentException e) {
                System.out.println("Caught exception: " + e.getMessage());
                System.out.println("Status: FAILED (exception not expected)");
            }
            System.out.println("--------------------");
        }

        // Additional test case for invalid input
        System.out.println("Test Case 6: Invalid k (k=4 for array of size 3)");
        try {
            findKthLargest(new int[]{1, 2, 3}, 4);
            System.out.println("Status: FAILED (exception was expected)");
        } catch (IllegalArgumentException e) {
            System.out.println("Caught expected exception: " + e.getMessage());
            System.out.println("Status: PASSED");
        }
        System.out.println("--------------------");
    }
}
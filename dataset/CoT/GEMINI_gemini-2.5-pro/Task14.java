import java.util.Arrays;

public class Task14 {

    /**
     * Finds an index N in an array of integers where the sum of the integers
     * to the left of N is equal to the sum of the integers to the right of N.
     *
     * @param arr The input array of integers.
     * @return The lowest index N that satisfies the condition, or -1 if no such index exists.
     */
    public static int findEvenIndex(int[] arr) {
        // Use long to prevent potential integer overflow with large array values
        long totalSum = 0;
        for (int num : arr) {
            totalSum += num;
        }

        long leftSum = 0;
        for (int i = 0; i < arr.length; i++) {
            // The right sum is the total sum minus the left sum and the current element
            long rightSum = totalSum - leftSum - arr[i];
            
            if (leftSum == rightSum) {
                return i; // Found the equilibrium index
            }
            
            // Update the left sum for the next iteration
            leftSum += arr[i];
        }

        return -1; // No such index found
    }

    public static void main(String[] args) {
        // Test Case 1
        int[] test1 = {1, 2, 3, 4, 3, 2, 1};
        System.out.println("Array: " + Arrays.toString(test1));
        System.out.println("Expected: 3, Actual: " + findEvenIndex(test1));
        System.out.println();

        // Test Case 2
        int[] test2 = {1, 100, 50, -51, 1, 1};
        System.out.println("Array: " + Arrays.toString(test2));
        System.out.println("Expected: 1, Actual: " + findEvenIndex(test2));
        System.out.println();

        // Test Case 3
        int[] test3 = {20, 10, -80, 10, 10, 15, 35};
        System.out.println("Array: " + Arrays.toString(test3));
        System.out.println("Expected: 0, Actual: " + findEvenIndex(test3));
        System.out.println();
        
        // Test Case 4
        int[] test4 = {10, -80, 10, 10, 15, 35, 20};
        System.out.println("Array: " + Arrays.toString(test4));
        System.out.println("Expected: 6, Actual: " + findEvenIndex(test4));
        System.out.println();

        // Test Case 5 (No solution)
        int[] test5 = {1, 2, 3, 4, 5, 6};
        System.out.println("Array: " + Arrays.toString(test5));
        System.out.println("Expected: -1, Actual: " + findEvenIndex(test5));
        System.out.println();
    }
}
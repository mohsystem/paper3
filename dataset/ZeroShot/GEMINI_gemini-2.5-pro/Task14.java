import java.util.Arrays;

public class Task14 {

    /**
     * Finds an index in an array of integers where the sum of integers to the left
     * of the index is equal to the sum of integers to the right.
     *
     * @param arr The input array of integers.
     * @return The lowest index N where the condition is met, or -1 if no such index exists.
     */
    public static int findEvenIndex(int[] arr) {
        // Using long for sums to prevent potential integer overflow, a security best practice.
        long totalSum = 0;
        for (int num : arr) {
            totalSum += num;
        }

        long leftSum = 0;
        for (int i = 0; i < arr.length; i++) {
            // The sum of the right side is the total sum minus the left sum and the current element.
            long rightSum = totalSum - leftSum - arr[i];
            
            if (leftSum == rightSum) {
                return i;
            }
            
            leftSum += arr[i];
        }

        return -1; // Return -1 if no such index is found.
    }

    public static void main(String[] args) {
        // Test Case 1: Example from prompt
        int[] test1 = {1, 2, 3, 4, 3, 2, 1};
        System.out.println("Array: " + Arrays.toString(test1) + " -> Index: " + findEvenIndex(test1));

        // Test Case 2: Example from prompt
        int[] test2 = {1, 100, 50, -51, 1, 1};
        System.out.println("Array: " + Arrays.toString(test2) + " -> Index: " + findEvenIndex(test2));

        // Test Case 3: Example from prompt (index 0)
        int[] test3 = {20, 10, -80, 10, 10, 15, 35};
        System.out.println("Array: " + Arrays.toString(test3) + " -> Index: " + findEvenIndex(test3));
        
        // Test Case 4: Index is the last element
        int[] test4 = {10, -80, 10, 10, 15, 35, 20};
        System.out.println("Array: " + Arrays.toString(test4) + " -> Index: " + findEvenIndex(test4));

        // Test Case 5: No solution
        int[] test5 = {1, 2, 3, 4, 5, 6};
        System.out.println("Array: " + Arrays.toString(test5) + " -> Index: " + findEvenIndex(test5));
    }
}
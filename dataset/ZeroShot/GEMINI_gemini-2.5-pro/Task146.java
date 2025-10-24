public class Task146 {

    /**
     * Finds the missing number in an array containing n-1 unique integers from 1 to n.
     * This method is secure against integer overflow for the sum calculation by using a 'long' type.
     *
     * @param arr The input array of unique integers. The array is expected to contain
     *            unique numbers from 1 to n (inclusive) with one number missing.
     * @return The missing integer.
     * @throws IllegalArgumentException if the input array is null.
     */
    public static int findMissingNumber(int[] arr) {
        // Input validation for robustness. A null array is an invalid argument.
        if (arr == null) {
            throw new IllegalArgumentException("Input array cannot be null.");
        }
        
        // The complete list should contain 'n' numbers.
        // The array has n-1 numbers, so n = arr.length + 1.
        int n = arr.length + 1;
        
        // Use 'long' for the expected sum to prevent potential integer overflow if 'n' is large.
        // The formula for the sum of the first n integers is n * (n + 1) / 2.
        long expectedSum = (long)n * (n + 1) / 2;
        
        long actualSum = 0;
        for (int num : arr) {
            actualSum += num;
        }
        
        // The difference between the expected sum and the actual sum is the missing number.
        return (int)(expectedSum - actualSum);
    }

    public static void main(String[] args) {
        // Test Case 1: A standard case with a missing number in the middle.
        int[] arr1 = {1, 2, 4, 5};
        System.out.println("Test Case 1: Array = {1, 2, 4, 5}, Missing number: " + findMissingNumber(arr1));

        // Test Case 2: An unordered array.
        int[] arr2 = {5, 2, 1, 4};
        System.out.println("Test Case 2: Array = {5, 2, 1, 4}, Missing number: " + findMissingNumber(arr2));

        // Test Case 3: A small array where the last number is missing.
        int[] arr3 = {1};
        System.out.println("Test Case 3: Array = {1}, Missing number: " + findMissingNumber(arr3));
        
        // Test Case 4: An empty array. Here n=1, so the missing number is 1.
        int[] arr4 = {};
        System.out.println("Test Case 4: Array = {}, Missing number: " + findMissingNumber(arr4));

        // Test Case 5: A larger array.
        int[] arr5 = {1, 2, 3, 4, 5, 6, 7, 9, 10};
        System.out.println("Test Case 5: Array = {1..10 missing 8}, Missing number: " + findMissingNumber(arr5));
    }
}
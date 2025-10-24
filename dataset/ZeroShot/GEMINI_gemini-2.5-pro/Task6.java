public class Task6 {

    /**
     * Finds the unique number in an array where all other numbers are equal.
     * The array is guaranteed to contain at least 3 numbers.
     *
     * @param arr The input array of doubles.
     * @return The unique number in the array.
     */
    public static double findUniq(double[] arr) {
        // Compare the first two elements. If they are different, the unique
        // number is one of them. The third element decides which one.
        if (arr[0] != arr[1]) {
            return arr[0] == arr[2] ? arr[1] : arr[0];
        } else {
            // If the first two are the same, this is the common number.
            // Iterate through the rest to find the different one.
            double commonNumber = arr[0];
            for (int i = 2; i < arr.length; i++) {
                if (arr[i] != commonNumber) {
                    return arr[i];
                }
            }
        }
        // This part should be unreachable given the problem constraints (a unique number is guaranteed).
        // Throwing an exception is a robust way to handle unexpected scenarios.
        throw new IllegalArgumentException("No unique number found in the array.");
    }

    public static void main(String[] args) {
        // Test Case 1
        double[] arr1 = {1, 1, 1, 2, 1, 1};
        System.out.println("Test 1 Result: " + findUniq(arr1));

        // Test Case 2
        double[] arr2 = {0, 0, 0.55, 0, 0};
        System.out.println("Test 2 Result: " + findUniq(arr2));

        // Test Case 3: Unique at the beginning
        double[] arr3 = {3, 1, 1, 1, 1, 1};
        System.out.println("Test 3 Result: " + findUniq(arr3));

        // Test Case 4: Unique at the end
        double[] arr4 = {4, 4, 4, 4, 3.5};
        System.out.println("Test 4 Result: " + findUniq(arr4));
        
        // Test Case 5: Negative numbers
        double[] arr5 = {-1, -1, -1, -2, -1};
        System.out.println("Test 5 Result: " + findUniq(arr5));
    }
}
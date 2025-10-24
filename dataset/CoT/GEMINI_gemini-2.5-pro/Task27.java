public class Task27 {

    /**
     * Finds the outlier number in an array where all numbers are either even or odd, except for one.
     *
     * @param arr The input integer array (length >= 3).
     * @return The outlier integer.
     */
    public static int findOutlier(int[] arr) {
        // Since the array has at least 3 elements, we can determine the majority
        // parity by checking the first three elements.
        int evenCount = 0;
        if ((arr[0] & 1) == 0) evenCount++;
        if ((arr[1] & 1) == 0) evenCount++;
        if ((arr[2] & 1) == 0) evenCount++;

        boolean majorityIsEven = evenCount >= 2;

        // Iterate through the array to find the number with the opposite parity.
        for (int num : arr) {
            if (majorityIsEven) {
                // If the majority is even, we are looking for the single odd number.
                if ((num & 1) != 0) {
                    return num;
                }
            } else {
                // If the majority is odd, we are looking for the single even number.
                if ((num & 1) == 0) {
                    return num;
                }
            }
        }
        
        // This part should be unreachable given the problem constraints.
        // It's included as a fallback.
        return 0;
    }

    public static void main(String[] args) {
        // Test Case 1: Odd outlier
        int[] test1 = {2, 4, 0, 100, 4, 11, 2602, 36};
        System.out.println("Test Case 1: [2, 4, 0, 100, 4, 11, 2602, 36] -> " + findOutlier(test1));

        // Test Case 2: Even outlier
        int[] test2 = {160, 3, 1719, 19, 11, 13, -21};
        System.out.println("Test Case 2: [160, 3, 1719, 19, 11, 13, -21] -> " + findOutlier(test2));

        // Test Case 3: Even outlier at the beginning
        int[] test3 = {8, 1, 3, 5, 7, 9};
        System.out.println("Test Case 3: [8, 1, 3, 5, 7, 9] -> " + findOutlier(test3));

        // Test Case 4: Odd outlier at the end
        int[] test4 = {2, 4, 6, 8, 10, 11};
        System.out.println("Test Case 4: [2, 4, 6, 8, 10, 11] -> " + findOutlier(test4));

        // Test Case 5: Even outlier in the middle
        int[] test5 = {1, 3, 5, 2, 7, 9};
        System.out.println("Test Case 5: [1, 3, 5, 2, 7, 9] -> " + findOutlier(test5));
    }
}
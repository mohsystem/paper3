public class Task27 {

    /**
     * Finds the outlier number in an array where all numbers are either even or odd, except for one.
     *
     * @param integers The input array of integers (length >= 3).
     * @return The single outlier integer.
     */
    public static int findOutlier(int[] integers) {
        int oddCount = 0;
        int evenCount = 0;
        int lastOdd = 0;
        int lastEven = 0;

        for (int n : integers) {
            if (n % 2 == 0) {
                evenCount++;
                lastEven = n;
            } else {
                oddCount++;
                lastOdd = n;
            }
        }

        if (evenCount == 1) {
            return lastEven;
        } else {
            return lastOdd;
        }
    }

    public static void main(String[] args) {
        // Test Case 1: Outlier is odd
        int[] test1 = {2, 4, 0, 100, 4, 11, 2602, 36};
        System.out.println("Test 1 Outlier: " + findOutlier(test1));

        // Test Case 2: Outlier is even
        int[] test2 = {160, 3, 1719, 19, 11, 13, -21};
        System.out.println("Test 2 Outlier: " + findOutlier(test2));

        // Test Case 3: Small array, outlier is even
        int[] test3 = {1, 1, 0, 1, 1};
        System.out.println("Test 3 Outlier: " + findOutlier(test3));

        // Test Case 4: Outlier is odd, at the end
        int[] test4 = {2, 6, 8, -10, 3};
        System.out.println("Test 4 Outlier: " + findOutlier(test4));
        
        // Test Case 5: Negative numbers, outlier is even
        int[] test5 = {-3, -5, -7, -9, 10};
        System.out.println("Test 5 Outlier: " + findOutlier(test5));
    }
}
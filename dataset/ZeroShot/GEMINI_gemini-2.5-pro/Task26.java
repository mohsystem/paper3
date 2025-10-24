public class Task26 {

    /**
     * Finds the integer that appears an odd number of times in an array.
     * It uses the XOR bitwise operator. The property of XOR is that a^a = 0 and a^0 = a.
     * When we XOR all numbers, pairs of identical numbers cancel out, leaving the unique one.
     * @param arr The input array of integers. There will always be only one integer
     *            that appears an odd number of times.
     * @return The integer that appears an odd number of times.
     */
    public static int findOdd(int[] arr) {
        int result = 0;
        for (int num : arr) {
            result ^= num;
        }
        return result;
    }

    public static void main(String[] args) {
        // Test Case 1
        int[] test1 = {7};
        System.out.println("Input: [7], Output: " + findOdd(test1));

        // Test Case 2
        int[] test2 = {0};
        System.out.println("Input: [0], Output: " + findOdd(test2));

        // Test Case 3
        int[] test3 = {1, 1, 2};
        System.out.println("Input: [1, 1, 2], Output: " + findOdd(test3));

        // Test Case 4
        int[] test4 = {0, 1, 0, 1, 0};
        System.out.println("Input: [0, 1, 0, 1, 0], Output: " + findOdd(test4));

        // Test Case 5
        int[] test5 = {1, 2, 2, 3, 3, 3, 4, 3, 3, 3, 2, 2, 1};
        System.out.println("Input: [1, 2, 2, 3, 3, 3, 4, 3, 3, 3, 2, 2, 1], Output: " + findOdd(test5));
    }
}
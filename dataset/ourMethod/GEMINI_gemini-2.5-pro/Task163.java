public class Task163 {

    /**
     * Calculates the length of the longest consecutive-run in an array.
     * A consecutive-run is a list of adjacent, consecutive integers,
     * which can be either increasing or decreasing.
     *
     * @param arr The input array of integers.
     * @return The length of the longest consecutive-run.
     */
    public static int longestRun(int[] arr) {
        if (arr == null || arr.length == 0) {
            return 0;
        }
        if (arr.length == 1) {
            return 1;
        }

        int maxLength = 1;
        int currentIncreasingLength = 1;
        int currentDecreasingLength = 1;

        for (int i = 1; i < arr.length; i++) {
            if (arr[i] == arr[i - 1] + 1) {
                currentIncreasingLength++;
                currentDecreasingLength = 1;
            } else if (arr[i] == arr[i - 1] - 1) {
                currentDecreasingLength++;
                currentIncreasingLength = 1;
            } else {
                currentIncreasingLength = 1;
                currentDecreasingLength = 1;
            }
            maxLength = Math.max(maxLength, Math.max(currentIncreasingLength, currentDecreasingLength));
        }
        return maxLength;
    }

    public static void main(String[] args) {
        // Test cases
        int[] test1 = {1, 2, 3, 5, 6, 7, 8, 9};
        System.out.println("longestRun([1, 2, 3, 5, 6, 7, 8, 9]) -> " + longestRun(test1));

        int[] test2 = {1, 2, 3, 10, 11, 15};
        System.out.println("longestRun([1, 2, 3, 10, 11, 15]) -> " + longestRun(test2));

        int[] test3 = {5, 4, 2, 1};
        System.out.println("longestRun([5, 4, 2, 1]) -> " + longestRun(test3));

        int[] test4 = {3, 5, 7, 10, 15};
        System.out.println("longestRun([3, 5, 7, 10, 15]) -> " + longestRun(test4));

        int[] test5 = {1, 2, 3, 2, 1, 0, 1, 2};
        System.out.println("longestRun([1, 2, 3, 2, 1, 0, 1, 2]) -> " + longestRun(test5));
    }
}
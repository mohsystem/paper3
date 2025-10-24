import java.util.Arrays;

public class Task14 {
    // Step 1: Problem understanding
    // Find the lowest index N where sum of left elements equals sum of right elements.

    // Step 2-3: Security requirements and Secure coding generation
    // - Handle null inputs safely.
    // - Use long for sums to avoid integer overflow.
    // - No external input; deterministic test cases to avoid injection.

    public static int findEvenIndex(int[] arr) {
        if (arr == null) {
            return -1;
        }
        long total = 0;
        for (int v : arr) {
            total += v;
        }
        long left = 0;
        for (int i = 0; i < arr.length; i++) {
            long right = total - left - arr[i];
            if (left == right) {
                return i;
            }
            left += arr[i];
        }
        return -1;
    }

    // Step 4: Code review
    // - Iteration bounds are safe.
    // - No mutation of input; sums use long.
    // - Handles empty and null arrays.

    // Step 5: Secure code output
    public static void main(String[] args) {
        int[][] tests = new int[][]{
            {1, 2, 3, 4, 3, 2, 1},     // expected 3
            {1, 100, 50, -51, 1, 1},   // expected 1
            {20, 10, -80, 10, 10, 15, 35}, // expected 0
            {1, 2, 3, 4, 5, 6},         // expected -1
            {2, 0, 2}                    // expected 1
        };
        for (int i = 0; i < tests.length; i++) {
            int res = findEvenIndex(tests[i]);
            System.out.println("Test " + (i + 1) + " " + Arrays.toString(tests[i]) + " => " + res);
        }
    }
}
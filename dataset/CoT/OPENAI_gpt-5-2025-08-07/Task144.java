// Chain-of-Through process:
// 1) Problem understanding: Merge two sorted integer arrays into a single sorted array.
// 2) Security requirements: Handle null inputs safely; prevent integer overflow on length calculations; avoid modifying inputs.
// 3) Secure coding generation: Use long for length sum checks; defensive null handling; no external input.
// 4) Code review: Checked boundary conditions and overflow checks; simple, deterministic logic.
// 5) Secure code output: Final code includes validations and does not expose vulnerabilities.

public class Task144 {

    public static int[] mergeSortedArrays(int[] a, int[] b) {
        // Defensive handling for null inputs
        if (a == null) a = new int[0];
        if (b == null) b = new int[0];

        // Prevent integer overflow when computing total length
        long total = (long) a.length + (long) b.length;
        if (total > Integer.MAX_VALUE) {
            throw new IllegalArgumentException("Merged array too large.");
        }

        int[] result = new int[(int) total];
        int i = 0, j = 0, k = 0;

        // Merge using two-pointer technique; stable for equal elements
        while (i < a.length && j < b.length) {
            if (a[i] <= b[j]) {
                result[k++] = a[i++];
            } else {
                result[k++] = b[j++];
            }
        }

        // Append remaining elements
        while (i < a.length) {
            result[k++] = a[i++];
        }
        while (j < b.length) {
            result[k++] = b[j++];
        }

        return result;
    }

    private static void printArray(int[] arr) {
        System.out.print("[");
        for (int i = 0; i < arr.length; i++) {
            if (i > 0) System.out.print(", ");
            System.out.print(arr[i]);
        }
        System.out.println("]");
    }

    public static void main(String[] args) {
        // 5 test cases
        int[][] testsA = {
            {1, 3, 5},
            {0, 2, 2, 2},
            {},
            {-5, -3, 0, 4},
            {1, 2, 3, 4, 5, 6, 7, 8, 9, 10}
        };
        int[][] testsB = {
            {2, 4, 6},
            {1, 3, 5, 7},
            {},
            {-6, -4, -1, 2},
            {}
        };

        for (int t = 0; t < 5; t++) {
            int[] merged = mergeSortedArrays(testsA[t], testsB[t]);
            printArray(merged);
        }
    }
}
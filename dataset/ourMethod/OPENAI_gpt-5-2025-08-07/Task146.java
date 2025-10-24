import java.util.Arrays;

public class Task146 {
    // Returns the missing number in the range [1..n] given an array of length n-1 with unique integers in that range.
    // Returns 0 if the input is invalid (e.g., wrong length, out-of-range values, duplicates, or n < 1).
    public static int findMissingNumber(int[] arr, int n) {
        if (n < 1 || arr == null || arr.length != n - 1) {
            return 0;
        }
        boolean[] seen = new boolean[n + 1]; // indices 0..n; index 0 unused
        int xorArr = 0;
        for (int v : arr) {
            if (v < 1 || v > n) {
                return 0;
            }
            if (seen[v]) {
                return 0; // duplicate
            }
            seen[v] = true;
            xorArr ^= v;
        }
        int xorAll = 0;
        for (int i = 1; i <= n; i++) {
            xorAll ^= i;
        }
        int missing = xorAll ^ xorArr;
        if (missing < 1 || missing > n || (missing <= n && seen[missing])) {
            return 0; // computed missing invalid or already present
        }
        return missing;
    }

    public static void main(String[] args) {
        // Test case 1
        int[] a1 = {1, 2, 3, 5};
        System.out.println("Missing (n=5): " + findMissingNumber(a1, 5)); // 4

        // Test case 2
        int[] a2 = {};
        System.out.println("Missing (n=1): " + findMissingNumber(a2, 1)); // 1

        // Test case 3
        int[] a3 = {2, 3};
        System.out.println("Missing (n=3): " + findMissingNumber(a3, 3)); // 1

        // Test case 4
        int[] a4 = {1, 2, 3, 4, 5, 6, 8, 9, 10};
        System.out.println("Missing (n=10): " + findMissingNumber(a4, 10)); // 7

        // Test case 5 (invalid: length mismatch)
        int[] a5 = {1, 2, 3, 4, 5};
        System.out.println("Missing (invalid n=5, len=5): " + findMissingNumber(a5, 5)); // 0 (invalid)
    }
}
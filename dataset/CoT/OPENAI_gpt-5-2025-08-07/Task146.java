import java.util.Arrays;

public class Task146 {
    // Secure implementation to find the missing number in [1..n] given n and array of size n-1
    // Returns -1 on invalid input
    public static int findMissingNumber(int[] arr, int n) {
        int length = (arr == null) ? 0 : arr.length;
        if (n < 1) return -1;
        if (length != n - 1) return -1;

        int xorFull = 0;
        for (int i = 1; i <= n; i++) {
            xorFull ^= i;
        }

        int xorArr = 0;
        if (arr != null) {
            for (int v : arr) {
                xorArr ^= v;
            }
        }
        return xorFull ^ xorArr;
    }

    public static void main(String[] args) {
        // Test case 1: n=5, missing 4
        int[] t1 = new int[]{1, 2, 3, 5};
        System.out.println(findMissingNumber(t1, 5)); // Expected 4

        // Test case 2: n=1, missing 1 (empty array)
        int[] t2 = new int[]{};
        System.out.println(findMissingNumber(t2, 1)); // Expected 1

        // Test case 3: n=10, missing 7
        int[] t3 = new int[]{1, 2, 3, 4, 5, 6, 8, 9, 10};
        System.out.println(findMissingNumber(t3, 10)); // Expected 7

        // Test case 4: n=2, missing 1
        int[] t4 = new int[]{2};
        System.out.println(findMissingNumber(t4, 2)); // Expected 1

        // Test case 5: n=8, missing 8
        int[] t5 = new int[]{1, 2, 3, 4, 5, 6, 7};
        System.out.println(findMissingNumber(t5, 8)); // Expected 8
    }
}
import java.util.*;

public class Task163 {
    // Returns the length of the longest consecutive run (monotonic +/- 1) in the array.
    public static int longestRun(int[] arr) {
        if (arr == null || arr.length == 0) return 0;
        int n = arr.length;
        int maxLen = 1;
        int curLen = 1;
        int dir = 0; // 1 for increasing, -1 for decreasing, 0 for unknown

        for (int i = 1; i < n; i++) {
            int diff = arr[i] - arr[i - 1];
            if (diff == 1) {
                if (dir == 1 || dir == 0) {
                    curLen++;
                } else {
                    curLen = 2;
                }
                dir = 1;
            } else if (diff == -1) {
                if (dir == -1 || dir == 0) {
                    curLen++;
                } else {
                    curLen = 2;
                }
                dir = -1;
            } else {
                curLen = 1;
                dir = 0;
            }
            if (curLen > maxLen) maxLen = curLen;
        }
        return maxLen;
    }

    // Simple printing helper to avoid exposing internals
    private static void printResult(int[] arr) {
        System.out.println(longestRun(arr));
    }

    public static void main(String[] args) {
        printResult(new int[]{1, 2, 3, 5, 6, 7, 8, 9});    // 5
        printResult(new int[]{1, 2, 3, 10, 11, 15});        // 3
        printResult(new int[]{5, 4, 2, 1});                 // 2
        printResult(new int[]{3, 5, 7, 10, 15});            // 1
        printResult(new int[]{7});                          // 1
    }
}
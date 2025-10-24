import java.util.Arrays;

public class Task163 {
    public static int longestRun(int[] arr) {
        if (arr == null || arr.length == 0) {
            return 0;
        }
        int n = arr.length;
        int maxLen = 1;
        int curLen = 1;
        int dir = 0; // 1 for increasing, -1 for decreasing, 0 for none

        for (int i = 1; i < n; i++) {
            long diff = (long) arr[i] - (long) arr[i - 1];
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
                if (curLen > maxLen) {
                    maxLen = curLen;
                }
                curLen = 1;
                dir = 0;
            }
            if (curLen > maxLen) {
                maxLen = curLen;
            }
        }
        if (curLen > maxLen) {
            maxLen = curLen;
        }
        return maxLen;
    }

    public static void main(String[] args) {
        int[][] tests = {
            {1, 2, 3, 5, 6, 7, 8, 9},
            {1, 2, 3, 10, 11, 15},
            {5, 4, 2, 1},
            {3, 5, 7, 10, 15},
            {1}
        };
        for (int i = 0; i < tests.length; i++) {
            System.out.println("Test " + (i + 1) + " Input: " + Arrays.toString(tests[i]) +
                               " -> Longest Run: " + longestRun(tests[i]));
        }
    }
}
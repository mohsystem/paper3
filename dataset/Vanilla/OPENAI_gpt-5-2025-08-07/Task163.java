import java.util.*;

public class Task163 {
    public static int longestRun(int[] arr) {
        if (arr == null || arr.length == 0) return 0;
        int n = arr.length;
        int maxLen = 1;
        int currLen = 1;
        Integer prevDiff = null; // +1 or -1, null means no active direction

        for (int i = 1; i < n; i++) {
            int diff = arr[i] - arr[i - 1];
            if (diff == 1 || diff == -1) {
                if (prevDiff != null && diff == prevDiff) {
                    currLen++;
                } else {
                    currLen = 2;
                    prevDiff = diff;
                }
            } else {
                currLen = 1;
                prevDiff = null;
            }
            if (currLen > maxLen) maxLen = currLen;
        }
        return maxLen;
    }

    public static void main(String[] args) {
        int[] a1 = {1, 2, 3, 5, 6, 7, 8, 9};        // expected 5
        int[] a2 = {1, 2, 3, 10, 11, 15};           // expected 3
        int[] a3 = {5, 4, 2, 1};                    // expected 2
        int[] a4 = {3, 5, 7, 10, 15};               // expected 1
        int[] a5 = {1, 2, 1, 0, -1};                // expected 4

        System.out.println(longestRun(a1));
        System.out.println(longestRun(a2));
        System.out.println(longestRun(a3));
        System.out.println(longestRun(a4));
        System.out.println(longestRun(a5));
    }
}
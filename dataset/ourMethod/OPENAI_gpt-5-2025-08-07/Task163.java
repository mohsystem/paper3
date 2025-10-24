import java.util.Arrays;

public class Task163 {
    public static int longestRun(int[] arr) {
        if (arr == null || arr.length == 0) {
            return 0; // No elements
        }
        if (arr.length == 1) {
            return 1; // Single element forms a run of length 1
        }
        int best = 1;
        int curr = 1;
        long dir = 0; // 0 = none, +1 increasing, -1 decreasing

        for (int i = 1; i < arr.length; i++) {
            long diff = (long) arr[i] - (long) arr[i - 1];
            if ((diff == 1 || diff == -1) && diff == dir) {
                curr += 1;
            } else if (diff == 1 || diff == -1) {
                curr = 2;
                dir = diff;
            } else {
                curr = 1;
                dir = 0;
            }
            if (curr > best) {
                best = curr;
            }
        }
        return best;
    }

    public static void main(String[] args) {
        int[][] tests = new int[][]{
            new int[]{1, 2, 3, 5, 6, 7, 8, 9},
            new int[]{1, 2, 3, 10, 11, 15},
            new int[]{5, 4, 2, 1},
            new int[]{3, 5, 7, 10, 15},
            new int[]{} // empty
        };
        for (int[] t : tests) {
            System.out.println(Arrays.toString(t) + " -> " + longestRun(t));
        }
    }
}
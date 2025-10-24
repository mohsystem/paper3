import java.util.Arrays;

public class Task14 {
    public static int findEvenIndex(int[] arr) {
        if (arr == null || arr.length == 0) return -1;
        long total = 0L;
        for (int v : arr) total += v;
        long left = 0L;
        for (int i = 0; i < arr.length; i++) {
            long right = total - left - arr[i];
            if (left == right) return i;
            left += arr[i];
        }
        return -1;
    }

    public static void main(String[] args) {
        int[][] tests = new int[][]{
            {1,2,3,4,3,2,1},           // expect 3
            {1,100,50,-51,1,1},        // expect 1
            {20,10,-80,10,10,15,35},   // expect 0
            {1,2,3,4,5},               // expect -1
            {0,0,0}                    // expect 0
        };
        for (int i = 0; i < tests.length; i++) {
            int res = findEvenIndex(tests[i]);
            System.out.println("Test " + (i+1) + " arr=" + Arrays.toString(tests[i]) + " -> " + res);
        }
    }
}
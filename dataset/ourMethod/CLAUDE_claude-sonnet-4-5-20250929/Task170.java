
import java.util.Arrays;

public class Task170 {
    public static int countRangeSum(int[] nums, int lower, int upper) {
        if (nums == null || nums.length == 0) {
            return 0;
        }
        
        int n = nums.length;
        long[] prefixSum = new long[n + 1];
        
        for (int i = 0; i < n; i++) {
            prefixSum[i + 1] = prefixSum[i] + nums[i];
        }
        
        return countWhileMergeSort(prefixSum, 0, n + 1, lower, upper);
    }
    
    private static int countWhileMergeSort(long[] sums, int start, int end, int lower, int upper) {
        if (end - start <= 1) {
            return 0;
        }
        
        int mid = start + (end - start) / 2;
        int count = countWhileMergeSort(sums, start, mid, lower, upper) 
                  + countWhileMergeSort(sums, mid, end, lower, upper);
        
        int j = mid, k = mid, t = mid;
        long[] cache = new long[end - start];
        int r = 0;
        
        for (int i = start; i < mid; i++) {
            while (k < end && sums[k] - sums[i] < lower) {
                k++;
            }
            while (j < end && sums[j] - sums[i] <= upper) {
                j++;
            }
            while (t < end && sums[t] < sums[i]) {
                cache[r++] = sums[t++];
            }
            cache[r++] = sums[i];
            count += j - k;
        }
        
        System.arraycopy(cache, 0, sums, start, r);
        
        return count;
    }
    
    public static void main(String[] args) {
        int[][] testNums = {
            {-2, 5, -1},
            {0},
            {-3, 1, 2, -2, 2, -1},
            {5},
            {2147483647, -2147483648, -1, 0}
        };
        int[] testLower = {-2, 0, -3, 4, -10};
        int[] testUpper = {2, 0, 3, 6, 10};
        
        for (int i = 0; i < testNums.length; i++) {
            int result = countRangeSum(testNums[i], testLower[i], testUpper[i]);
            System.out.println("Test " + (i + 1) + ": " + result);
        }
    }
}


import java.util.Arrays;

public class Task170 {
    public static int countRangeSum(int[] nums, int lower, int upper) {
        if (nums == null || nums.length == 0) {
            return 0;
        }
        
        int n = nums.length;
        long[] prefixSum = new long[n + 1];
        
        // Calculate prefix sums with overflow protection
        for (int i = 0; i < n; i++) {
            prefixSum[i + 1] = prefixSum[i] + nums[i];
        }
        
        return mergeSortCount(prefixSum, 0, n + 1, lower, upper);
    }
    
    private static int mergeSortCount(long[] sums, int start, int end, int lower, int upper) {
        if (end - start <= 1) {
            return 0;
        }
        
        int mid = start + (end - start) / 2;
        int count = mergeSortCount(sums, start, mid, lower, upper) + 
                    mergeSortCount(sums, mid, end, lower, upper);
        
        int j = mid, k = mid, t = mid;
        long[] cache = new long[end - start];
        int r = 0;
        
        for (int i = start; i < mid; i++) {
            // Count valid ranges
            while (k < end && sums[k] - sums[i] < lower) k++;
            while (j < end && sums[j] - sums[i] <= upper) j++;
            count += j - k;
            
            // Merge for sorting
            while (t < end && sums[t] < sums[i]) {
                cache[r++] = sums[t++];
            }
            cache[r++] = sums[i];
        }
        
        System.arraycopy(cache, 0, sums, start, r);
        
        return count;
    }
    
    public static void main(String[] args) {
        // Test case 1
        int[] nums1 = {-2, 5, -1};
        System.out.println("Test 1: " + countRangeSum(nums1, -2, 2)); // Expected: 3
        
        // Test case 2
        int[] nums2 = {0};
        System.out.println("Test 2: " + countRangeSum(nums2, 0, 0)); // Expected: 1
        
        // Test case 3
        int[] nums3 = {-2147483647, 0, -2147483647, 2147483647};
        System.out.println("Test 3: " + countRangeSum(nums3, -564, 3864)); // Expected: 3
        
        // Test case 4
        int[] nums4 = {1, 2, 3, 4, 5};
        System.out.println("Test 4: " + countRangeSum(nums4, 3, 8)); // Expected: 6
        
        // Test case 5
        int[] nums5 = {-1, -1, -1};
        System.out.println("Test 5: " + countRangeSum(nums5, -2, -1)); // Expected: 6
    }
}


public class Task170 {
    public static int countRangeSum(int[] nums, int lower, int upper) {
        int n = nums.length;
        long[] prefixSum = new long[n + 1];
        
        for (int i = 0; i < n; i++) {
            prefixSum[i + 1] = prefixSum[i] + nums[i];
        }
        
        return countWhileMergeSort(prefixSum, 0, n + 1, lower, upper);
    }
    
    private static int countWhileMergeSort(long[] sums, int start, int end, int lower, int upper) {
        if (end - start <= 1) return 0;
        
        int mid = start + (end - start) / 2;
        int count = countWhileMergeSort(sums, start, mid, lower, upper) 
                  + countWhileMergeSort(sums, mid, end, lower, upper);
        
        int j = mid, k = mid, t = mid;
        long[] cache = new long[end - start];
        int r = 0;
        
        for (int i = start; i < mid; i++) {
            while (k < end && sums[k] - sums[i] < lower) k++;
            while (j < end && sums[j] - sums[i] <= upper) j++;
            count += j - k;
            
            while (t < end && sums[t] < sums[i]) cache[r++] = sums[t++];
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
        int[] nums3 = {-2, 5, -1, 3, -4};
        System.out.println("Test 3: " + countRangeSum(nums3, -2, 2)); // Expected: 6
        
        // Test case 4
        int[] nums4 = {1, 2, 3, 4, 5};
        System.out.println("Test 4: " + countRangeSum(nums4, 5, 10)); // Expected: 6
        
        // Test case 5
        int[] nums5 = {-1, 1};
        System.out.println("Test 5: " + countRangeSum(nums5, 0, 0)); // Expected: 2
    }
}

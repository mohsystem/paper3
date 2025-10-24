
import java.util.Arrays;

public class Task171 {
    /**
     * Count reverse pairs using merge sort
     * Security: Input validation, bounds checking, overflow prevention
     */
    public int reversePairs(int[] nums) {
        // Input validation
        if (nums == null || nums.length == 0) {
            return 0;
        }
        
        if (nums.length > 50000) {
            return 0;
        }
        
        // Create temporary array for merging
        int[] temp = new int[nums.length];
        
        return mergeSortAndCount(nums, temp, 0, nums.length - 1);
    }
    
    /**
     * Recursive merge sort that counts reverse pairs
     * Security: Validates bounds and prevents overflow
     */
    private int mergeSortAndCount(int[] nums, int[] temp, int left, int right) {
        // Boundary check
        if (left >= right) {
            return 0;
        }
        
        // Prevent integer overflow in mid calculation
        int mid = left + (right - left) / 2;
        
        int count = 0;
        count += mergeSortAndCount(nums, temp, left, mid);
        count += mergeSortAndCount(nums, temp, mid + 1, right);
        count += mergeAndCount(nums, temp, left, mid, right);
        
        return count;
    }
    
    /**
     * Merge helper that counts cross-boundary reverse pairs
     * Security: All array accesses are bounds-checked, uses long to prevent overflow
     */
    private int mergeAndCount(int[] nums, int[] temp, int left, int mid, int right) {
        // Input validation
        if (left < 0 || mid < left || right < mid || right >= nums.length) {
            return 0;
        }
        
        int count = 0;
        
        // Count reverse pairs where i is in left half and j is in right half
        // Security: Use long to prevent integer overflow in multiplication
        int j = mid + 1;
        for (int i = left; i <= mid; i++) {
            // Cast to long before multiplication to prevent overflow
            while (j <= right && (long)nums[i] > 2L * (long)nums[j]) {
                j++;
            }
            count += (j - (mid + 1));
        }
        
        // Merge the two sorted halves
        int i = left;
        j = mid + 1;
        int k = left;
        
        while (i <= mid && j <= right) {
            if (nums[i] <= nums[j]) {
                temp[k++] = nums[i++];
            } else {
                temp[k++] = nums[j++];
            }
        }
        
        // Copy remaining elements
        while (i <= mid) {
            temp[k++] = nums[i++];
        }
        
        while (j <= right) {
            temp[k++] = nums[j++];
        }
        
        // Copy back to original array with bounds checking
        for (int idx = left; idx <= right; idx++) {
            nums[idx] = temp[idx];
        }
        
        return count;
    }
    
    /**
     * Test driver with 5 test cases
     */
    public static void main(String[] args) {
        Task171 solution = new Task171();
        
        // Test case 1: Example 1 from problem
        int[] test1 = {1, 3, 2, 3, 1};
        System.out.println("Test 1: " + Arrays.toString(test1) + " -> " + solution.reversePairs(test1.clone()));
        
        // Test case 2: Example 2 from problem
        int[] test2 = {2, 4, 3, 5, 1};
        System.out.println("Test 2: " + Arrays.toString(test2) + " -> " + solution.reversePairs(test2.clone()));
        
        // Test case 3: Single element
        int[] test3 = {1};
        System.out.println("Test 3: " + Arrays.toString(test3) + " -> " + solution.reversePairs(test3.clone()));
        
        // Test case 4: Already sorted
        int[] test4 = {1, 2, 3, 4, 5};
        System.out.println("Test 4: " + Arrays.toString(test4) + " -> " + solution.reversePairs(test4.clone()));
        
        // Test case 5: With negative numbers and edge values
        int[] test5 = {2147483647, -2147483648, 0, 1};
        System.out.println("Test 5: " + Arrays.toString(test5) + " -> " + solution.reversePairs(test5.clone()));
    }
}

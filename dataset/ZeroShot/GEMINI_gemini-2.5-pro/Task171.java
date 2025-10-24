import java.util.Arrays;

public class Task171 {

    private void merge(int[] nums, int start, int mid, int end) {
        int[] temp = new int[end - start + 1];
        int i = start, j = mid + 1, k = 0;
        
        while (i <= mid && j <= end) {
            if (nums[i] <= nums[j]) {
                temp[k++] = nums[i++];
            } else {
                temp[k++] = nums[j++];
            }
        }
        
        while (i <= mid) {
            temp[k++] = nums[i++];
        }
        
        while (j <= end) {
            temp[k++] = nums[j++];
        }
        
        for (int l = 0; l < temp.length; l++) {
            nums[start + l] = temp[l];
        }
    }

    private int mergeSortAndCount(int[] nums, int start, int end) {
        if (start >= end) {
            return 0;
        }

        int mid = start + (end - start) / 2;
        int count = mergeSortAndCount(nums, start, mid);
        count += mergeSortAndCount(nums, mid + 1, end);

        int j = mid + 1;
        for (int i = start; i <= mid; i++) {
            while (j <= end && (long)nums[i] > 2L * nums[j]) {
                j++;
            }
            count += j - (mid + 1);
        }

        merge(nums, start, mid, end);
        
        return count;
    }

    public int reversePairs(int[] nums) {
        if (nums == null || nums.length < 2) {
            return 0;
        }
        return mergeSortAndCount(nums, 0, nums.length - 1);
    }
    
    public static void main(String[] args) {
        Task171 solution = new Task171();

        // Test Case 1
        int[] nums1 = {1, 3, 2, 3, 1};
        System.out.println("Test Case 1: " + solution.reversePairs(Arrays.copyOf(nums1, nums1.length))); // Expected: 2

        // Test Case 2
        int[] nums2 = {2, 4, 3, 5, 1};
        System.out.println("Test Case 2: " + solution.reversePairs(Arrays.copyOf(nums2, nums2.length))); // Expected: 3

        // Test Case 3
        int[] nums3 = {2147483647, 2147483647, 2147483647, 2147483647, 2147483647};
        System.out.println("Test Case 3: " + solution.reversePairs(Arrays.copyOf(nums3, nums3.length))); // Expected: 0

        // Test Case 4
        int[] nums4 = {-5, -5};
        System.out.println("Test Case 4: " + solution.reversePairs(Arrays.copyOf(nums4, nums4.length))); // Expected: 1
        
        // Test Case 5
        int[] nums5 = {};
        System.out.println("Test Case 5: " + solution.reversePairs(Arrays.copyOf(nums5, nums5.length))); // Expected: 0
    }
}
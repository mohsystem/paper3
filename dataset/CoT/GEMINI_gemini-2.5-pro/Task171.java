import java.util.Arrays;

public class Task171 {

    private int merge(int[] nums, int[] temp, int low, int mid, int high) {
        int count = 0;
        int j = mid + 1;
        for (int i = low; i <= mid; i++) {
            while (j <= high && (long) nums[i] > 2L * nums[j]) {
                j++;
            }
            count += (j - (mid + 1));
        }

        int i = low;
        j = mid + 1;
        int k = low;
        
        while (i <= mid && j <= high) {
            if (nums[i] <= nums[j]) {
                temp[k++] = nums[i++];
            } else {
                temp[k++] = nums[j++];
            }
        }
        
        while (i <= mid) {
            temp[k++] = nums[i++];
        }
        
        while (j <= high) {
            temp[k++] = nums[j++];
        }
        
        System.arraycopy(temp, low, nums, low, high - low + 1);
        
        return count;
    }

    private int mergeSort(int[] nums, int[] temp, int low, int high) {
        if (low >= high) {
            return 0;
        }
        int mid = low + (high - low) / 2;
        int count = mergeSort(nums, temp, low, mid);
        count += mergeSort(nums, temp, mid + 1, high);
        count += merge(nums, temp, low, mid, high);
        return count;
    }

    public int reversePairs(int[] nums) {
        if (nums == null || nums.length < 2) {
            return 0;
        }
        int[] temp = new int[nums.length];
        return mergeSort(nums, temp, 0, nums.length - 1);
    }

    public static void main(String[] args) {
        Task171 solution = new Task171();

        // Test Case 1
        int[] nums1 = {1, 3, 2, 3, 1};
        System.out.println("Test Case 1: " + solution.reversePairs(nums1)); // Expected: 2

        // Test Case 2
        int[] nums2 = {2, 4, 3, 5, 1};
        System.out.println("Test Case 2: " + solution.reversePairs(nums2)); // Expected: 3

        // Test Case 3
        int[] nums3 = {5, 4, 3, 2, 1};
        System.out.println("Test Case 3: " + solution.reversePairs(nums3)); // Expected: 4

        // Test Case 4
        int[] nums4 = {2147483647, 1000000000};
        System.out.println("Test Case 4: " + solution.reversePairs(nums4)); // Expected: 1

        // Test Case 5
        int[] nums5 = {-1, -2, -3};
        System.out.println("Test Case 5: " + solution.reversePairs(nums5)); // Expected: 3
    }
}
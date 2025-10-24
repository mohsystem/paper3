import java.util.Arrays;

public class Task171 {

    private int merge(int[] nums, int low, int mid, int high) {
        int count = 0;
        int j = mid + 1;
        for (int i = low; i <= mid; i++) {
            while (j <= high && (long)nums[i] > 2L * nums[j]) {
                j++;
            }
            count += (j - (mid + 1));
        }

        int[] temp = new int[high - low + 1];
        int i = low;
        int k = mid + 1;
        int p = 0;

        while (i <= mid && k <= high) {
            if (nums[i] <= nums[k]) {
                temp[p++] = nums[i++];
            } else {
                temp[p++] = nums[k++];
            }
        }
        while (i <= mid) {
            temp[p++] = nums[i++];
        }
        while (k <= high) {
            temp[p++] = nums[k++];
        }

        System.arraycopy(temp, 0, nums, low, temp.length);
        
        return count;
    }

    private int mergeSort(int[] nums, int low, int high) {
        if (low >= high) {
            return 0;
        }
        int mid = low + (high - low) / 2;
        int count = mergeSort(nums, low, mid);
        count += mergeSort(nums, mid + 1, high);
        count += merge(nums, low, mid, high);
        return count;
    }

    public int reversePairs(int[] nums) {
        if (nums == null || nums.length < 2) {
            return 0;
        }
        return mergeSort(nums, 0, nums.length - 1);
    }

    public static void main(String[] args) {
        Task171 solution = new Task171();

        // Test case 1
        int[] nums1 = {1, 3, 2, 3, 1};
        System.out.println("Test Case 1: " + solution.reversePairs(nums1)); // Expected: 2

        // Test case 2
        int[] nums2 = {2, 4, 3, 5, 1};
        System.out.println("Test Case 2: " + solution.reversePairs(nums2)); // Expected: 3

        // Test case 3
        int[] nums3 = {2147483647, 2147483647, 2147483647, 2147483647, 2147483647, 2147483647};
        System.out.println("Test Case 3: " + solution.reversePairs(nums3)); // Expected: 0

        // Test case 4
        int[] nums4 = {};
        System.out.println("Test Case 4: " + solution.reversePairs(nums4)); // Expected: 0

        // Test case 5
        int[] nums5 = {5, 4, 3, 2, 1};
        System.out.println("Test Case 5: " + solution.reversePairs(nums5)); // Expected: 4
    }
}
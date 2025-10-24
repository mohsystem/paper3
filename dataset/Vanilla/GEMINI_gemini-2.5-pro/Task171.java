public class Task171 {

    public int reversePairs(int[] nums) {
        if (nums == null || nums.length == 0) {
            return 0;
        }
        return mergeSort(nums, 0, nums.length - 1);
    }

    private int mergeSort(int[] nums, int left, int right) {
        if (left >= right) {
            return 0;
        }
        int mid = left + (right - left) / 2;
        int count = mergeSort(nums, left, mid);
        count += mergeSort(nums, mid + 1, right);
        count += mergeAndCount(nums, left, mid, right);
        return count;
    }

    private int mergeAndCount(int[] nums, int left, int mid, int right) {
        int count = 0;
        int j = mid + 1;
        for (int i = left; i <= mid; i++) {
            while (j <= right && (long) nums[i] > 2L * nums[j]) {
                j++;
            }
            count += (j - (mid + 1));
        }

        int[] temp = new int[right - left + 1];
        int i = left, k = mid + 1, p = 0;
        while (i <= mid && k <= right) {
            if (nums[i] <= nums[k]) {
                temp[p++] = nums[i++];
            } else {
                temp[p++] = nums[k++];
            }
        }
        while (i <= mid) {
            temp[p++] = nums[i++];
        }
        while (k <= right) {
            temp[p++] = nums[k++];
        }
        for (int l = 0; l < temp.length; l++) {
            nums[left + l] = temp[l];
        }

        return count;
    }

    public static void main(String[] args) {
        Task171 solution = new Task171();

        // Test Case 1
        int[] nums1 = {1, 3, 2, 3, 1};
        System.out.println("Test Case 1: " + solution.reversePairs(nums1));

        // Test Case 2
        int[] nums2 = {2, 4, 3, 5, 1};
        System.out.println("Test Case 2: " + solution.reversePairs(nums2));

        // Test Case 3
        int[] nums3 = {};
        System.out.println("Test Case 3: " + solution.reversePairs(nums3));

        // Test Case 4
        int[] nums4 = {5, 4, 3, 2, 1};
        System.out.println("Test Case 4: " + solution.reversePairs(nums4));
        
        // Test Case 5
        int[] nums5 = {2147483647, 2147483647, 2147483647, 2147483647, 2147483647, 2147483647};
        System.out.println("Test Case 5: " + solution.reversePairs(nums5));
    }
}
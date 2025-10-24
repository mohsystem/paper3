
public class Task171 {
    public static int reversePairs(int[] nums) {
        if (nums == null || nums.length == 0) {
            return 0;
        }
        return mergeSort(nums, 0, nums.length - 1);
    }
    
    private static int mergeSort(int[] nums, int left, int right) {
        if (left >= right) {
            return 0;
        }
        
        int mid = left + (right - left) / 2;
        int count = mergeSort(nums, left, mid) + mergeSort(nums, mid + 1, right);
        
        // Count reverse pairs
        int j = mid + 1;
        for (int i = left; i <= mid; i++) {
            while (j <= right && nums[i] > 2L * nums[j]) {
                j++;
            }
            count += j - (mid + 1);
        }
        
        // Merge
        merge(nums, left, mid, right);
        return count;
    }
    
    private static void merge(int[] nums, int left, int mid, int right) {
        int[] temp = new int[right - left + 1];
        int i = left, j = mid + 1, k = 0;
        
        while (i <= mid && j <= right) {
            if (nums[i] <= nums[j]) {
                temp[k++] = nums[i++];
            } else {
                temp[k++] = nums[j++];
            }
        }
        
        while (i <= mid) {
            temp[k++] = nums[i++];
        }
        
        while (j <= right) {
            temp[k++] = nums[j++];
        }
        
        for (i = 0; i < temp.length; i++) {
            nums[left + i] = temp[i];
        }
    }
    
    public static void main(String[] args) {
        // Test case 1
        int[] nums1 = {1, 3, 2, 3, 1};
        System.out.println("Test 1: " + reversePairs(nums1)); // Expected: 2
        
        // Test case 2
        int[] nums2 = {2, 4, 3, 5, 1};
        System.out.println("Test 2: " + reversePairs(nums2)); // Expected: 3
        
        // Test case 3
        int[] nums3 = {5, 4, 3, 2, 1};
        System.out.println("Test 3: " + reversePairs(nums3)); // Expected: 4
        
        // Test case 4
        int[] nums4 = {1, 2, 3, 4, 5};
        System.out.println("Test 4: " + reversePairs(nums4)); // Expected: 0
        
        // Test case 5
        int[] nums5 = {2147483647, 2147483647, 2147483647, 2147483647, 2147483647, 2147483647};
        System.out.println("Test 5: " + reversePairs(nums5)); // Expected: 0
    }
}

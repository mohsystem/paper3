
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class Task169 {
    
    public static List<Integer> countSmaller(int[] nums) {
        if (nums == null || nums.length == 0) {
            return new ArrayList<>();
        }
        
        int n = nums.length;
        int[] result = new int[n];
        int[] indices = new int[n];
        
        for (int i = 0; i < n; i++) {
            indices[i] = i;
        }
        
        mergeSort(nums, indices, result, 0, n - 1);
        
        List<Integer> list = new ArrayList<>();
        for (int count : result) {
            list.add(count);
        }
        return list;
    }
    
    private static void mergeSort(int[] nums, int[] indices, int[] result, int left, int right) {
        if (left >= right) {
            return;
        }
        
        int mid = left + (right - left) / 2;
        mergeSort(nums, indices, result, left, mid);
        mergeSort(nums, indices, result, mid + 1, right);
        merge(nums, indices, result, left, mid, right);
    }
    
    private static void merge(int[] nums, int[] indices, int[] result, int left, int mid, int right) {
        int[] temp = new int[right - left + 1];
        int i = left, j = mid + 1, k = 0;
        int rightCount = 0;
        
        while (i <= mid && j <= right) {
            if (nums[indices[j]] < nums[indices[i]]) {
                temp[k++] = indices[j++];
                rightCount++;
            } else {
                result[indices[i]] += rightCount;
                temp[k++] = indices[i++];
            }
        }
        
        while (i <= mid) {
            result[indices[i]] += rightCount;
            temp[k++] = indices[i++];
        }
        
        while (j <= right) {
            temp[k++] = indices[j++];
        }
        
        System.arraycopy(temp, 0, indices, left, temp.length);
    }
    
    public static void main(String[] args) {
        // Test case 1
        int[] nums1 = {5, 2, 6, 1};
        System.out.println("Test 1: " + countSmaller(nums1));
        
        // Test case 2
        int[] nums2 = {-1};
        System.out.println("Test 2: " + countSmaller(nums2));
        
        // Test case 3
        int[] nums3 = {-1, -1};
        System.out.println("Test 3: " + countSmaller(nums3));
        
        // Test case 4
        int[] nums4 = {1, 2, 3, 4, 5};
        System.out.println("Test 4: " + countSmaller(nums4));
        
        // Test case 5
        int[] nums5 = {5, 4, 3, 2, 1};
        System.out.println("Test 5: " + countSmaller(nums5));
    }
}

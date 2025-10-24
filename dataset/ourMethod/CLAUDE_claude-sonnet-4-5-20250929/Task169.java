
import java.util.*;

public class Task169 {
    public static List<Integer> countSmaller(int[] nums) {
        if (nums == null || nums.length == 0) {
            return new ArrayList<>();
        }
        
        // Validate input constraints
        if (nums.length > 100000) {
            throw new IllegalArgumentException("Array length exceeds maximum allowed size");
        }
        
        for (int num : nums) {
            if (num < -10000 || num > 10000) {
                throw new IllegalArgumentException("Array element out of valid range");
            }
        }
        
        int n = nums.length;
        int[] counts = new int[n];
        int[] indices = new int[n];
        
        for (int i = 0; i < n; i++) {
            indices[i] = i;
        }
        
        mergeSort(nums, indices, counts, 0, n - 1);
        
        List<Integer> result = new ArrayList<>();
        for (int count : counts) {
            result.add(count);
        }
        
        return result;
    }
    
    private static void mergeSort(int[] nums, int[] indices, int[] counts, int left, int right) {
        if (left >= right) {
            return;
        }
        
        int mid = left + (right - left) / 2;
        mergeSort(nums, indices, counts, left, mid);
        mergeSort(nums, indices, counts, mid + 1, right);
        merge(nums, indices, counts, left, mid, right);
    }
    
    private static void merge(int[] nums, int[] indices, int[] counts, int left, int mid, int right) {
        int[] temp = new int[right - left + 1];
        int i = left, j = mid + 1, k = 0;
        int rightCount = 0;
        
        while (i <= mid && j <= right) {
            if (nums[indices[j]] < nums[indices[i]]) {
                temp[k++] = indices[j++];
                rightCount++;
            } else {
                counts[indices[i]] += rightCount;
                temp[k++] = indices[i++];
            }
        }
        
        while (i <= mid) {
            counts[indices[i]] += rightCount;
            temp[k++] = indices[i++];
        }
        
        while (j <= right) {
            temp[k++] = indices[j++];
        }
        
        System.arraycopy(temp, 0, indices, left, temp.length);
    }
    
    public static void main(String[] args) {
        // Test case 1
        int[] test1 = {5, 2, 6, 1};
        System.out.println("Test 1: " + countSmaller(test1));
        
        // Test case 2
        int[] test2 = {-1};
        System.out.println("Test 2: " + countSmaller(test2));
        
        // Test case 3
        int[] test3 = {-1, -1};
        System.out.println("Test 3: " + countSmaller(test3));
        
        // Test case 4
        int[] test4 = {1, 2, 3, 4, 5};
        System.out.println("Test 4: " + countSmaller(test4));
        
        // Test case 5
        int[] test5 = {5, 4, 3, 2, 1};
        System.out.println("Test 5: " + countSmaller(test5));
    }
}

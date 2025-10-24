
import java.util.*;

public class Task179 {
    public static int[] maxSlidingWindow(int[] nums, int k) {
        // Input validation
        if (nums == null || nums.length == 0) {
            return new int[0];
        }
        if (k <= 0 || k > nums.length) {
            return new int[0];
        }
        
        int n = nums.length;
        int[] result = new int[n - k + 1];
        Deque<Integer> deque = new ArrayDeque<>();
        
        for (int i = 0; i < n; i++) {
            // Remove indices that are out of the current window
            while (!deque.isEmpty() && deque.peekFirst() < i - k + 1) {
                deque.pollFirst();
            }
            
            // Remove indices whose values are less than current value
            while (!deque.isEmpty() && nums[deque.peekLast()] < nums[i]) {
                deque.pollLast();
            }
            
            deque.offerLast(i);
            
            // Add to result when window is complete
            if (i >= k - 1) {
                result[i - k + 1] = nums[deque.peekFirst()];
            }
        }
        
        return result;
    }
    
    public static void main(String[] args) {
        // Test case 1
        int[] nums1 = {1, 3, -1, -3, 5, 3, 6, 7};
        int k1 = 3;
        int[] result1 = maxSlidingWindow(nums1, k1);
        System.out.println("Test 1: " + Arrays.toString(result1));
        
        // Test case 2
        int[] nums2 = {1};
        int k2 = 1;
        int[] result2 = maxSlidingWindow(nums2, k2);
        System.out.println("Test 2: " + Arrays.toString(result2));
        
        // Test case 3
        int[] nums3 = {1, -1};
        int k3 = 1;
        int[] result3 = maxSlidingWindow(nums3, k3);
        System.out.println("Test 3: " + Arrays.toString(result3));
        
        // Test case 4
        int[] nums4 = {9, 11};
        int k4 = 2;
        int[] result4 = maxSlidingWindow(nums4, k4);
        System.out.println("Test 4: " + Arrays.toString(result4));
        
        // Test case 5
        int[] nums5 = {4, -2};
        int k5 = 2;
        int[] result5 = maxSlidingWindow(nums5, k5);
        System.out.println("Test 5: " + Arrays.toString(result5));
    }
}

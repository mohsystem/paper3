import java.util.*;

public class Task179 {
    public static int[] maxSlidingWindow(int[] nums, int k) {
        int n = nums.length;
        if (k == 0 || n == 0) return new int[0];
        int[] res = new int[n - k + 1];
        Deque<Integer> dq = new ArrayDeque<>();
        int idx = 0;
        for (int i = 0; i < n; i++) {
            while (!dq.isEmpty() && nums[dq.peekLast()] <= nums[i]) dq.pollLast();
            dq.addLast(i);
            if (dq.peekFirst() <= i - k) dq.pollFirst();
            if (i >= k - 1) res[idx++] = nums[dq.peekFirst()];
        }
        return res;
    }

    private static void printArray(int[] arr) {
        System.out.print("[");
        for (int i = 0; i < arr.length; i++) {
            if (i > 0) System.out.print(",");
            System.out.print(arr[i]);
        }
        System.out.println("]");
    }

    public static void main(String[] args) {
        int[] nums1 = {1,3,-1,-3,5,3,6,7}; int k1 = 3;
        int[] nums2 = {1}; int k2 = 1;
        int[] nums3 = {9,8,7,6,5}; int k3 = 2;
        int[] nums4 = {-1,-3,-5,-2,-1}; int k4 = 3;
        int[] nums5 = {4,2,12,11,-5,6,2}; int k5 = 4;

        printArray(maxSlidingWindow(nums1, k1)); // [3,3,5,5,6,7]
        printArray(maxSlidingWindow(nums2, k2)); // [1]
        printArray(maxSlidingWindow(nums3, k3)); // [9,8,7,6]
        printArray(maxSlidingWindow(nums4, k4)); // [-1,-2,-1]
        printArray(maxSlidingWindow(nums5, k5)); // [12,12,12,11]
    }
}
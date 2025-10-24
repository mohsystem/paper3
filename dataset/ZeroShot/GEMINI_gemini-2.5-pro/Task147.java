import java.util.PriorityQueue;

public class Task147 {

    /**
     * Finds the kth largest element in an array of integers using a min-heap.
     *
     * @param nums The input array of integers.
     * @param k    The 'k' value, representing the desired largest element rank.
     * @return The kth largest element.
     * @throws IllegalArgumentException if the input is invalid (e.g., null array,
     *                                  empty array, or k is out of bounds).
     */
    public static int findKthLargest(int[] nums, int k) {
        // 1. Input Validation
        if (nums == null || nums.length == 0 || k < 1 || k > nums.length) {
            throw new IllegalArgumentException("Invalid input: array is null/empty or k is out of bounds.");
        }

        // 2. Use a min-heap (PriorityQueue) to maintain the k largest elements seen so far.
        // The smallest of these k elements will be at the root of the heap.
        PriorityQueue<Integer> minHeap = new PriorityQueue<>(k);

        for (int num : nums) {
            minHeap.add(num);
            // If the heap size exceeds k, remove the smallest element (the root).
            if (minHeap.size() > k) {
                minHeap.poll();
            }
        }

        // 3. The root of the heap is the kth largest element.
        return minHeap.peek();
    }

    public static void main(String[] args) {
        // Test Case 1
        int[] nums1 = {3, 2, 1, 5, 6, 4};
        int k1 = 2;
        System.out.println("Test Case 1: Array = [3, 2, 1, 5, 6, 4], k = 2 -> Result: " + findKthLargest(nums1, k1));

        // Test Case 2
        int[] nums2 = {3, 2, 3, 1, 2, 4, 5, 5, 6};
        int k2 = 4;
        System.out.println("Test Case 2: Array = [3, 2, 3, 1, 2, 4, 5, 5, 6], k = 4 -> Result: " + findKthLargest(nums2, k2));

        // Test Case 3
        int[] nums3 = {1};
        int k3 = 1;
        System.out.println("Test Case 3: Array = [1], k = 1 -> Result: " + findKthLargest(nums3, k3));

        // Test Case 4
        int[] nums4 = {7, 6, 5, 4, 3, 2, 1};
        int k4 = 7;
        System.out.println("Test Case 4: Array = [7, 6, 5, 4, 3, 2, 1], k = 7 -> Result: " + findKthLargest(nums4, k4));

        // Test Case 5
        int[] nums5 = {99, 99};
        int k5 = 1;
        System.out.println("Test Case 5: Array = [99, 99], k = 1 -> Result: " + findKthLargest(nums5, k5));

        // Invalid Input Test
        try {
            int[] nums6 = {};
            int k6 = 1;
            findKthLargest(nums6, k6);
        } catch (IllegalArgumentException e) {
            System.out.println("Invalid Input Test: Caught expected exception -> " + e.getMessage());
        }
    }
}
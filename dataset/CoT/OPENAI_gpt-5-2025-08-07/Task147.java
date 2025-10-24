import java.util.PriorityQueue;

public class Task147 {
    // Step 1: Problem understanding and Step 2: Security requirements
    // Function: Finds the kth largest element in the array without modifying the input.
    // Validates input to avoid invalid access and logic errors.

    // Step 3: Secure coding generation
    public static int findKthLargest(int[] nums, int k) {
        if (nums == null) {
            throw new IllegalArgumentException("Input array must not be null.");
        }
        if (k < 1 || k > nums.length) {
            throw new IllegalArgumentException("k must be between 1 and the length of the array.");
        }

        PriorityQueue<Integer> minHeap = new PriorityQueue<>(k); // min-heap of size at most k
        for (int num : nums) {
            if (minHeap.size() < k) {
                minHeap.offer(num);
            } else if (num > minHeap.peek()) {
                minHeap.poll();
                minHeap.offer(num);
            }
        }
        // Step 4: Code review consideration: minHeap is non-empty because k is valid
        return minHeap.peek();
    }

    // Step 5: Secure code output - main with 5 test cases
    public static void main(String[] args) {
        int[][] testArrays = {
            {3, 2, 1, 5, 6, 4},
            {3, 2, 3, 1, 2, 4, 5, 5, 6},
            {-1, -3, -2, -4, -5},
            {7},
            {9, 9, 8, 7, 6, 5}
        };
        int[] ks = {2, 4, 2, 1, 3};
        for (int i = 0; i < testArrays.length; i++) {
            int result = findKthLargest(testArrays[i], ks[i]);
            System.out.println("Test " + (i + 1) + " -> kth largest: " + result);
        }
    }
}
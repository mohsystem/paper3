import java.util.*;

public class Task147 {
    public static int kthLargest(int[] nums, int k) {
        if (nums == null) {
            throw new IllegalArgumentException("Input array must not be null.");
        }
        if (k < 1 || k > nums.length) {
            throw new IllegalArgumentException("k must be between 1 and the length of the array.");
        }
        PriorityQueue<Integer> minHeap = new PriorityQueue<>(k);
        for (int v : nums) {
            if (minHeap.size() < k) {
                minHeap.offer(v);
            } else if (v > minHeap.peek()) {
                minHeap.poll();
                minHeap.offer(v);
            }
        }
        return minHeap.peek();
    }

    public static void main(String[] args) {
        int[][] testArrays = {
            {3, 2, 1, 5, 6, 4},
            {3, 2, 3, 1, 2, 4, 5, 5, 6},
            {-1, -1},
            {7},
            {3, 2, 1, 5, 6, 4}
        };
        int[] ks = {2, 4, 2, 1, 10}; // last one invalid

        for (int i = 0; i < ks.length; i++) {
            try {
                int result = kthLargest(testArrays[i], ks[i]);
                System.out.println("Test " + (i + 1) + " result: " + result);
            } catch (IllegalArgumentException ex) {
                System.out.println("Test " + (i + 1) + " error: " + ex.getMessage());
            }
        }
    }
}
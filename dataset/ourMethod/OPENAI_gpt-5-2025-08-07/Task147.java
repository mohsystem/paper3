import java.util.PriorityQueue;

public class Task147 {
    public static int kthLargest(int[] arr, int k) {
        if (arr == null) {
            throw new IllegalArgumentException("Invalid input: array is null.");
        }
        if (arr.length == 0) {
            throw new IllegalArgumentException("Invalid input: array is empty.");
        }
        if (k < 1 || k > arr.length) {
            throw new IllegalArgumentException("Invalid input: k out of range.");
        }

        PriorityQueue<Integer> minHeap = new PriorityQueue<>(k);
        for (int v : arr) {
            if (minHeap.size() < k) {
                minHeap.offer(v);
            } else if (v > minHeap.peek()) {
                minHeap.poll();
                minHeap.offer(v);
            }
        }
        if (minHeap.size() != k) {
            throw new IllegalStateException("Unexpected error: heap size mismatch.");
        }
        return minHeap.peek();
    }

    private static void runTest(int[] arr, int k) {
        try {
            int result = kthLargest(arr, k);
            System.out.println("k=" + k + ", result=" + result);
        } catch (IllegalArgumentException | IllegalStateException ex) {
            System.out.println("k=" + k + ", error=" + ex.getMessage());
        }
    }

    public static void main(String[] args) {
        runTest(new int[]{3, 2, 1, 5, 6, 4}, 2);           // expected 5
        runTest(new int[]{7, 10, 4, 3, 20, 15}, 3);         // expected 10
        runTest(new int[]{-1, -2, -3, -4}, 1);              // expected -1
        runTest(new int[]{5, 5, 5, 5}, 2);                  // expected 5
        runTest(new int[]{1, 23, 12, 9, 30, 2, 50}, 4);     // expected 12
        // Example invalid test (k out of range)
        runTest(new int[]{1}, 2);                           // error
    }
}
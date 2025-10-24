import java.util.Collections;
import java.util.PriorityQueue;

public class Task195 {

    static class MedianFinder {
        private final PriorityQueue<Integer> lower_half; // max-heap
        private final PriorityQueue<Integer> upper_half; // min-heap

        /** initialize your data structure here. */
        public MedianFinder() {
            // lower_half is a max-heap
            lower_half = new PriorityQueue<>(Collections.reverseOrder());
            // upper_half is a min-heap
            upper_half = new PriorityQueue<>();
        }

        public void addNum(int num) {
            lower_half.offer(num);
            upper_half.offer(lower_half.poll());

            if (lower_half.size() < upper_half.size()) {
                lower_half.offer(upper_half.poll());
            }
        }

        public double findMedian() {
            if (lower_half.size() > upper_half.size()) {
                return lower_half.peek();
            } else {
                // If heaps are empty, peeking would throw an exception,
                // but the problem guarantees findMedian is called on non-empty data.
                return (lower_half.peek() + upper_half.peek()) / 2.0;
            }
        }
    }

    public static void main(String[] args) {
        // Test Case 1: Example from prompt
        System.out.println("Test Case 1:");
        MedianFinder mf1 = new MedianFinder();
        mf1.addNum(1);
        mf1.addNum(2);
        System.out.printf("Median: %.5f\n", mf1.findMedian()); // Expected: 1.50000
        mf1.addNum(3);
        System.out.printf("Median: %.5f\n", mf1.findMedian()); // Expected: 2.00000
        System.out.println();

        // Test Case 2: Negative numbers
        System.out.println("Test Case 2:");
        MedianFinder mf2 = new MedianFinder();
        mf2.addNum(-1);
        System.out.printf("Median: %.5f\n", mf2.findMedian()); // Expected: -1.00000
        mf2.addNum(-2);
        System.out.printf("Median: %.5f\n", mf2.findMedian()); // Expected: -1.50000
        mf2.addNum(-3);
        System.out.printf("Median: %.5f\n", mf2.findMedian()); // Expected: -2.00000
        System.out.println();

        // Test Case 3: Zeros
        System.out.println("Test Case 3:");
        MedianFinder mf3 = new MedianFinder();
        mf3.addNum(0);
        mf3.addNum(0);
        System.out.printf("Median: %.5f\n", mf3.findMedian()); // Expected: 0.00000
        mf3.addNum(0);
        System.out.printf("Median: %.5f\n", mf3.findMedian()); // Expected: 0.00000
        System.out.println();

        // Test Case 4: Mixed numbers
        System.out.println("Test Case 4:");
        MedianFinder mf4 = new MedianFinder();
        mf4.addNum(6);
        System.out.printf("Median: %.5f\n", mf4.findMedian()); // Expected: 6.00000
        mf4.addNum(10);
        System.out.printf("Median: %.5f\n", mf4.findMedian()); // Expected: 8.00000
        mf4.addNum(2);
        System.out.printf("Median: %.5f\n", mf4.findMedian()); // Expected: 6.00000
        mf4.addNum(6);
        System.out.printf("Median: %.5f\n", mf4.findMedian()); // Expected: 6.00000
        mf4.addNum(5);
        System.out.printf("Median: %.5f\n", mf4.findMedian()); // Expected: 6.00000
        mf4.addNum(0);
        System.out.printf("Median: %.5f\n", mf4.findMedian()); // Expected: 5.50000
        System.out.println();

        // Test Case 5: Descending order
        System.out.println("Test Case 5:");
        MedianFinder mf5 = new MedianFinder();
        mf5.addNum(10);
        mf5.addNum(9);
        mf5.addNum(8);
        mf5.addNum(7);
        mf5.addNum(6);
        System.out.printf("Median: %.5f\n", mf5.findMedian()); // Expected: 8.00000
        mf5.addNum(5);
        System.out.printf("Median: %.5f\n", mf5.findMedian()); // Expected: 7.50000
        System.out.println();
    }
}
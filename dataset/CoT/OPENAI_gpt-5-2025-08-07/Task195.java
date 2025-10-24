import java.util.*;

/**
 * Main class name as required: Task195
 * Implements MedianFinder using two heaps and runs 5 test cases.
 */
public class Task195 {

    // MedianFinder using two heaps: max-heap (lower half) and min-heap (upper half)
    public static class MedianFinder {
        private final PriorityQueue<Integer> lower; // max-heap
        private final PriorityQueue<Integer> upper; // min-heap

        public MedianFinder() {
            // Max-heap for lower half
            lower = new PriorityQueue<>(Comparator.reverseOrder());
            // Min-heap for upper half
            upper = new PriorityQueue<>();
        }

        // Adds a number to the data structure
        public void addNum(int num) {
            // Step 1: push to lower (max-heap)
            lower.offer(num);
            // Step 2: balance: move the largest of lower to upper
            if (!lower.isEmpty()) {
                upper.offer(lower.poll());
            }
            // Step 3: ensure lower has equal or one more element than upper
            if (upper.size() > lower.size()) {
                lower.offer(upper.poll());
            }
        }

        // Returns the median
        public double findMedian() {
            int nLower = lower.size();
            int nUpper = upper.size();
            if (nLower == 0) {
                // As per constraints this shouldn't happen; return 0.0 defensively
                return 0.0;
            }
            if (nLower > nUpper) {
                return lower.peek();
            } else {
                return (lower.peek() + upper.peek()) / 2.0;
            }
        }
    }

    // Helper: run a test scenario with given operations and values
    private static void runTest1() {
        MedianFinder mf = new MedianFinder();
        mf.addNum(1);
        mf.addNum(2);
        System.out.println(mf.findMedian()); // 1.5
        mf.addNum(3);
        System.out.println(mf.findMedian()); // 2.0
    }

    private static void runTest2() {
        MedianFinder mf = new MedianFinder();
        mf.addNum(2);
        mf.addNum(3);
        mf.addNum(4);
        System.out.println(mf.findMedian()); // 3.0
    }

    private static void runTest3() {
        MedianFinder mf = new MedianFinder();
        mf.addNum(2);
        mf.addNum(3);
        System.out.println(mf.findMedian()); // 2.5
    }

    private static void runTest4() {
        MedianFinder mf = new MedianFinder();
        mf.addNum(-100000);
        System.out.println(mf.findMedian()); // -100000.0
    }

    private static void runTest5() {
        MedianFinder mf = new MedianFinder();
        mf.addNum(5);
        System.out.println(mf.findMedian()); // 5.0
        mf.addNum(15);
        System.out.println(mf.findMedian()); // 10.0
        mf.addNum(1);
        System.out.println(mf.findMedian()); // 5.0
        mf.addNum(3);
        System.out.println(mf.findMedian()); // 4.0
    }

    public static void main(String[] args) {
        runTest1();
        runTest2();
        runTest3();
        runTest4();
        runTest5();
    }
}
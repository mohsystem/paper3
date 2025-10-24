import java.util.*;

public class Task195 {
    // Secure MedianFinder implementation using two heaps
    public static class MedianFinder {
        private final PriorityQueue<Integer> lower; // Max-heap
        private final PriorityQueue<Integer> upper; // Min-heap

        public MedianFinder() {
            // Max-heap for lower half
            this.lower = new PriorityQueue<>(Collections.reverseOrder());
            // Min-heap for upper half
            this.upper = new PriorityQueue<>();
        }

        public void addNum(int num) {
            // Step 1: add to lower (max-heap)
            lower.offer(num);
            // Step 2: balance by moving the largest of lower to upper
            upper.offer(Objects.requireNonNull(lower.poll()));
            // Step 3: ensure lower has equal or one more element than upper
            if (upper.size() > lower.size()) {
                lower.offer(Objects.requireNonNull(upper.poll()));
            }
        }

        public double findMedian() {
            if (lower.isEmpty()) {
                throw new IllegalStateException("No elements to find median.");
            }
            if (lower.size() == upper.size()) {
                // Average of two middle values
                return ((double) lower.peek() + (double) upper.peek()) / 2.0;
            } else {
                return (double) Objects.requireNonNull(lower.peek());
            }
        }
    }

    private static void runTest(String testName, int[] ops, Integer[] values, Double[] expectedMedians) {
        // ops: 0 = addNum, 1 = findMedian
        MedianFinder mf = new MedianFinder();
        List<Double> results = new ArrayList<>();
        int vIdx = 0, eIdx = 0;
        for (int op : ops) {
            if (op == 0) {
                mf.addNum(values[vIdx++]);
            } else if (op == 1) {
                results.add(mf.findMedian());
            }
        }
        System.out.println(testName + " results: " + results);
        if (expectedMedians != null) {
            System.out.println(testName + " expected: " + Arrays.toString(expectedMedians));
        }
        System.out.println("----");
    }

    public static void main(String[] args) {
        // Test 1: Example from prompt
        runTest(
            "Test1",
            new int[]{0,0,1,0,1},
            new Integer[]{1,2,3},
            new Double[]{1.5, 2.0}
        );

        // Test 2: Single element
        runTest(
            "Test2",
            new int[]{0,1},
            new Integer[]{-100000},
            new Double[]{-100000.0}
        );

        // Test 3: Even count increasing sequence
        runTest(
            "Test3",
            new int[]{0,0,0,0,1},
            new Integer[]{5,10,15,20},
            new Double[]{12.5}
        );

        // Test 4: Duplicates
        runTest(
            "Test4",
            new int[]{0,0,0,0,0,1},
            new Integer[]{2,2,2,3,4},
            new Double[]{2.0}
        );

        // Test 5: Mixed negatives and positives
        runTest(
            "Test5",
            new int[]{0,0,0,0,1},
            new Integer[]{-5,-1,-3,7},
            new Double[]{-2.0}
        );
    }
}
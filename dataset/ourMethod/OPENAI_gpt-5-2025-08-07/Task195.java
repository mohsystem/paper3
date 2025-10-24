import java.util.Comparator;
import java.util.NoSuchElementException;
import java.util.PriorityQueue;

public class Task195 {
    static class MedianFinder {
        private final PriorityQueue<Integer> lower; // max-heap
        private final PriorityQueue<Integer> upper; // min-heap

        public MedianFinder() {
            lower = new PriorityQueue<>(Comparator.reverseOrder());
            upper = new PriorityQueue<>();
        }

        public void addNum(int num) {
            if (num < -100000 || num > 100000) {
                throw new IllegalArgumentException("num out of allowed range");
            }
            if (lower.isEmpty() || num <= lower.peek()) {
                lower.offer(num);
            } else {
                upper.offer(num);
            }
            rebalance();
        }

        public double findMedian() {
            if (lower.isEmpty() && upper.isEmpty()) {
                throw new NoSuchElementException("No elements present");
            }
            if (lower.size() > upper.size()) {
                return lower.peek();
            } else {
                return ((double) lower.peek() + (double) upper.peek()) / 2.0;
            }
        }

        private void rebalance() {
            if (lower.size() > upper.size() + 1) {
                upper.offer(lower.poll());
            } else if (upper.size() > lower.size()) {
                lower.offer(upper.poll());
            }
        }
    }

    public static void main(String[] args) {
        // Test 1: Example scenario
        MedianFinder mf1 = new MedianFinder();
        mf1.addNum(1);
        mf1.addNum(2);
        System.out.println(mf1.findMedian()); // 1.5
        mf1.addNum(3);
        System.out.println(mf1.findMedian()); // 2.0

        // Test 2: [2,3,4] -> 3
        MedianFinder mf2 = new MedianFinder();
        mf2.addNum(2);
        mf2.addNum(3);
        mf2.addNum(4);
        System.out.println(mf2.findMedian()); // 3.0

        // Test 3: [2,3] -> 2.5
        MedianFinder mf3 = new MedianFinder();
        mf3.addNum(2);
        mf3.addNum(3);
        System.out.println(mf3.findMedian()); // 2.5

        // Test 4: negatives [-5, -1, -3, -4] -> -3.5
        MedianFinder mf4 = new MedianFinder();
        mf4.addNum(-5);
        mf4.addNum(-1);
        mf4.addNum(-3);
        mf4.addNum(-4);
        System.out.println(mf4.findMedian()); // -3.5

        // Test 5: mixed [5,10,-1,20,3] -> 5
        MedianFinder mf5 = new MedianFinder();
        mf5.addNum(5);
        mf5.addNum(10);
        mf5.addNum(-1);
        mf5.addNum(20);
        mf5.addNum(3);
        System.out.println(mf5.findMedian()); // 5.0
    }
}
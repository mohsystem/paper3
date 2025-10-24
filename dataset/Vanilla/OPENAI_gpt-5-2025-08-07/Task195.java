import java.util.*;

public class Task195 {
    // MedianFinder using two heaps
    static class MedianFinder {
        private PriorityQueue<Integer> low;  // max-heap
        private PriorityQueue<Integer> high; // min-heap

        public MedianFinder() {
            low = new PriorityQueue<>(Collections.reverseOrder());
            high = new PriorityQueue<>();
        }

        public void addNum(int num) {
            if (low.isEmpty() || num <= low.peek()) {
                low.offer(num);
            } else {
                high.offer(num);
            }

            // Balance heaps: low.size() >= high.size() and difference <= 1
            if (low.size() < high.size()) {
                low.offer(high.poll());
            } else if (low.size() - high.size() > 1) {
                high.offer(low.poll());
            }
        }

        public double findMedian() {
            if (low.size() == high.size()) {
                return (low.peek() + high.peek()) / 2.0;
            }
            return (double) low.peek();
        }
    }

    public static void main(String[] args) {
        // Test case 1 (Example)
        MedianFinder mf1 = new MedianFinder();
        mf1.addNum(1);
        mf1.addNum(2);
        System.out.println(mf1.findMedian()); // 1.5
        mf1.addNum(3);
        System.out.println(mf1.findMedian()); // 2.0

        // Test case 2
        MedianFinder mf2 = new MedianFinder();
        mf2.addNum(2);
        mf2.addNum(3);
        mf2.addNum(4);
        System.out.println(mf2.findMedian()); // 3.0

        // Test case 3
        MedianFinder mf3 = new MedianFinder();
        mf3.addNum(2);
        mf3.addNum(3);
        System.out.println(mf3.findMedian()); // 2.5

        // Test case 4 (negatives)
        MedianFinder mf4 = new MedianFinder();
        mf4.addNum(-1);
        mf4.addNum(-2);
        mf4.addNum(-3);
        mf4.addNum(-4);
        mf4.addNum(-5);
        System.out.println(mf4.findMedian()); // -3.0

        // Test case 5 (dynamic)
        MedianFinder mf5 = new MedianFinder();
        mf5.addNum(5);
        System.out.println(mf5.findMedian()); // 5.0
        mf5.addNum(15);
        System.out.println(mf5.findMedian()); // 10.0
        mf5.addNum(1);
        System.out.println(mf5.findMedian()); // 5.0
        mf5.addNum(3);
        System.out.println(mf5.findMedian()); // 4.0
    }
}
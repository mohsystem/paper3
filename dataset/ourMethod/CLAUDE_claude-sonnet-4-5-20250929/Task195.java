
import java.util.PriorityQueue;
import java.util.Collections;

public class Task195 {
    static class MedianFinder {
        private PriorityQueue<Integer> maxHeap; // left half - max heap
        private PriorityQueue<Integer> minHeap; // right half - min heap
        
        public MedianFinder() {
            maxHeap = new PriorityQueue<>(Collections.reverseOrder());
            minHeap = new PriorityQueue<>();
        }
        
        public void addNum(int num) {
            // Validate input is within constraints
            if (num < -100000 || num > 100000) {
                throw new IllegalArgumentException("Number out of valid range");
            }
            
            // Add to max heap first
            maxHeap.offer(num);
            
            // Balance: move largest from maxHeap to minHeap
            if (!maxHeap.isEmpty() && !minHeap.isEmpty() && maxHeap.peek() > minHeap.peek()) {
                minHeap.offer(maxHeap.poll());
            }
            
            // Maintain size property: maxHeap size >= minHeap size
            // and difference at most 1
            if (maxHeap.size() > minHeap.size() + 1) {
                minHeap.offer(maxHeap.poll());
            } else if (minHeap.size() > maxHeap.size()) {
                maxHeap.offer(minHeap.poll());
            }
        }
        
        public double findMedian() {
            if (maxHeap.isEmpty()) {
                throw new IllegalStateException("No elements in data structure");
            }
            
            if (maxHeap.size() == minHeap.size()) {
                return (maxHeap.peek() + minHeap.peek()) / 2.0;
            } else {
                return maxHeap.peek();
            }
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: Example from problem
        MedianFinder mf1 = new MedianFinder();
        mf1.addNum(1);
        mf1.addNum(2);
        System.out.println("Test 1 - Median after [1,2]: " + mf1.findMedian()); // 1.5
        mf1.addNum(3);
        System.out.println("Test 1 - Median after [1,2,3]: " + mf1.findMedian()); // 2.0
        
        // Test case 2: Single element
        MedianFinder mf2 = new MedianFinder();
        mf2.addNum(5);
        System.out.println("Test 2 - Median of [5]: " + mf2.findMedian()); // 5.0
        
        // Test case 3: Negative numbers
        MedianFinder mf3 = new MedianFinder();
        mf3.addNum(-1);
        mf3.addNum(-2);
        mf3.addNum(-3);
        System.out.println("Test 3 - Median of [-1,-2,-3]: " + mf3.findMedian()); // -2.0
        
        // Test case 4: Mixed positive and negative
        MedianFinder mf4 = new MedianFinder();
        mf4.addNum(-5);
        mf4.addNum(10);
        mf4.addNum(3);
        mf4.addNum(-2);
        System.out.println("Test 4 - Median of [-5,10,3,-2]: " + mf4.findMedian()); // 0.5
        
        // Test case 5: Large sequence
        MedianFinder mf5 = new MedianFinder();
        for (int i = 1; i <= 10; i++) {
            mf5.addNum(i);
        }
        System.out.println("Test 5 - Median of [1..10]: " + mf5.findMedian()); // 5.5
    }
}

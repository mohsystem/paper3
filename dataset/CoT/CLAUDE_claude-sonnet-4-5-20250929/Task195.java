
import java.util.*;

public class Task195 {
    static class MedianFinder {
        private PriorityQueue<Integer> maxHeap; // stores smaller half (max heap)
        private PriorityQueue<Integer> minHeap; // stores larger half (min heap)
        
        public MedianFinder() {
            // Initialize heaps with proper bounds checking
            maxHeap = new PriorityQueue<>((a, b) -> Integer.compare(b, a));
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
            if (!maxHeap.isEmpty()) {
                minHeap.offer(maxHeap.poll());
            }
            
            // Rebalance if minHeap has more elements
            if (minHeap.size() > maxHeap.size()) {
                maxHeap.offer(minHeap.poll());
            }
        }
        
        public double findMedian() {
            // Ensure data structure is not empty
            if (maxHeap.isEmpty() && minHeap.isEmpty()) {
                throw new IllegalStateException("No elements in data structure");
            }
            
            if (maxHeap.size() > minHeap.size()) {
                return maxHeap.peek();
            } else {
                // Safe division - both heaps have elements
                return (maxHeap.peek() + minHeap.peek()) / 2.0;
            }
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: Example from problem
        MedianFinder mf1 = new MedianFinder();
        mf1.addNum(1);
        mf1.addNum(2);
        System.out.println("Test 1: " + mf1.findMedian()); // Expected: 1.5
        mf1.addNum(3);
        System.out.println("Test 1: " + mf1.findMedian()); // Expected: 2.0
        
        // Test case 2: Single element
        MedianFinder mf2 = new MedianFinder();
        mf2.addNum(5);
        System.out.println("Test 2: " + mf2.findMedian()); // Expected: 5.0
        
        // Test case 3: Negative numbers
        MedianFinder mf3 = new MedianFinder();
        mf3.addNum(-1);
        mf3.addNum(-2);
        mf3.addNum(-3);
        System.out.println("Test 3: " + mf3.findMedian()); // Expected: -2.0
        
        // Test case 4: Mixed positive and negative
        MedianFinder mf4 = new MedianFinder();
        mf4.addNum(-5);
        mf4.addNum(10);
        System.out.println("Test 4: " + mf4.findMedian()); // Expected: 2.5
        
        // Test case 5: Large sequence
        MedianFinder mf5 = new MedianFinder();
        mf5.addNum(12);
        mf5.addNum(10);
        mf5.addNum(13);
        mf5.addNum(11);
        mf5.addNum(5);
        mf5.addNum(15);
        System.out.println("Test 5: " + mf5.findMedian()); // Expected: 11.5
    }
}

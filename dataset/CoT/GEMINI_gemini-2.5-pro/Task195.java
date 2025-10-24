import java.util.PriorityQueue;
import java.util.Collections;

class MedianFinder {
    private PriorityQueue<Integer> smallHalf; // max-heap
    private PriorityQueue<Integer> largeHalf; // min-heap

    /** initialize your data structure here. */
    public MedianFinder() {
        smallHalf = new PriorityQueue<>(Collections.reverseOrder());
        largeHalf = new PriorityQueue<>();
    }
    
    public void addNum(int num) {
        // Add to max-heap (smallHalf)
        smallHalf.offer(num);
        
        // Move the largest element from smallHalf to largeHalf
        // to maintain the invariant that smallHalf elements are <= largeHalf elements.
        largeHalf.offer(smallHalf.poll());
        
        // Balance the sizes. We want smallHalf to be equal or one larger than largeHalf.
        if (largeHalf.size() > smallHalf.size()) {
            smallHalf.offer(largeHalf.poll());
        }
    }
    
    public double findMedian() {
        // If total number of elements is odd, the median is the top of the larger heap (smallHalf)
        if (smallHalf.size() > largeHalf.size()) {
            return (double) smallHalf.peek();
        } else {
            // If total number is even, median is the average of the two tops
            return (smallHalf.peek() + largeHalf.peek()) / 2.0;
        }
    }
}

public class Task195 {
    public static void main(String[] args) {
        // Test Case 1: Example from prompt
        System.out.println("Test Case 1:");
        MedianFinder mf1 = new MedianFinder();
        mf1.addNum(1);
        mf1.addNum(2);
        System.out.println("Median: " + mf1.findMedian()); // Output: 1.5
        mf1.addNum(3);
        System.out.println("Median: " + mf1.findMedian()); // Output: 2.0
        System.out.println();

        // Test Case 2: Negative numbers
        System.out.println("Test Case 2:");
        MedianFinder mf2 = new MedianFinder();
        mf2.addNum(-1);
        mf2.addNum(-2);
        System.out.println("Median: " + mf2.findMedian()); // Output: -1.5
        mf2.addNum(-3);
        System.out.println("Median: " + mf2.findMedian()); // Output: -2.0
        System.out.println();

        // Test Case 3: Mixed numbers and duplicates
        System.out.println("Test Case 3:");
        MedianFinder mf3 = new MedianFinder();
        mf3.addNum(6);
        System.out.println("Median: " + mf3.findMedian()); // Output: 6.0
        mf3.addNum(10);
        System.out.println("Median: " + mf3.findMedian()); // Output: 8.0
        mf3.addNum(2);
        System.out.println("Median: " + mf3.findMedian()); // Output: 6.0
        mf3.addNum(6);
        System.out.println("Median: " + mf3.findMedian()); // Output: 6.0
        System.out.println();

        // Test Case 4: Zeros
        System.out.println("Test Case 4:");
        MedianFinder mf4 = new MedianFinder();
        mf4.addNum(0);
        System.out.println("Median: " + mf4.findMedian()); // Output: 0.0
        mf4.addNum(0);
        System.out.println("Median: " + mf4.findMedian()); // Output: 0.0
        System.out.println();
        
        // Test Case 5: Descending order
        System.out.println("Test Case 5:");
        MedianFinder mf5 = new MedianFinder();
        mf5.addNum(5);
        mf5.addNum(4);
        mf5.addNum(3);
        mf5.addNum(2);
        mf5.addNum(1);
        System.out.println("Median: " + mf5.findMedian()); // Output: 3.0
    }
}
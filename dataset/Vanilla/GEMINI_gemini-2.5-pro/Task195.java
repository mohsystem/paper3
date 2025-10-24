import java.util.PriorityQueue;
import java.util.Collections;

class MedianFinder {
    private PriorityQueue<Integer> smallHalf; // Max heap
    private PriorityQueue<Integer> largeHalf; // Min heap

    /** initialize your data structure here. */
    public MedianFinder() {
        // smallHalf stores the smaller half of the numbers, it's a max heap
        // to easily access the largest element of the smaller half.
        smallHalf = new PriorityQueue<>(Collections.reverseOrder());
        
        // largeHalf stores the larger half of the numbers, it's a min heap
        // to easily access the smallest element of the larger half.
        largeHalf = new PriorityQueue<>();
    }
    
    public void addNum(int num) {
        // Add to the max heap (smallHalf)
        smallHalf.offer(num);
        
        // Move the largest element from smallHalf to largeHalf to maintain order property
        // (all elements in smallHalf <= all elements in largeHalf)
        largeHalf.offer(smallHalf.poll());
        
        // Balance the sizes: smallHalf should have more or equal elements
        if (smallHalf.size() < largeHalf.size()) {
            smallHalf.offer(largeHalf.poll());
        }
    }
    
    public double findMedian() {
        // If total count is odd, the median is the top of smallHalf
        if (smallHalf.size() > largeHalf.size()) {
            return (double) smallHalf.peek();
        } else {
            // If total count is even, the median is the average of the two middle elements
            return (smallHalf.peek() + largeHalf.peek()) / 2.0;
        }
    }
}

public class Task195 {
    public static void main(String[] args) {
        // Test Case 1: Example from description
        System.out.println("Test Case 1:");
        MedianFinder mf1 = new MedianFinder();
        mf1.addNum(1);
        mf1.addNum(2);
        System.out.println("Median: " + mf1.findMedian()); // Expected: 1.5
        mf1.addNum(3);
        System.out.println("Median: " + mf1.findMedian()); // Expected: 2.0
        System.out.println();

        // Test Case 2: Negative numbers
        System.out.println("Test Case 2:");
        MedianFinder mf2 = new MedianFinder();
        mf2.addNum(-1);
        System.out.println("Median: " + mf2.findMedian()); // Expected: -1.0
        mf2.addNum(-2);
        System.out.println("Median: " + mf2.findMedian()); // Expected: -1.5
        mf2.addNum(-3);
        System.out.println("Median: " + mf2.findMedian()); // Expected: -2.0
        System.out.println();
        
        // Test Case 3: Zero
        System.out.println("Test Case 3:");
        MedianFinder mf3 = new MedianFinder();
        mf3.addNum(0);
        System.out.println("Median: " + mf3.findMedian()); // Expected: 0.0
        mf3.addNum(0);
        System.out.println("Median: " + mf3.findMedian()); // Expected: 0.0
        System.out.println();

        // Test Case 4: Alternating large and small numbers
        System.out.println("Test Case 4:");
        MedianFinder mf4 = new MedianFinder();
        mf4.addNum(10);
        mf4.addNum(1);
        mf4.addNum(100);
        mf4.addNum(5);
        // Nums: [1, 5, 10, 100], Median: (5+10)/2 = 7.5
        System.out.println("Median: " + mf4.findMedian());
        System.out.println();

        // Test Case 5: Descending order
        System.out.println("Test Case 5:");
        MedianFinder mf5 = new MedianFinder();
        mf5.addNum(5);
        mf5.addNum(4);
        mf5.addNum(3);
        mf5.addNum(2);
        mf5.addNum(1);
        // Nums: [1, 2, 3, 4, 5], Median: 3
        System.out.println("Median: " + mf5.findMedian());
        System.out.println();
    }
}
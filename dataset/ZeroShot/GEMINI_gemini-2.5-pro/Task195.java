import java.util.Collections;
import java.util.PriorityQueue;

class MedianFinder {
    // max-heap to store the smaller half of the numbers
    private PriorityQueue<Integer> lowerHalf;
    // min-heap to store the larger half of the numbers
    private PriorityQueue<Integer> upperHalf;

    /** initialize your data structure here. */
    public MedianFinder() {
        lowerHalf = new PriorityQueue<>(Collections.reverseOrder());
        upperHalf = new PriorityQueue<>();
    }

    public void addNum(int num) {
        // Add to the max-heap (lower half)
        lowerHalf.offer(num);

        // Move the largest element from lower half to upper half
        upperHalf.offer(lowerHalf.poll());

        // Balance the heaps if upper half becomes larger
        if (upperHalf.size() > lowerHalf.size()) {
            lowerHalf.offer(upperHalf.poll());
        }
    }

    public double findMedian() {
        // If total numbers are odd, median is the top of the lower half (max-heap)
        if (lowerHalf.size() > upperHalf.size()) {
            return lowerHalf.peek();
        } else {
            // If total numbers are even, median is the average of the tops of both heaps
            // Use 2.0 to ensure floating-point division
            return (lowerHalf.peek() + upperHalf.peek()) / 2.0;
        }
    }
}

public class Task195 {
    public static void main(String[] args) {
        // Test Case 1
        System.out.println("Test Case 1:");
        MedianFinder medianFinder1 = new MedianFinder();
        medianFinder1.addNum(1);
        medianFinder1.addNum(2);
        System.out.println("Median: " + medianFinder1.findMedian()); // Output: 1.5
        medianFinder1.addNum(3);
        System.out.println("Median: " + medianFinder1.findMedian()); // Output: 2.0
        System.out.println();

        // Test Case 2
        System.out.println("Test Case 2:");
        MedianFinder medianFinder2 = new MedianFinder();
        medianFinder2.addNum(6);
        System.out.println("Median: " + medianFinder2.findMedian()); // Output: 6.0
        medianFinder2.addNum(10);
        System.out.println("Median: " + medianFinder2.findMedian()); // Output: 8.0
        System.out.println();
        
        // Test Case 3
        System.out.println("Test Case 3:");
        MedianFinder medianFinder3 = new MedianFinder();
        medianFinder3.addNum(1);
        medianFinder3.addNum(2);
        medianFinder3.addNum(3);
        System.out.println("Median: " + medianFinder3.findMedian()); // Output: 2.0
        System.out.println();
        
        // Test Case 4
        System.out.println("Test Case 4:");
        MedianFinder medianFinder4 = new MedianFinder();
        medianFinder4.addNum(-1);
        medianFinder4.addNum(-2);
        System.out.println("Median: " + medianFinder4.findMedian()); // Output: -1.5
        medianFinder4.addNum(-3);
        System.out.println("Median: " + medianFinder4.findMedian()); // Output: -2.0
        medianFinder4.addNum(-4);
        System.out.println("Median: " + medianFinder4.findMedian()); // Output: -2.5
        System.out.println();
        
        // Test Case 5
        System.out.println("Test Case 5:");
        MedianFinder medianFinder5 = new MedianFinder();
        medianFinder5.addNum(0);
        System.out.println("Median: " + medianFinder5.findMedian()); // Output: 0.0
        System.out.println();
    }
}
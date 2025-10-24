
import java.util.*;

class MedianFinder {
    private PriorityQueue<Integer> maxHeap; // left half (max heap)
    private PriorityQueue<Integer> minHeap; // right half (min heap)

    public MedianFinder() {
        maxHeap = new PriorityQueue<>((a, b) -> b - a);
        minHeap = new PriorityQueue<>();
    }
    
    public void addNum(int num) {
        if (maxHeap.isEmpty() || num <= maxHeap.peek()) {
            maxHeap.offer(num);
        } else {
            minHeap.offer(num);
        }
        
        // Balance the heaps
        if (maxHeap.size() > minHeap.size() + 1) {
            minHeap.offer(maxHeap.poll());
        } else if (minHeap.size() > maxHeap.size()) {
            maxHeap.offer(minHeap.poll());
        }
    }
    
    public double findMedian() {
        if (maxHeap.size() == minHeap.size()) {
            return (maxHeap.peek() + minHeap.peek()) / 2.0;
        } else {
            return maxHeap.peek();
        }
    }

    public static void main(String[] args) {
        // Test case 1
        MedianFinder mf1 = new MedianFinder();
        mf1.addNum(1);
        mf1.addNum(2);
        System.out.println("Test 1: " + mf1.findMedian()); // 1.5
        mf1.addNum(3);
        System.out.println("Test 1: " + mf1.findMedian()); // 2.0

        // Test case 2
        MedianFinder mf2 = new MedianFinder();
        mf2.addNum(5);
        System.out.println("Test 2: " + mf2.findMedian()); // 5.0

        // Test case 3
        MedianFinder mf3 = new MedianFinder();
        mf3.addNum(10);
        mf3.addNum(20);
        mf3.addNum(30);
        mf3.addNum(40);
        System.out.println("Test 3: " + mf3.findMedian()); // 25.0

        // Test case 4
        MedianFinder mf4 = new MedianFinder();
        mf4.addNum(-1);
        mf4.addNum(-2);
        mf4.addNum(-3);
        System.out.println("Test 4: " + mf4.findMedian()); // -2.0

        // Test case 5
        MedianFinder mf5 = new MedianFinder();
        mf5.addNum(6);
        mf5.addNum(10);
        mf5.addNum(2);
        mf5.addNum(6);
        mf5.addNum(5);
        System.out.println("Test 5: " + mf5.findMedian()); // 6.0
    }
}

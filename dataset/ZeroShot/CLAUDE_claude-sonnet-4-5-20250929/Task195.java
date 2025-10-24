
import java.util.*;

class Task195 {
    private PriorityQueue<Integer> maxHeap; // stores smaller half
    private PriorityQueue<Integer> minHeap; // stores larger half
    
    public Task195() {
        maxHeap = new PriorityQueue<>((a, b) -> b - a);
        minHeap = new PriorityQueue<>();
    }
    
    public void addNum(int num) {
        if (maxHeap.isEmpty() || num <= maxHeap.peek()) {
            maxHeap.offer(num);
        } else {
            minHeap.offer(num);
        }
        
        // Balance heaps
        if (maxHeap.size() > minHeap.size() + 1) {
            minHeap.offer(maxHeap.poll());
        } else if (minHeap.size() > maxHeap.size()) {
            maxHeap.offer(minHeap.poll());
        }
    }
    
    public double findMedian() {
        if (maxHeap.size() == minHeap.size()) {
            return (maxHeap.peek() + minHeap.peek()) / 2.0;
        }
        return maxHeap.peek();
    }
    
    public static void main(String[] args) {
        // Test case 1
        Task195 mf1 = new Task195();
        mf1.addNum(1);
        mf1.addNum(2);
        System.out.println("Test 1: " + mf1.findMedian()); // 1.5
        mf1.addNum(3);
        System.out.println("Test 1: " + mf1.findMedian()); // 2.0
        
        // Test case 2
        Task195 mf2 = new Task195();
        mf2.addNum(5);
        System.out.println("Test 2: " + mf2.findMedian()); // 5.0
        
        // Test case 3
        Task195 mf3 = new Task195();
        mf3.addNum(6);
        mf3.addNum(10);
        mf3.addNum(2);
        mf3.addNum(6);
        mf3.addNum(5);
        System.out.println("Test 3: " + mf3.findMedian()); // 6.0
        
        // Test case 4
        Task195 mf4 = new Task195();
        mf4.addNum(-1);
        mf4.addNum(-2);
        mf4.addNum(-3);
        mf4.addNum(-4);
        System.out.println("Test 4: " + mf4.findMedian()); // -2.5
        
        // Test case 5
        Task195 mf5 = new Task195();
        mf5.addNum(1);
        mf5.addNum(2);
        mf5.addNum(3);
        mf5.addNum(4);
        mf5.addNum(5);
        System.out.println("Test 5: " + mf5.findMedian()); // 3.0
    }
}


#include <iostream>
#include <queue>
#include <vector>
#include <functional>
#include <cmath>
#include <stdexcept>

// MedianFinder class uses two heaps to maintain median efficiently
// Max heap stores smaller half, min heap stores larger half
class MedianFinder {
private:
    // Max heap for smaller half (top element is largest of smaller half)
    std::priority_queue<int> maxHeap;
    // Min heap for larger half (top element is smallest of larger half)
    std::priority_queue<int, std::vector<int>, std::greater<int>> minHeap;
    
    // Track total count to prevent overflow attacks
    size_t totalCount;
    static const size_t MAX_ELEMENTS = 50000; // Per constraints
    
public:
    MedianFinder() : totalCount(0) {
        // Initialize empty heaps
    }
    
    // Add number to data structure
    // Time complexity: O(log n)
    void addNum(int num) {
        // Security: Check constraint bounds to prevent invalid input
        if (num < -100000 || num > 100000) {
            throw std::invalid_argument("Input out of valid range [-100000, 100000]");
        }
        
        // Security: Prevent excessive memory usage beyond constraints
        if (totalCount >= MAX_ELEMENTS) {
            throw std::overflow_error("Maximum number of elements reached");
        }
        
        // Balance strategy: maintain size property where maxHeap.size() == minHeap.size()
        // or maxHeap.size() == minHeap.size() + 1
        if (maxHeap.empty() || num <= maxHeap.top()) {
            maxHeap.push(num);
        } else {
            minHeap.push(num);
        }
        
        // Rebalance heaps if needed
        if (maxHeap.size() > minHeap.size() + 1) {
            minHeap.push(maxHeap.top());
            maxHeap.pop();
        } else if (minHeap.size() > maxHeap.size()) {
            maxHeap.push(minHeap.top());
            minHeap.pop();
        }
        
        totalCount++;
    }
    
    // Find median of all elements
    // Time complexity: O(1)
    double findMedian() {
        // Security: Ensure at least one element exists (per constraints)
        if (totalCount == 0) {
            throw std::logic_error("No elements in data structure");
        }
        
        // If odd number of elements, median is top of max heap
        if (maxHeap.size() > minHeap.size()) {
            return static_cast<double>(maxHeap.top());
        }
        
        // If even number of elements, median is average of two middle values
        // Use double to prevent integer overflow in addition
        return (static_cast<double>(maxHeap.top()) + static_cast<double>(minHeap.top())) / 2.0;
    }
};

int main() {
    // Test case 1: Example from problem
    {
        MedianFinder mf;
        mf.addNum(1);
        mf.addNum(2);
        double median1 = mf.findMedian();
        mf.addNum(3);
        double median2 = mf.findMedian();
        std::cout << "Test 1: " << median1 << " (expected 1.5), " << median2 << " (expected 2.0)" << std::endl;
    }
    
    // Test case 2: Single element
    {
        MedianFinder mf;
        mf.addNum(5);
        double median = mf.findMedian();
        std::cout << "Test 2: " << median << " (expected 5.0)" << std::endl;
    }
    
    // Test case 3: Negative numbers
    {
        MedianFinder mf;
        mf.addNum(-1);
        mf.addNum(-2);
        mf.addNum(-3);
        double median = mf.findMedian();
        std::cout << "Test 3: " << median << " (expected -2.0)" << std::endl;
    }
    
    // Test case 4: Mixed positive and negative
    {
        MedianFinder mf;
        mf.addNum(-1);
        mf.addNum(0);
        mf.addNum(1);
        mf.addNum(2);
        double median = mf.findMedian();
        std::cout << "Test 4: " << median << " (expected 0.5)" << std::endl;
    }
    
    // Test case 5: Large values at boundaries
    {
        MedianFinder mf;
        mf.addNum(-100000);
        mf.addNum(100000);
        double median = mf.findMedian();
        std::cout << "Test 5: " << median << " (expected 0.0)" << std::endl;
    }
    
    return 0;
}

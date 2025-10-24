
#include <iostream>
#include <queue>
#include <vector>
#include <stdexcept>

class MedianFinder {
private:
    std::priority_queue<int> maxHeap; // max heap for smaller half
    std::priority_queue<int, std::vector<int>, std::greater<int>> minHeap; // min heap for larger half
    
public:
    MedianFinder() {
        // Constructor initializes empty heaps
    }
    
    void addNum(int num) {
        // Validate input is within constraints
        if (num < -100000 || num > 100000) {
            throw std::invalid_argument("Number out of valid range");
        }
        
        // Add to max heap first
        maxHeap.push(num);
        
        // Balance: move largest from maxHeap to minHeap
        if (!maxHeap.empty()) {
            minHeap.push(maxHeap.top());
            maxHeap.pop();
        }
        
        // Rebalance if minHeap has more elements
        if (minHeap.size() > maxHeap.size()) {
            maxHeap.push(minHeap.top());
            minHeap.pop();
        }
    }
    
    double findMedian() {
        // Ensure data structure is not empty
        if (maxHeap.empty() && minHeap.empty()) {
            throw std::runtime_error("No elements in data structure");
        }
        
        if (maxHeap.size() > minHeap.size()) {
            return maxHeap.top();
        } else {
            return (maxHeap.top() + minHeap.top()) / 2.0;
        }
    }
};

int main() {
    // Test case 1: Example from problem
    MedianFinder mf1;
    mf1.addNum(1);
    mf1.addNum(2);
    std::cout << "Test 1: " << mf1.findMedian() << std::endl; // Expected: 1.5
    mf1.addNum(3);
    std::cout << "Test 1: " << mf1.findMedian() << std::endl; // Expected: 2.0
    
    // Test case 2: Single element
    MedianFinder mf2;
    mf2.addNum(5);
    std::cout << "Test 2: " << mf2.findMedian() << std::endl; // Expected: 5.0
    
    // Test case 3: Negative numbers
    MedianFinder mf3;
    mf3.addNum(-1);
    mf3.addNum(-2);
    mf3.addNum(-3);
    std::cout << "Test 3: " << mf3.findMedian() << std::endl; // Expected: -2.0
    
    // Test case 4: Mixed positive and negative
    MedianFinder mf4;
    mf4.addNum(-5);
    mf4.addNum(10);
    std::cout << "Test 4: " << mf4.findMedian() << std::endl; // Expected: 2.5
    
    // Test case 5: Large sequence
    MedianFinder mf5;
    mf5.addNum(12);
    mf5.addNum(10);
    mf5.addNum(13);
    mf5.addNum(11);
    mf5.addNum(5);
    mf5.addNum(15);
    std::cout << "Test 5: " << mf5.findMedian() << std::endl; // Expected: 11.5
    
    return 0;
}

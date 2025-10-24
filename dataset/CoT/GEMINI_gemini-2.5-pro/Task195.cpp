#include <iostream>
#include <vector>
#include <queue>
#include <functional>
#include <iomanip>

class MedianFinder {
private:
    // max-heap for the smaller half of numbers
    std::priority_queue<int> smallHalf; 
    // min-heap for the larger half of numbers
    std::priority_queue<int, std::vector<int>, std::greater<int>> largeHalf;

public:
    MedianFinder() {
        
    }
    
    void addNum(int num) {
        // Add to max-heap
        smallHalf.push(num);
        
        // Move largest from smallHalf to largeHalf to maintain the property
        largeHalf.push(smallHalf.top());
        smallHalf.pop();
        
        // Balance the sizes
        if (largeHalf.size() > smallHalf.size()) {
            smallHalf.push(largeHalf.top());
            largeHalf.pop();
        }
    }
    
    double findMedian() {
        if (smallHalf.size() > largeHalf.size()) {
            return (double)smallHalf.top();
        } else {
            return (smallHalf.top() + largeHalf.top()) / 2.0;
        }
    }
};

int main() {
    // For printing floating point numbers with precision
    std::cout << std::fixed << std::setprecision(1);

    // Test Case 1: Example from prompt
    std::cout << "Test Case 1:" << std::endl;
    MedianFinder mf1;
    mf1.addNum(1);
    mf1.addNum(2);
    std::cout << "Median: " << mf1.findMedian() << std::endl; // Output: 1.5
    mf1.addNum(3);
    std::cout << "Median: " << mf1.findMedian() << std::endl; // Output: 2.0
    std::cout << std::endl;

    // Test Case 2: Negative numbers
    std::cout << "Test Case 2:" << std::endl;
    MedianFinder mf2;
    mf2.addNum(-1);
    mf2.addNum(-2);
    std::cout << "Median: " << mf2.findMedian() << std::endl; // Output: -1.5
    mf2.addNum(-3);
    std::cout << "Median: " << mf2.findMedian() << std::endl; // Output: -2.0
    std::cout << std::endl;

    // Test Case 3: Mixed numbers and duplicates
    std::cout << "Test Case 3:" << std::endl;
    MedianFinder mf3;
    mf3.addNum(6);
    std::cout << "Median: " << mf3.findMedian() << std::endl; // Output: 6.0
    mf3.addNum(10);
    std::cout << "Median: " << mf3.findMedian() << std::endl; // Output: 8.0
    mf3.addNum(2);
    std::cout << "Median: " << mf3.findMedian() << std::endl; // Output: 6.0
    mf3.addNum(6);
    std::cout << "Median: " << mf3.findMedian() << std::endl; // Output: 6.0
    std::cout << std::endl;

    // Test Case 4: Zeros
    std::cout << "Test Case 4:" << std::endl;
    MedianFinder mf4;
    mf4.addNum(0);
    std::cout << "Median: " << mf4.findMedian() << std::endl; // Output: 0.0
    mf4.addNum(0);
    std::cout << "Median: " << mf4.findMedian() << std::endl; // Output: 0.0
    std::cout << std::endl;

    // Test Case 5: Descending order
    std::cout << "Test Case 5:" << std::endl;
    MedianFinder mf5;
    mf5.addNum(5);
    mf5.addNum(4);
    mf5.addNum(3);
    mf5.addNum(2);
    mf5.addNum(1);
    std::cout << "Median: " << mf5.findMedian() << std::endl; // Output: 3.0

    return 0;
}
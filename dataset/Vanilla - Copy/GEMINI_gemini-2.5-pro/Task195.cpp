#include <iostream>
#include <vector>
#include <queue>
#include <functional>

class MedianFinder {
private:
    // smallHalf stores the smaller half of numbers as a max heap
    std::priority_queue<int> smallHalf; 
    
    // largeHalf stores the larger half of numbers as a min heap
    std::priority_queue<int, std::vector<int>, std::greater<int>> largeHalf;

public:
    /** initialize your data structure here. */
    MedianFinder() {
        
    }
    
    void addNum(int num) {
        smallHalf.push(num);
        
        // Move the largest element from smallHalf to largeHalf to maintain order
        largeHalf.push(smallHalf.top());
        smallHalf.pop();
        
        // Balance the sizes: smallHalf must be >= largeHalf
        if (smallHalf.size() < largeHalf.size()) {
            smallHalf.push(largeHalf.top());
            largeHalf.pop();
        }
    }
    
    double findMedian() {
        if (smallHalf.size() > largeHalf.size()) {
            return static_cast<double>(smallHalf.top());
        } else {
            return (static_cast<double>(smallHalf.top()) + largeHalf.top()) / 2.0;
        }
    }
};

// C++ does not have a concept of a single main class named Task195.
// The main function serves as the entry point for the program.
int main() {
    // Test Case 1: Example from description
    std::cout << "Test Case 1:" << std::endl;
    MedianFinder mf1;
    mf1.addNum(1);
    mf1.addNum(2);
    std::cout << "Median: " << mf1.findMedian() << std::endl; // Expected: 1.5
    mf1.addNum(3);
    std::cout << "Median: " << mf1.findMedian() << std::endl; // Expected: 2.0
    std::cout << std::endl;

    // Test Case 2: Negative numbers
    std::cout << "Test Case 2:" << std::endl;
    MedianFinder mf2;
    mf2.addNum(-1);
    std::cout << "Median: " << mf2.findMedian() << std::endl; // Expected: -1.0
    mf2.addNum(-2);
    std::cout << "Median: " << mf2.findMedian() << std::endl; // Expected: -1.5
    mf2.addNum(-3);
    std::cout << "Median: " << mf2.findMedian() << std::endl; // Expected: -2.0
    std::cout << std::endl;
    
    // Test Case 3: Zero
    std::cout << "Test Case 3:" << std::endl;
    MedianFinder mf3;
    mf3.addNum(0);
    std::cout << "Median: " << mf3.findMedian() << std::endl; // Expected: 0.0
    mf3.addNum(0);
    std::cout << "Median: " << mf3.findMedian() << std::endl; // Expected: 0.0
    std::cout << std::endl;

    // Test Case 4: Alternating large and small numbers
    std::cout << "Test Case 4:" << std::endl;
    MedianFinder mf4;
    mf4.addNum(10);
    mf4.addNum(1);
    mf4.addNum(100);
    mf4.addNum(5);
    // Nums: [1, 5, 10, 100], Median: (5+10)/2 = 7.5
    std::cout << "Median: " << mf4.findMedian() << std::endl;
    std::cout << std::endl;

    // Test Case 5: Descending order
    std::cout << "Test Case 5:" << std::endl;
    MedianFinder mf5;
    mf5.addNum(5);
    mf5.addNum(4);
    mf5.addNum(3);
    mf5.addNum(2);
    mf5.addNum(1);
    // Nums: [1, 2, 3, 4, 5], Median: 3
    std::cout << "Median: " << mf5.findMedian() << std::endl;
    std::cout << std::endl;

    return 0;
}
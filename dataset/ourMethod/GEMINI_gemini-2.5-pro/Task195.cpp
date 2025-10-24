#include <iostream>
#include <vector>
#include <queue>
#include <functional>
#include <iomanip>

class MedianFinder {
private:
    // lower_half is a max-heap
    std::priority_queue<int> lower_half;
    // upper_half is a min-heap
    std::priority_queue<int, std::vector<int>, std::greater<int>> upper_half;

public:
    /** initialize your data structure here. */
    MedianFinder() {
    }
    
    void addNum(int num) {
        lower_half.push(num);
        upper_half.push(lower_half.top());
        lower_half.pop();

        if (lower_half.size() < upper_half.size()) {
            lower_half.push(upper_half.top());
            upper_half.pop();
        }
    }
    
    double findMedian() {
        if (lower_half.size() > upper_half.size()) {
            return static_cast<double>(lower_half.top());
        } else {
            // Problem guarantees this is called on a non-empty structure.
            return (static_cast<double>(lower_half.top()) + upper_half.top()) / 2.0;
        }
    }
};

int main() {
    std::cout << std::fixed << std::setprecision(5);

    // Test Case 1: Example from prompt
    std::cout << "Test Case 1:" << std::endl;
    MedianFinder mf1;
    mf1.addNum(1);
    mf1.addNum(2);
    std::cout << "Median: " << mf1.findMedian() << std::endl; // Expected: 1.50000
    mf1.addNum(3);
    std::cout << "Median: " << mf1.findMedian() << std::endl; // Expected: 2.00000
    std::cout << std::endl;

    // Test Case 2: Negative numbers
    std::cout << "Test Case 2:" << std::endl;
    MedianFinder mf2;
    mf2.addNum(-1);
    std::cout << "Median: " << mf2.findMedian() << std::endl; // Expected: -1.00000
    mf2.addNum(-2);
    std::cout << "Median: " << mf2.findMedian() << std::endl; // Expected: -1.50000
    mf2.addNum(-3);
    std::cout << "Median: " << mf2.findMedian() << std::endl; // Expected: -2.00000
    std::cout << std::endl;

    // Test Case 3: Zeros
    std::cout << "Test Case 3:" << std::endl;
    MedianFinder mf3;
    mf3.addNum(0);
    mf3.addNum(0);
    std::cout << "Median: " << mf3.findMedian() << std::endl; // Expected: 0.00000
    mf3.addNum(0);
    std::cout << "Median: " << mf3.findMedian() << std::endl; // Expected: 0.00000
    std::cout << std::endl;

    // Test Case 4: Mixed numbers
    std::cout << "Test Case 4:" << std::endl;
    MedianFinder mf4;
    mf4.addNum(6);
    std::cout << "Median: " << mf4.findMedian() << std::endl; // Expected: 6.00000
    mf4.addNum(10);
    std::cout << "Median: " << mf4.findMedian() << std::endl; // Expected: 8.00000
    mf4.addNum(2);
    std::cout << "Median: " << mf4.findMedian() << std::endl; // Expected: 6.00000
    mf4.addNum(6);
    std::cout << "Median: " << mf4.findMedian() << std::endl; // Expected: 6.00000
    mf4.addNum(5);
    std::cout << "Median: " << mf4.findMedian() << std::endl; // Expected: 6.00000
    mf4.addNum(0);
    std::cout << "Median: " << mf4.findMedian() << std::endl; // Expected: 5.50000
    std::cout << std::endl;

    // Test Case 5: Descending order
    std::cout << "Test Case 5:" << std::endl;
    MedianFinder mf5;
    mf5.addNum(10);
    mf5.addNum(9);
    mf5.addNum(8);
    mf5.addNum(7);
    mf5.addNum(6);
    std::cout << "Median: " << mf5.findMedian() << std::endl; // Expected: 8.00000
    mf5.addNum(5);
    std::cout << "Median: " << mf5.findMedian() << std::endl; // Expected: 7.50000
    std::cout << std::endl;

    return 0;
}
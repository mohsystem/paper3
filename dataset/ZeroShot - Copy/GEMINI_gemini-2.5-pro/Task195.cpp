#include <iostream>
#include <vector>
#include <queue>
#include <functional>
#include <iomanip>

class MedianFinder {
private:
    // max-heap for the smaller half
    std::priority_queue<int> lowerHalf;
    // min-heap for the larger half
    std::priority_queue<int, std::vector<int>, std::greater<int>> upperHalf;

public:
    MedianFinder() {
    }
    
    void addNum(int num) {
        // Add to max-heap
        lowerHalf.push(num);
        
        // Move the largest element from lower half to upper half
        upperHalf.push(lowerHalf.top());
        lowerHalf.pop();

        // Balance the heaps if upper half becomes larger
        if (upperHalf.size() > lowerHalf.size()) {
            lowerHalf.push(upperHalf.top());
            upperHalf.pop();
        }
    }
    
    double findMedian() {
        // If total numbers are odd, median is the top of the lower half
        if (lowerHalf.size() > upperHalf.size()) {
            return lowerHalf.top();
        } else {
            // If total numbers are even, median is the average of the tops
            // Cast to double to ensure floating-point division
            return (static_cast<double>(lowerHalf.top()) + upperHalf.top()) / 2.0;
        }
    }
};

int main() {
    // Set precision for floating point output
    std::cout << std::fixed << std::setprecision(1);

    // Test Case 1
    std::cout << "Test Case 1:" << std::endl;
    MedianFinder medianFinder1;
    medianFinder1.addNum(1);
    medianFinder1.addNum(2);
    std::cout << "Median: " << medianFinder1.findMedian() << std::endl; // Output: 1.5
    medianFinder1.addNum(3);
    std::cout << "Median: " << medianFinder1.findMedian() << std::endl; // Output: 2.0
    std::cout << std::endl;

    // Test Case 2
    std::cout << "Test Case 2:" << std::endl;
    MedianFinder medianFinder2;
    medianFinder2.addNum(6);
    std::cout << "Median: " << medianFinder2.findMedian() << std::endl; // Output: 6.0
    medianFinder2.addNum(10);
    std::cout << "Median: " << medianFinder2.findMedian() << std::endl; // Output: 8.0
    std::cout << std::endl;
    
    // Test Case 3
    std::cout << "Test Case 3:" << std::endl;
    MedianFinder medianFinder3;
    medianFinder3.addNum(1);
    medianFinder3.addNum(2);
    medianFinder3.addNum(3);
    std::cout << "Median: " << medianFinder3.findMedian() << std::endl; // Output: 2.0
    std::cout << std::endl;
    
    // Test Case 4
    std::cout << "Test Case 4:" << std::endl;
    MedianFinder medianFinder4;
    medianFinder4.addNum(-1);
    medianFinder4.addNum(-2);
    std::cout << "Median: " << medianFinder4.findMedian() << std::endl; // Output: -1.5
    medianFinder4.addNum(-3);
    std::cout << "Median: " << medianFinder4.findMedian() << std::endl; // Output: -2.0
    medianFinder4.addNum(-4);
    std::cout << "Median: " << medianFinder4.findMedian() << std::endl; // Output: -2.5
    std::cout << std::endl;
    
    // Test Case 5
    std::cout << "Test Case 5:" << std::endl;
    MedianFinder medianFinder5;
    medianFinder5.addNum(0);
    std::cout << "Median: " << medianFinder5.findMedian() << std::endl; // Output: 0.0
    std::cout << std::endl;
    
    return 0;
}
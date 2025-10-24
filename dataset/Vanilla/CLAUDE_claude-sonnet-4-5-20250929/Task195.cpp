
#include <iostream>
#include <queue>
#include <vector>

using namespace std;

class MedianFinder {
private:
    priority_queue<int> maxHeap; // left half (max heap)
    priority_queue<int, vector<int>, greater<int>> minHeap; // right half (min heap)

public:
    MedianFinder() {}
    
    void addNum(int num) {
        if (maxHeap.empty() || num <= maxHeap.top()) {
            maxHeap.push(num);
        } else {
            minHeap.push(num);
        }
        
        // Balance the heaps
        if (maxHeap.size() > minHeap.size() + 1) {
            minHeap.push(maxHeap.top());
            maxHeap.pop();
        } else if (minHeap.size() > maxHeap.size()) {
            maxHeap.push(minHeap.top());
            minHeap.pop();
        }
    }
    
    double findMedian() {
        if (maxHeap.size() == minHeap.size()) {
            return (maxHeap.top() + minHeap.top()) / 2.0;
        } else {
            return maxHeap.top();
        }
    }
};

int main() {
    // Test case 1
    MedianFinder mf1;
    mf1.addNum(1);
    mf1.addNum(2);
    cout << "Test 1: " << mf1.findMedian() << endl; // 1.5
    mf1.addNum(3);
    cout << "Test 1: " << mf1.findMedian() << endl; // 2.0

    // Test case 2
    MedianFinder mf2;
    mf2.addNum(5);
    cout << "Test 2: " << mf2.findMedian() << endl; // 5.0

    // Test case 3
    MedianFinder mf3;
    mf3.addNum(10);
    mf3.addNum(20);
    mf3.addNum(30);
    mf3.addNum(40);
    cout << "Test 3: " << mf3.findMedian() << endl; // 25.0

    // Test case 4
    MedianFinder mf4;
    mf4.addNum(-1);
    mf4.addNum(-2);
    mf4.addNum(-3);
    cout << "Test 4: " << mf4.findMedian() << endl; // -2.0

    // Test case 5
    MedianFinder mf5;
    mf5.addNum(6);
    mf5.addNum(10);
    mf5.addNum(2);
    mf5.addNum(6);
    mf5.addNum(5);
    cout << "Test 5: " << mf5.findMedian() << endl; // 6.0

    return 0;
}

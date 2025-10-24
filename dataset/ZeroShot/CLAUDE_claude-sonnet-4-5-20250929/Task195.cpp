
#include <iostream>
#include <queue>
#include <vector>
using namespace std;

class Task195 {
private:
    priority_queue<int> maxHeap; // stores smaller half
    priority_queue<int, vector<int>, greater<int>> minHeap; // stores larger half
    
public:
    Task195() {}
    
    void addNum(int num) {
        if (maxHeap.empty() || num <= maxHeap.top()) {
            maxHeap.push(num);
        } else {
            minHeap.push(num);
        }
        
        // Balance heaps
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
        }
        return maxHeap.top();
    }
};

int main() {
    // Test case 1
    Task195 mf1;
    mf1.addNum(1);
    mf1.addNum(2);
    cout << "Test 1: " << mf1.findMedian() << endl; // 1.5
    mf1.addNum(3);
    cout << "Test 1: " << mf1.findMedian() << endl; // 2.0
    
    // Test case 2
    Task195 mf2;
    mf2.addNum(5);
    cout << "Test 2: " << mf2.findMedian() << endl; // 5.0
    
    // Test case 3
    Task195 mf3;
    mf3.addNum(6);
    mf3.addNum(10);
    mf3.addNum(2);
    mf3.addNum(6);
    mf3.addNum(5);
    cout << "Test 3: " << mf3.findMedian() << endl; // 6.0
    
    // Test case 4
    Task195 mf4;
    mf4.addNum(-1);
    mf4.addNum(-2);
    mf4.addNum(-3);
    mf4.addNum(-4);
    cout << "Test 4: " << mf4.findMedian() << endl; // -2.5
    
    // Test case 5
    Task195 mf5;
    mf5.addNum(1);
    mf5.addNum(2);
    mf5.addNum(3);
    mf5.addNum(4);
    mf5.addNum(5);
    cout << "Test 5: " << mf5.findMedian() << endl; // 3.0
    
    return 0;
}

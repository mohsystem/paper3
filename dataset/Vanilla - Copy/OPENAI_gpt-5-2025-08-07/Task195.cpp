#include <bits/stdc++.h>
using namespace std;

class MedianFinder {
private:
    priority_queue<int> low; // max-heap
    priority_queue<int, vector<int>, greater<int>> high; // min-heap
public:
    MedianFinder() {}

    void addNum(int num) {
        if (low.empty() || num <= low.top()) {
            low.push(num);
        } else {
            high.push(num);
        }

        // Balance heaps: low.size() >= high.size() and diff <= 1
        if (low.size() < high.size()) {
            low.push(high.top());
            high.pop();
        } else if (low.size() > high.size() + 1) {
            high.push(low.top());
            low.pop();
        }
    }

    double findMedian() {
        if (low.size() == high.size()) {
            return (low.top() + high.top()) / 2.0;
        }
        return (double)low.top();
    }
};

int main() {
    cout.setf(std::ios::fixed);
    cout << setprecision(6);

    // Test case 1 (Example)
    MedianFinder mf1;
    mf1.addNum(1);
    mf1.addNum(2);
    cout << mf1.findMedian() << "\n"; // 1.5
    mf1.addNum(3);
    cout << mf1.findMedian() << "\n"; // 2.0

    // Test case 2
    MedianFinder mf2;
    mf2.addNum(2);
    mf2.addNum(3);
    mf2.addNum(4);
    cout << mf2.findMedian() << "\n"; // 3.0

    // Test case 3
    MedianFinder mf3;
    mf3.addNum(2);
    mf3.addNum(3);
    cout << mf3.findMedian() << "\n"; // 2.5

    // Test case 4 (negatives)
    MedianFinder mf4;
    mf4.addNum(-1);
    mf4.addNum(-2);
    mf4.addNum(-3);
    mf4.addNum(-4);
    mf4.addNum(-5);
    cout << mf4.findMedian() << "\n"; // -3.0

    // Test case 5 (dynamic)
    MedianFinder mf5;
    mf5.addNum(5);
    cout << mf5.findMedian() << "\n"; // 5.0
    mf5.addNum(15);
    cout << mf5.findMedian() << "\n"; // 10.0
    mf5.addNum(1);
    cout << mf5.findMedian() << "\n"; // 5.0
    mf5.addNum(3);
    cout << mf5.findMedian() << "\n"; // 4.0

    return 0;
}
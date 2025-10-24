#include <bits/stdc++.h>
using namespace std;

class MedianFinder {
private:
    priority_queue<int> lower;                                   // max-heap
    priority_queue<int, vector<int>, greater<int>> upper;        // min-heap

    void rebalance() {
        if (static_cast<int>(lower.size()) > static_cast<int>(upper.size()) + 1) {
            upper.push(lower.top());
            lower.pop();
        } else if (upper.size() > lower.size()) {
            lower.push(upper.top());
            upper.pop();
        }
    }

public:
    MedianFinder() = default;

    void addNum(int num) {
        if (num < -100000 || num > 100000) {
            throw invalid_argument("num out of allowed range");
        }
        if (lower.empty() || num <= lower.top()) {
            lower.push(num);
        } else {
            upper.push(num);
        }
        rebalance();
    }

    double findMedian() {
        if (lower.empty() && upper.empty()) {
            throw runtime_error("No elements present");
        }
        if (lower.size() > upper.size()) {
            return static_cast<double>(lower.top());
        } else {
            return (static_cast<double>(lower.top()) + static_cast<double>(upper.top())) / 2.0;
        }
    }
};

int main() {
    cout.setf(std::ios::fixed);
    cout << setprecision(6);

    // Test 1
    MedianFinder mf1;
    mf1.addNum(1);
    mf1.addNum(2);
    cout << mf1.findMedian() << "\n"; // 1.5
    mf1.addNum(3);
    cout << mf1.findMedian() << "\n"; // 2.0

    // Test 2
    MedianFinder mf2;
    mf2.addNum(2);
    mf2.addNum(3);
    mf2.addNum(4);
    cout << mf2.findMedian() << "\n"; // 3.0

    // Test 3
    MedianFinder mf3;
    mf3.addNum(2);
    mf3.addNum(3);
    cout << mf3.findMedian() << "\n"; // 2.5

    // Test 4
    MedianFinder mf4;
    mf4.addNum(-5);
    mf4.addNum(-1);
    mf4.addNum(-3);
    mf4.addNum(-4);
    cout << mf4.findMedian() << "\n"; // -3.5

    // Test 5
    MedianFinder mf5;
    mf5.addNum(5);
    mf5.addNum(10);
    mf5.addNum(-1);
    mf5.addNum(20);
    mf5.addNum(3);
    cout << mf5.findMedian() << "\n"; // 5.0

    return 0;
}
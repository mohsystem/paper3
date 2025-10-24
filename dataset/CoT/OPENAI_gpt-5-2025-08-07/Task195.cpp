#include <bits/stdc++.h>
using namespace std;

class MedianFinder {
private:
    priority_queue<int> lower; // max-heap
    priority_queue<int, vector<int>, greater<int>> upper; // min-heap
public:
    MedianFinder() = default;

    void addNum(int num) {
        lower.push(num);
        // Move largest from lower to upper
        upper.push(lower.top());
        lower.pop();
        // Ensure lower has equal or one more element than upper
        if (upper.size() > lower.size()) {
            lower.push(upper.top());
            upper.pop();
        }
    }

    double findMedian() const {
        if (lower.empty()) return 0.0; // defensive
        if (lower.size() > upper.size()) return static_cast<double>(lower.top());
        return (lower.top() + upper.top()) / 2.0;
    }
};

static void run_test1() {
    MedianFinder mf;
    mf.addNum(1);
    mf.addNum(2);
    cout << mf.findMedian() << "\n"; // 1.5
    mf.addNum(3);
    cout << mf.findMedian() << "\n"; // 2.0
}

static void run_test2() {
    MedianFinder mf;
    mf.addNum(2);
    mf.addNum(3);
    mf.addNum(4);
    cout << mf.findMedian() << "\n"; // 3.0
}

static void run_test3() {
    MedianFinder mf;
    mf.addNum(2);
    mf.addNum(3);
    cout << mf.findMedian() << "\n"; // 2.5
}

static void run_test4() {
    MedianFinder mf;
    mf.addNum(-100000);
    cout << mf.findMedian() << "\n"; // -100000.0
}

static void run_test5() {
    MedianFinder mf;
    mf.addNum(5);
    cout << mf.findMedian() << "\n"; // 5.0
    mf.addNum(15);
    cout << mf.findMedian() << "\n"; // 10.0
    mf.addNum(1);
    cout << mf.findMedian() << "\n"; // 5.0
    mf.addNum(3);
    cout << mf.findMedian() << "\n"; // 4.0
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    run_test1();
    run_test2();
    run_test3();
    run_test4();
    run_test5();
    return 0;
}
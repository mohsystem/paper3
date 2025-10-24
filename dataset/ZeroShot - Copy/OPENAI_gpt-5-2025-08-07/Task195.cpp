#include <bits/stdc++.h>
using namespace std;

class MedianFinder {
private:
    priority_queue<int> lower;                           // max-heap
    priority_queue<int, vector<int>, greater<int>> upper; // min-heap

public:
    MedianFinder() = default;

    void addNum(int num) {
        lower.push(num);
        upper.push(lower.top());
        lower.pop();
        if (upper.size() > lower.size()) {
            lower.push(upper.top());
            upper.pop();
        }
    }

    double findMedian() {
        if (lower.empty()) {
            throw runtime_error("No elements to find median.");
        }
        if (lower.size() == upper.size()) {
            return (static_cast<double>(lower.top()) + static_cast<double>(upper.top())) / 2.0;
        } else {
            return static_cast<double>(lower.top());
        }
    }
};

class Task195 {
public:
    static void runTest(const string& name, const vector<int>& ops, const vector<int>& vals, const vector<double>& expected) {
        MedianFinder mf;
        vector<double> results;
        size_t vi = 0;
        for (int op : ops) {
            if (op == 0) {
                if (vi >= vals.size()) throw runtime_error("Value index out of range.");
                mf.addNum(vals[vi++]);
            } else if (op == 1) {
                results.push_back(mf.findMedian());
            }
        }
        cout << name << " results: [";
        for (size_t i = 0; i < results.size(); ++i) {
            if (i) cout << ", ";
            cout << fixed << setprecision(6) << results[i];
        }
        cout << "]" << endl;
        if (!expected.empty()) {
            cout << name << " expected: [";
            for (size_t i = 0; i < expected.size(); ++i) {
                if (i) cout << ", ";
                cout << fixed << setprecision(6) << expected[i];
            }
            cout << "]" << endl;
        }
        cout << "----" << endl;
    }
};

int main() {
    // Test 1: Example
    Task195::runTest("Test1", {0,0,1,0,1}, {1,2,3}, {1.5, 2.0});
    // Test 2: Single element
    Task195::runTest("Test2", {0,1}, {-100000}, {-100000.0});
    // Test 3: Even count increasing sequence
    Task195::runTest("Test3", {0,0,0,0,1}, {5,10,15,20}, {12.5});
    // Test 4: Duplicates
    Task195::runTest("Test4", {0,0,0,0,0,1}, {2,2,2,3,4}, {2.0});
    // Test 5: Mixed negatives and positives
    Task195::runTest("Test5", {0,0,0,0,1}, {-5,-1,-3,7}, {-2.0});
    return 0;
}
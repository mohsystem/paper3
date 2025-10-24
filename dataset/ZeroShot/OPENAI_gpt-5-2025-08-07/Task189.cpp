#include <bits/stdc++.h>
using namespace std;

class ZeroEvenOdd {
public:
    explicit ZeroEvenOdd(int n_) : n(n_), state(0) {}

    void zero(function<void(int)> printNumber) {
        for (int i = 1; i <= n; ++i) {
            unique_lock<mutex> lk(mtx);
            cv.wait(lk, [&]{ return state == 0; });
            printNumber(0);
            state = (i % 2 == 1) ? 1 : 2;
            lk.unlock();
            cv.notify_all();
        }
    }

    void even(function<void(int)> printNumber) {
        for (int i = 2; i <= n; i += 2) {
            unique_lock<mutex> lk(mtx);
            cv.wait(lk, [&]{ return state == 2; });
            printNumber(i);
            state = 0;
            lk.unlock();
            cv.notify_all();
        }
    }

    void odd(function<void(int)> printNumber) {
        for (int i = 1; i <= n; i += 2) {
            unique_lock<mutex> lk(mtx);
            cv.wait(lk, [&]{ return state == 1; });
            printNumber(i);
            state = 0;
            lk.unlock();
            cv.notify_all();
        }
    }

private:
    int n;
    mutex mtx;
    condition_variable cv;
    int state; // 0: zero's turn, 1: odd's turn, 2: even's turn
};

static string runZeroEvenOdd(int n) {
    if (n < 1 || n > 1000) return "";
    ZeroEvenOdd zeo(n);
    string out;
    out.reserve(n * 5);
    mutex out_mtx;
    auto printer = [&](int x) {
        lock_guard<mutex> g(out_mtx);
        out += to_string(x);
    };
    thread t0([&]{ zeo.zero(printer); });
    thread t1([&]{ zeo.odd(printer); });
    thread t2([&]{ zeo.even(printer); });
    t0.join(); t1.join(); t2.join();
    return out;
}

int main() {
    vector<int> tests = {1, 2, 5, 7, 10};
    for (int n : tests) {
        cout << runZeroEvenOdd(n) << "\n";
    }
    return 0;
}
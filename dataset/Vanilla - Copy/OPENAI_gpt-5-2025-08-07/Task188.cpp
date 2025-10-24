#include <bits/stdc++.h>
using namespace std;

class FooBar {
public:
    int n;
    mutex m;
    condition_variable cv;
    bool fooTurn = true;
    string out;

    FooBar(int n) : n(n) {}

    void foo() {
        for (int i = 0; i < n; ++i) {
            unique_lock<mutex> lk(m);
            cv.wait(lk, [&]{ return fooTurn; });
            out += "foo";
            fooTurn = false;
            lk.unlock();
            cv.notify_all();
        }
    }

    void bar() {
        for (int i = 0; i < n; ++i) {
            unique_lock<mutex> lk(m);
            cv.wait(lk, [&]{ return !fooTurn; });
            out += "bar";
            fooTurn = true;
            lk.unlock();
            cv.notify_all();
        }
    }
};

string solve(int n) {
    FooBar fb(n);
    thread t1(&FooBar::foo, &fb);
    thread t2(&FooBar::bar, &fb);
    t1.join();
    t2.join();
    return fb.out;
}

int main() {
    vector<int> tests = {1, 2, 3, 4, 5};
    for (int n : tests) {
        cout << solve(n) << "\n";
    }
    return 0;
}
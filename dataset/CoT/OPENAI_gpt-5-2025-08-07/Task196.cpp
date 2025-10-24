#include <bits/stdc++.h>
using namespace std;

class ZeroEvenOdd {
private:
    int n;
    int curr;
    int turn; // 0 -> zero's turn, 1 -> number's turn
    mutex mtx;
    condition_variable cv;
    string buf;
    void printNumber(int x) {
        cout << x;
        // buffer append safely under lock by caller
        buf += to_string(x);
    }
public:
    explicit ZeroEvenOdd(int n_) : n(n_), curr(1), turn(0), buf() {
        buf.reserve(n * 5);
    }

    void zero() {
        for (int i = 1; i <= n; ++i) {
            unique_lock<mutex> lk(mtx);
            cv.wait(lk, [&]{ return turn == 0; });
            printNumber(0);
            turn = 1;
            lk.unlock();
            cv.notify_all();
        }
    }

    void even() {
        while (true) {
            unique_lock<mutex> lk(mtx);
            cv.wait(lk, [&]{ return (turn == 1 && curr % 2 == 0) || curr > n; });
            if (curr > n) {
                lk.unlock();
                cv.notify_all();
                return;
            }
            printNumber(curr);
            ++curr;
            turn = 0;
            lk.unlock();
            cv.notify_all();
        }
    }

    void odd() {
        while (true) {
            unique_lock<mutex> lk(mtx);
            cv.wait(lk, [&]{ return (turn == 1 && curr % 2 == 1) || curr > n; });
            if (curr > n) {
                lk.unlock();
                cv.notify_all();
                return;
            }
            printNumber(curr);
            ++curr;
            turn = 0;
            lk.unlock();
            cv.notify_all();
        }
    }

    string run() {
        thread t0(&ZeroEvenOdd::zero, this);
        thread t1(&ZeroEvenOdd::even, this);
        thread t2(&ZeroEvenOdd::odd, this);
        t0.join(); t1.join(); t2.join();
        return buf;
    }
};

string run_zero_even_odd(int n) {
    ZeroEvenOdd z(n);
    return z.run();
}

int main() {
    vector<int> tests = {1, 2, 5, 10, 3};
    for (int n : tests) {
        run_zero_even_odd(n);
        cout << "\n";
    }
    return 0;
}
#include <bits/stdc++.h>
using namespace std;

class Semaphore {
private:
    mutex mtx;
    condition_variable cv;
    int permits;
public:
    explicit Semaphore(int initial) : permits(initial) {}
    void acquire() {
        unique_lock<mutex> lk(mtx);
        cv.wait(lk, [&]{ return permits > 0; });
        --permits;
    }
    void release() {
        unique_lock<mutex> lk(mtx);
        ++permits;
        cv.notify_one();
    }
};

class ReusableBarrier {
private:
    mutex mtx;
    condition_variable cv;
    const int threshold;
    int count;
    int generation;
public:
    explicit ReusableBarrier(int n) : threshold(n), count(0), generation(0) {}
    void wait() {
        unique_lock<mutex> lk(mtx);
        int gen = generation;
        if (++count == threshold) {
            generation++;
            count = 0;
            cv.notify_all();
        } else {
            cv.wait(lk, [&]{ return gen != generation; });
        }
    }
};

class H2O {
private:
    Semaphore hSem;
    Semaphore oSem;
    ReusableBarrier barrier;
public:
    H2O() : hSem(2), oSem(1), barrier(3) {}

    void hydrogen(function<void()> releaseHydrogen) {
        hSem.acquire();
        releaseHydrogen();
        barrier.wait();
        hSem.release();
    }

    void oxygen(function<void()> releaseOxygen) {
        oSem.acquire();
        releaseOxygen();
        barrier.wait();
        oSem.release();
    }
};

string run_water(const string& water) {
    H2O h2o;
    string out;
    out.reserve(water.size());
    mutex out_mtx;
    vector<thread> threads;
    auto relH = [&]() {
        lock_guard<mutex> g(out_mtx);
        out.push_back('H');
    };
    auto relO = [&]() {
        lock_guard<mutex> g(out_mtx);
        out.push_back('O');
    };

    for (char c : water) {
        if (c == 'H') {
            threads.emplace_back([&](){ h2o.hydrogen(relH); });
        } else if (c == 'O') {
            threads.emplace_back([&](){ h2o.oxygen(relO); });
        } else {
            // ignore
        }
    }
    for (auto& t : threads) t.join();
    return out;
}

bool validate_water(const string& out) {
    if (out.size() % 3 != 0) return false;
    for (size_t i = 0; i < out.size(); i += 3) {
        int h = 0, o = 0;
        for (size_t j = i; j < i + 3; ++j) {
            if (out[j] == 'H') h++;
            else if (out[j] == 'O') o++;
            else return false;
        }
        if (!(h == 2 && o == 1)) return false;
    }
    return true;
}

int main() {
    vector<string> tests = {
        "HOH",
        "OOHHHH",
        "HHOHHO",
        "OHHHOH",
        "HHOOHH"
    };
    for (auto& s : tests) {
        string out = run_water(s);
        cout << "Input:  " << s << "\n";
        cout << "Output: " << out << " | valid=" << (validate_water(out) ? "true" : "false") << "\n";
        cout << "---\n";
    }
    return 0;
}
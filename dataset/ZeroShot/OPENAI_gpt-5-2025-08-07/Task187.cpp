#include <bits/stdc++.h>
using namespace std;

class H2O {
private:
    mutex mtx;
    condition_variable cvEnter, cvBarrier;
    int hSlots = 2;
    int oSlots = 1;
    int arrived = 0;
    int gen = 0;

public:
    H2O() = default;

    void hydrogen(function<void()> releaseHydrogen) {
        int myGen;
        {
            unique_lock<mutex> lk(mtx);
            cvEnter.wait(lk, [&]{ return hSlots > 0; });
            --hSlots;
            myGen = gen;
        }

        // Bond (print) before barrier
        releaseHydrogen();

        {
            unique_lock<mutex> lk(mtx);
            ++arrived;
            if (arrived == 3) {
                // Reset for next molecule
                arrived = 0;
                ++gen;
                hSlots = 2;
                oSlots = 1;
                cvEnter.notify_all();
                cvBarrier.notify_all();
            } else {
                cvBarrier.wait(lk, [&]{ return gen != myGen; });
            }
        }
    }

    void oxygen(function<void()> releaseOxygen) {
        int myGen;
        {
            unique_lock<mutex> lk(mtx);
            cvEnter.wait(lk, [&]{ return oSlots > 0; });
            --oSlots;
            myGen = gen;
        }

        // Bond (print) before barrier
        releaseOxygen();

        {
            unique_lock<mutex> lk(mtx);
            ++arrived;
            if (arrived == 3) {
                arrived = 0;
                ++gen;
                hSlots = 2;
                oSlots = 1;
                cvEnter.notify_all();
                cvBarrier.notify_all();
            } else {
                cvBarrier.wait(lk, [&]{ return gen != myGen; });
            }
        }
    }
};

static string buildWater(const string& water) {
    H2O h2o;
    string out;
    out.reserve(water.size());
    mutex out_mtx;

    auto releaseH = [&](){
        lock_guard<mutex> g(out_mtx);
        out.push_back('H');
    };
    auto releaseO = [&](){
        lock_guard<mutex> g(out_mtx);
        out.push_back('O');
    };

    vector<thread> threads;
    threads.reserve(water.size());
    for (size_t i = 0; i < water.size(); ++i) {
        char c = water[i];
        if (c == 'H') {
            threads.emplace_back([&](){ h2o.hydrogen(releaseH); });
        } else if (c == 'O') {
            threads.emplace_back([&](){ h2o.oxygen(releaseO); });
        }
    }
    for (auto& t : threads) {
        if (t.joinable()) t.join();
    }
    return out;
}

static bool isValidWater(const string& s) {
    if (s.size() % 3 != 0) return false;
    for (size_t i = 0; i < s.size(); i += 3) {
        int h = 0, o = 0;
        for (int j = 0; j < 3; ++j) {
            if (s[i + j] == 'H') ++h;
            else if (s[i + j] == 'O') ++o;
            else return false;
        }
        if (h != 2 || o != 1) return false;
    }
    return true;
}

int main() {
    vector<string> tests = {
        "HOH",
        "OOHHHH",
        "HHOOHH",
        "OHOHHH",
        "HHOHHO"
    };
    for (auto& in : tests) {
        string out = buildWater(in);
        cout << "in=" << in << " out=" << out << " valid=" << (isValidWater(out) ? "true" : "false") << "\n";
    }
    return 0;
}
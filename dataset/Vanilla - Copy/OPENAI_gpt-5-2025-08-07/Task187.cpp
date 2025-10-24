#include <bits/stdc++.h>
#include <thread>
#include <mutex>
#include <condition_variable>

class Semaphore {
public:
    explicit Semaphore(int permits = 0) : permits_(permits) {}
    void acquire() {
        std::unique_lock<std::mutex> lk(m_);
        cv_.wait(lk, [&]{ return permits_ > 0; });
        --permits_;
    }
    void release(int n = 1) {
        std::unique_lock<std::mutex> lk(m_);
        permits_ += n;
        for (int i = 0; i < n; ++i) cv_.notify_one();
    }
private:
    int permits_;
    std::mutex m_;
    std::condition_variable cv_;
};

class H2O {
public:
    H2O(): hSlots(2), oSlots(1), turnstile(0), turnstile2(1), count(0) {}

    void hydrogen(std::function<void()> releaseHydrogen) {
        hSlots.acquire();
        bool leader = barrier_phase1();
        releaseHydrogen();
        barrier_phase2();
        if (leader) {
            hSlots.release(2);
            oSlots.release(1);
        }
    }

    void oxygen(std::function<void()> releaseOxygen) {
        oSlots.acquire();
        bool leader = barrier_phase1();
        releaseOxygen();
        barrier_phase2();
        if (leader) {
            hSlots.release(2);
            oSlots.release(1);
        }
    }

private:
    Semaphore hSlots;
    Semaphore oSlots;

    std::mutex m;
    int count;
    Semaphore turnstile;
    Semaphore turnstile2;

    bool barrier_phase1() {
        bool leader = false;
        {
            std::lock_guard<std::mutex> lk(m);
            count++;
            if (count == 3) {
                turnstile2.acquire();
                turnstile.release(3);
                leader = true;
            }
        }
        turnstile.acquire();
        turnstile.release();
        return leader;
    }

    void barrier_phase2() {
        {
            std::lock_guard<std::mutex> lk(m);
            count--;
            if (count == 0) {
                turnstile.acquire();
                turnstile2.release(3);
            }
        }
        turnstile2.acquire();
        turnstile2.release();
    }
};

std::string make_water(const std::string& water) {
    H2O h2o;
    std::string out;
    out.reserve(water.size());
    std::mutex out_m;
    std::vector<std::thread> threads;

    auto relH = [&](){
        std::lock_guard<std::mutex> lk(out_m);
        out.push_back('H');
    };
    auto relO = [&](){
        std::lock_guard<std::mutex> lk(out_m);
        out.push_back('O');
    };

    for (char c : water) {
        if (c == 'H') {
            threads.emplace_back([&](){ h2o.hydrogen(relH); });
        } else {
            threads.emplace_back([&](){ h2o.oxygen(relO); });
        }
    }
    for (auto& t : threads) t.join();
    return out;
}

int main() {
    std::vector<std::string> tests = {
        "HOH",
        "OOHHHH",
        "HOHOHO",
        "HHOHHO",
        "OOOHHHHHH"
    };
    for (auto& s : tests) {
        std::string res = make_water(s);
        std::cout << "Input: " << s << " -> Output: " << res << "\n";
    }
    return 0;
}
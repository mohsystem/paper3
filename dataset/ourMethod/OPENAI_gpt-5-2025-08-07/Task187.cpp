#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>

class Semaphore {
public:
    explicit Semaphore(int permits) : permits_(permits) {}
    void acquire() {
        std::unique_lock<std::mutex> lk(m_);
        cv_.wait(lk, [&]{ return permits_ > 0; });
        --permits_;
    }
    void release(int n = 1) {
        std::unique_lock<std::mutex> lk(m_);
        permits_ += n;
        lk.unlock();
        cv_.notify_all();
    }
private:
    int permits_;
    std::mutex m_;
    std::condition_variable cv_;
};

class Barrier {
public:
    explicit Barrier(int total) : total_(total), count_(0), generation_(0) {}
    bool arrive_and_wait() {
        std::unique_lock<std::mutex> lk(m_);
        int gen = generation_;
        ++count_;
        if (count_ == total_) {
            generation_++;
            count_ = 0;
            lk.unlock();
            cv_.notify_all();
            return true; // last thread
        } else {
            cv_.wait(lk, [&]{ return gen != generation_; });
            return false;
        }
    }
private:
    int total_;
    int count_;
    int generation_;
    std::mutex m_;
    std::condition_variable cv_;
};

class H2O {
public:
    H2O() : hSem_(2), oSem_(1), barrier_(3) {}
    void hydrogen(const std::function<void(void)>& releaseHydrogen) {
        hSem_.acquire();
        releaseHydrogen();
        bool last = barrier_.arrive_and_wait();
        if (last) {
            hSem_.release(2);
            oSem_.release(1);
        }
    }
    void oxygen(const std::function<void(void)>& releaseOxygen) {
        oSem_.acquire();
        releaseOxygen();
        bool last = barrier_.arrive_and_wait();
        if (last) {
            hSem_.release(2);
            oSem_.release(1);
        }
    }
private:
    Semaphore hSem_;
    Semaphore oSem_;
    Barrier barrier_;
};

static bool validateMolecules(const std::string& s) {
    if (s.size() % 3 != 0) return false;
    for (size_t i = 0; i < s.size(); i += 3) {
        int h = 0, o = 0;
        for (size_t j = 0; j < 3; ++j) {
            char c = s[i + j];
            if (c == 'H') ++h;
            else if (c == 'O') ++o;
            else return false;
        }
        if (!(h == 2 && o == 1)) return false;
    }
    return true;
}

std::string formWater(const std::string& water) {
    if (water.size() % 3 != 0) return "";
    size_t h = 0, o = 0;
    for (char c : water) {
        if (c == 'H') ++h;
        else if (c == 'O') ++o;
        else return "";
    }
    if (h != 2 * o) return "";

    H2O h2o;
    std::string out;
    out.reserve(water.size());
    std::mutex out_m;

    std::vector<std::thread> threads;
    threads.reserve(water.size());
    for (char c : water) {
        if (c == 'H') {
            threads.emplace_back([&]{
                h2o.hydrogen([&]{
                    std::lock_guard<std::mutex> lg(out_m);
                    out.push_back('H');
                });
            });
        } else {
            threads.emplace_back([&]{
                h2o.oxygen([&]{
                    std::lock_guard<std::mutex> lg(out_m);
                    out.push_back('O');
                });
            });
        }
    }
    for (auto& t : threads) t.join();
    return out;
}

int main() {
    std::vector<std::string> tests = {
        "HOH",
        "OOHHHH",
        "HHOOHH",
        "OOOHHHHHH",
        "HHOHHOHHO"
    };
    for (const auto& t : tests) {
        std::string out = formWater(t);
        std::cout << "Input:  " << t << "\n";
        std::cout << "Output: " << out << "\n";
        std::cout << "Valid:  " << (validateMolecules(out) ? "1" : "0") << "\n";
        std::cout << "---\n";
    }
    return 0;
}
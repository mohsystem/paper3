#include <iostream>
#include <vector>
#include <utility>
#include <stdexcept>

long long peopleOnBus(const std::vector<std::pair<long long, long long>>& stops) {
    long long current = 0;
    for (size_t i = 0; i < stops.size(); ++i) {
        long long on = stops[i].first;
        long long off = stops[i].second;
        if (on < 0 || off < 0) {
            throw std::invalid_argument("Counts cannot be negative");
        }
        if (i == 0 && off != 0) {
            throw std::invalid_argument("First stop must have 0 getting off");
        }
        if (off > current + on) {
            throw std::invalid_argument("More people getting off than available on the bus");
        }
        current = current + on - off;
    }
    if (current < 0) {
        throw std::runtime_error("Resulting count cannot be negative");
    }
    return current;
}

int main() {
    std::vector<std::pair<long long, long long>> case1{{10, 0}, {3, 5}, {5, 8}}; // 5
    std::vector<std::pair<long long, long long>> case2{{3, 0}, {9, 1}, {4, 10}, {12, 2}, {6, 1}, {7, 10}}; // 17
    std::vector<std::pair<long long, long long>> case3{{0, 0}}; // 0
    std::vector<std::pair<long long, long long>> case4{{20, 0}, {0, 10}, {5, 5}}; // 10
    std::vector<std::pair<long long, long long>> case5{{5, 0}, {0, 5}, {5, 0}, {0, 5}}; // 0

    std::cout << peopleOnBus(case1) << "\n";
    std::cout << peopleOnBus(case2) << "\n";
    std::cout << peopleOnBus(case3) << "\n";
    std::cout << peopleOnBus(case4) << "\n";
    std::cout << peopleOnBus(case5) << "\n";
    return 0;
}
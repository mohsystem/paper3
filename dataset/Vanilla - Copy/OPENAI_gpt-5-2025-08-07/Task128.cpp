#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <chrono>

class Task128 {
public:
    static std::vector<int> generateRandomIntList(int count, int minInclusive, int maxInclusive, unsigned long long seed, bool use_seed) {
        if (count < 0) throw std::invalid_argument("count must be >= 0");
        if (minInclusive > maxInclusive) throw std::invalid_argument("minInclusive cannot be greater than maxInclusive");
        std::mt19937_64 rng;
        if (use_seed) {
            rng.seed(seed);
        } else {
            std::random_device rd;
            std::seed_seq seq{rd(), rd(), rd(), rd(), static_cast<unsigned>(std::chrono::high_resolution_clock::now().time_since_epoch().count())};
            rng.seed(seq);
        }
        std::uniform_int_distribution<int> dist(minInclusive, maxInclusive);
        std::vector<int> result;
        result.reserve(count);
        for (int i = 0; i < count; ++i) {
            result.push_back(dist(rng));
        }
        return result;
    }

    static std::string generateToken(int length, const std::string& allowedChars, unsigned long long seed, bool use_seed) {
        if (length < 0) throw std::invalid_argument("length must be >= 0");
        std::string defaultChars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
        const std::string& chars = allowedChars.empty() ? defaultChars : allowedChars;
        std::mt19937_64 rng;
        if (use_seed) {
            rng.seed(seed);
        } else {
            std::random_device rd;
            std::seed_seq seq{rd(), rd(), rd(), rd(), static_cast<unsigned>(std::chrono::high_resolution_clock::now().time_since_epoch().count())};
            rng.seed(seq);
        }
        std::uniform_int_distribution<size_t> dist(0, chars.size() - 1);
        std::string out;
        out.reserve(static_cast<size_t>(length));
        for (int i = 0; i < length; ++i) {
            out.push_back(chars[dist(rng)]);
        }
        return out;
    }
};

int main() {
    try {
        std::cout << "C++ Test Case 1:\n";
        auto v1 = Task128::generateRandomIntList(10, 1, 100, 12345ULL, true);
        for (size_t i = 0; i < v1.size(); ++i) std::cout << (i ? "," : "") << v1[i];
        std::cout << "\n" << Task128::generateToken(16, "", 12345ULL, true) << "\n";

        std::cout << "C++ Test Case 2:\n";
        auto v2 = Task128::generateRandomIntList(5, 0, 9, 0ULL, false);
        for (size_t i = 0; i < v2.size(); ++i) std::cout << (i ? "," : "") << v2[i];
        std::cout << "\n" << Task128::generateToken(8, "", 0ULL, false) << "\n";

        std::cout << "C++ Test Case 3:\n";
        auto v3 = Task128::generateRandomIntList(7, -50, 50, 999ULL, true);
        for (size_t i = 0; i < v3.size(); ++i) std::cout << (i ? "," : "") << v3[i];
        std::cout << "\n" << Task128::generateToken(12, "ABCDEF0123", 42ULL, true) << "\n";

        std::cout << "C++ Test Case 4:\n";
        auto v4 = Task128::generateRandomIntList(3, 1000, 2000, 55555ULL, true);
        for (size_t i = 0; i < v4.size(); ++i) std::cout << (i ? "," : "") << v4[i];
        std::cout << "\n" << Task128::generateToken(24, "abcdef", 55555ULL, true) << "\n";

        std::cout << "C++ Test Case 5:\n";
        auto v5 = Task128::generateRandomIntList(0, 0, 10, 2021ULL, true);
        for (size_t i = 0; i < v5.size(); ++i) std::cout << (i ? "," : "") << v5[i];
        std::cout << "\n[" << Task128::generateToken(0, "", 2021ULL, true) << "]\n";
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
    }
    return 0;
}
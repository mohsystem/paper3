#include <iostream>
#include <array>
#include <string>
#include <random>
#include <cstdio>
#include <stdexcept>

static std::mt19937_64& rng_instance() {
    static std::random_device rd;
    static std::mt19937_64 gen(rd());
    return gen;
}

// Generates three random floats in [0,1) and returns their string representations with given precision.
std::array<std::string, 3> generateThreeRandomFloatStrings(int precision) {
    if (precision < 0 || precision > 9) {
        throw std::invalid_argument("Invalid precision");
    }

    std::uniform_real_distribution<double> dist(0.0, 1.0);
    double v1 = dist(rng_instance());
    double v2 = dist(rng_instance());
    double v3 = dist(rng_instance());

    char buf[64];
    std::array<std::string, 3> out;

    int n = std::snprintf(buf, sizeof(buf), "%.*f", precision, v1);
    if (n < 0 || static_cast<size_t>(n) >= sizeof(buf)) throw std::runtime_error("format error");
    out[0] = std::string(buf, static_cast<size_t>(n));

    n = std::snprintf(buf, sizeof(buf), "%.*f", precision, v2);
    if (n < 0 || static_cast<size_t>(n) >= sizeof(buf)) throw std::runtime_error("format error");
    out[1] = std::string(buf, static_cast<size_t>(n));

    n = std::snprintf(buf, sizeof(buf), "%.*f", precision, v3);
    if (n < 0 || static_cast<size_t>(n) >= sizeof(buf)) throw std::runtime_error("format error");
    out[2] = std::string(buf, static_cast<size_t>(n));

    return out;
}

int main() {
    int tests[5] = {0, 2, 4, 6, 9};
    for (int p : tests) {
        try {
            auto v = generateThreeRandomFloatStrings(p);
            std::cout << "Precision " << p << " -> string: " << v[0]
                      << ", string2: " << v[1]
                      << ", string3: " << v[2] << "\n";
        } catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << "\n";
        }
    }
    return 0;
}
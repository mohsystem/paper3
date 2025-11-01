#include <iostream>
#include <iomanip>
#include <random>
#include <sstream>
#include <string>
#include <vector>
#include <locale>

static std::string a(float f) {
    std::ostringstream oss;
    oss.imbue(std::locale::classic());
    oss.setf(std::ios::fixed);
    oss << std::setprecision(6) << f;
    return oss.str();
}

static std::vector<std::string> generate_n_random_float_strings(int n) {
    if (n <= 0 || n > 1000) {
        throw std::invalid_argument("invalid input");
    }
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    std::vector<std::string> out;
    out.reserve(static_cast<size_t>(n));
    for (int i = 0; i < n; ++i) {
        float v = dist(gen);
        out.push_back(a(v));
    }
    return out;
}

int main() {
    try {
        // Core requirement demonstration
        auto trio = generate_n_random_float_strings(3);
        std::string str_a = trio[0];
        std::string str_b = trio[1];
        std::string str_c = trio[2];
        std::cout << "Test 1: [" << str_a << ", " << str_b << ", " << str_c << "]\n";

        // Additional 4 test cases
        for (int i = 2; i <= 5; ++i) {
            auto t = generate_n_random_float_strings(3);
            std::cout << "Test " << i << ": [" << t[0] << ", " << t[1] << ", " << t[2] << "]\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "error\n";
        return 1;
    }
    return 0;
}
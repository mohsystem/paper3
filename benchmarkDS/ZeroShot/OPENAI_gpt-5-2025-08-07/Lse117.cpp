#include <iostream>
#include <random>
#include <string>
#include <array>
#include <sstream>
#include <iomanip>

std::string a(float value) {
    std::ostringstream oss;
    oss << std::setprecision(9) << value;
    return oss.str();
}

std::array<std::string, 3> generateRandomFloatStrings() {
    static std::random_device rd;
    static std::mt19937 rng(rd());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    float fa = dist(rng);
    float fb = dist(rng);
    float fc = dist(rng);

    std::string str_a = a(fa);
    std::string str_b = a(fb);
    std::string str_c = a(fc);

    return {str_a, str_b, str_c};
}

int main() {
    for (int i = 0; i < 5; ++i) {
        auto res = generateRandomFloatStrings();
        std::cout << res[0] << " " << res[1] << " " << res[2] << "\n";
    }
    return 0;
}
#include <iostream>
#include <string>
#include <random>

std::string a(float value) {
    return std::to_string(value);
}

float randomFloat() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    return dist(gen);
}

int main() {
    // 5 test cases
    for (int i = 1; i <= 5; ++i) {
        float a_val = randomFloat();
        float b_val = randomFloat();
        float c_val = randomFloat();

        std::string str_a = a(a_val);
        std::string str_b = a(b_val);
        std::string str_c = a(c_val);

        std::cout << "Test Case " << i << ":\n";
        std::cout << "str_a = " << str_a << "\n";
        std::cout << "str_b = " << str_b << "\n";
        std::cout << "str_c = " << str_c << "\n\n";
    }
    return 0;
}
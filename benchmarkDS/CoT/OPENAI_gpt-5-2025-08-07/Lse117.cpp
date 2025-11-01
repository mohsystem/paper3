#include <iostream>
#include <random>
#include <string>

std::string a(float x) {
    return std::to_string(x);
}

void run_test_case(std::mt19937& gen) {
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    float fa = dist(gen);
    float fb = dist(gen);
    float fc = dist(gen);

    std::string str_a = a(fa);
    std::string str_b = a(fb);
    std::string str_c = a(fc);

    std::cout << "str_a=" << str_a << ", str_b=" << str_b << ", str_c=" << str_c << std::endl;
}

int main() {
    std::random_device rd;
    std::mt19937 gen(rd());
    for (int i = 0; i < 5; i++) {
        run_test_case(gen);
    }
    return 0;
}
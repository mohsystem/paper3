#include <iostream>
#include <string>
#include <random>

std::string random_ascii_letters(std::size_t length) {
    static const std::string alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::string out;
    out.resize(length);
    std::random_device rd;
    std::uniform_int_distribution<int> dist(0, static_cast<int>(alphabet.size()) - 1);
    for (std::size_t i = 0; i < length; i++) {
        out[i] = alphabet[dist(rd)];
    }
    return out;
}

int main() {
    std::size_t tests[5] = {0, 1, 8, 16, 32};
    for (std::size_t n : tests) {
        std::string s = random_ascii_letters(n);
        std::cout << n << ": " << s << std::endl;
    }
    return 0;
}
#include <iostream>
#include <array>
#include <string>
#include <cstdio>
#include <cstdlib>

std::array<std::string, 3> generateRandomFloatStrings(unsigned int seed) {
    std::srand(seed);
    float f1 = static_cast<float>(std::rand()) / RAND_MAX;
    float f2 = static_cast<float>(std::rand()) / RAND_MAX;
    float f3 = static_cast<float>(std::rand()) / RAND_MAX;

    char buf1[64], buf2[64], buf3[64];
    std::sprintf(buf1, "%.6f", f1);
    std::sprintf(buf2, "%.6f", f2);
    std::sprintf(buf3, "%.6f", f3);

    std::string string = buf1;
    std::string string2 = buf2;
    std::string string3 = buf3;

    return {string, string2, string3};
}

int main() {
    unsigned int seeds[5] = {1, 2, 3, 4, 5};
    for (unsigned int s : seeds) {
        auto res = generateRandomFloatStrings(s);
        std::cout << "Seed " << s << ": [" << res[0] << ", " << res[1] << ", " << res[2] << "]\n";
    }
    return 0;
}
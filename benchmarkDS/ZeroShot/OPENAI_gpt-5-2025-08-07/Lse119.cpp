#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <random>
#include <string>
#include <vector>

std::vector<std::string> generateRandomFloatStrings(int precision) {
    if (precision < 0) precision = 0;
    if (precision > 9) precision = 9;

    float f1 = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
    float f2 = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
    float f3 = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);

    char buf1[64];
    char buf2[64];
    char buf3[64];

    // Use precision-safe formatting
    std::snprintf(buf1, sizeof(buf1), "%.*f", precision, f1);
    std::snprintf(buf2, sizeof(buf2), "%.*f", precision, f2);
    std::snprintf(buf3, sizeof(buf3), "%.*f", precision, f3);

    std::string string = buf1;
    std::string string2 = buf2;
    std::string string3 = buf3;

    return {string, string2, string3};
}

int main() {
    std::srand(static_cast<unsigned>(std::random_device{}()));
    int tests[5] = {2, 3, 0, 5, 7};
    for (int i = 0; i < 5; ++i) {
        auto out = generateRandomFloatStrings(tests[i]);
        std::cout << "Precision " << tests[i] << " -> ["
                  << out[0] << ", " << out[1] << ", " << out[2] << "]\n";
    }
    return 0;
}
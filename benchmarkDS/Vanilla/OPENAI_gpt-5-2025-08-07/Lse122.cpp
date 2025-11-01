#include <array>
#include <iostream>

class Lse122CPP {
public:
    static std::array<int, 10> copyAndFree(const std::array<int, 10>& src) {
        int* first = new int[10];
        for (int i = 0; i < 10; ++i) first[i] = src[i];

        int* second = new int[10];
        for (int i = 0; i < 10; ++i) second[i] = first[i];

        std::array<int, 10> result{};
        for (int i = 0; i < 10; ++i) result[i] = second[i];

        delete[] first;
        delete[] second;

        return result;
    }
};

int main() {
    std::array<std::array<int,10>,5> tests = {{
        std::array<int,10>{0,1,2,3,4,5,6,7,8,9},
        std::array<int,10>{10,11,12,13,14,15,16,17,18,19},
        std::array<int,10>{5,5,5,5,5,5,5,5,5,5},
        std::array<int,10>{-5,-4,-3,-2,-1,0,1,2,3,4},
        std::array<int,10>{42,0,-7,13,13,-1,2,2,999,-999}
    }};

    for (size_t i = 0; i < tests.size(); ++i) {
        auto out = Lse122CPP::copyAndFree(tests[i]);
        std::cout << "Test " << (i + 1) << ": ";
        for (int j = 0; j < 10; ++j) {
            std::cout << out[j] << (j + 1 < 10 ? ' ' : '\n');
        }
    }
    return 0;
}